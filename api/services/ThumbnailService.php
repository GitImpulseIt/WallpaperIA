<?php
require_once __DIR__ . '/../config/Config.php';
require_once __DIR__ . '/FtpService.php';

/**
 * Service pour la génération et gestion des thumbnails
 */
class ThumbnailService {
    private $thumbnail_dir;
    private $ftpService;
    private $thumbnail_config;

    public function __construct() {
        $paths = Config::getPaths();
        $this->thumbnail_dir = $paths['thumbnail_dir'];
        $this->thumbnail_config = Config::getThumbnailConfig();
        $this->ftpService = new FtpService();
    }

    /**
     * Valide le nom de fichier contre les attaques de path traversal
     */
    private function validateFilename($filename) {
        // Interdire les chemins vides
        if (empty($filename)) {
            throw new Exception('Filename cannot be empty');
        }

        // Utiliser basename pour extraire uniquement le nom de fichier (supprime les path traversal)
        $sanitized = basename($filename);

        // Vérifier que le fichier n'a pas été modifié par basename (détection de path traversal)
        if ($sanitized !== $filename) {
            throw new Exception('Invalid filename: path traversal detected');
        }

        // Interdire les caractères dangereux supplémentaires
        if (preg_match('/[\/\\\\]/', $filename)) {
            throw new Exception('Invalid filename: directory separators not allowed');
        }

        // Vérifier le format avec regex stricte
        if (!preg_match('/^[a-zA-Z0-9._-]+$/', $filename)) {
            throw new Exception('Invalid filename: only alphanumeric, dots, hyphens and underscores allowed');
        }
    }

    /**
     * Configure la mémoire pour le traitement d'images
     */
    private function configureMemory($image_info = null) {
        // Augmenter la limite de mémoire pour le traitement d'images
        $current_limit = ini_get('memory_limit');
        $required_memory = '512M';
        if (intval($current_limit) < 512) {
            ini_set('memory_limit', $required_memory);
        }

        // Si les infos d'image sont disponibles, ajuster selon la taille
        if ($image_info) {
            $estimated_memory = ($image_info[0] * $image_info[1] * 4 * 3);
            $memory_mb = round($estimated_memory / (1024 * 1024));

            if ($memory_mb > 256) {
                ini_set('memory_limit', '1024M');
            }
        }
    }

    /**
     * Vérifie si un thumbnail existe déjà
     */
    private function getThumbnailIfExists($filename) {
        $original_extension = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
        $thumbnail_filename = pathinfo($filename, PATHINFO_FILENAME) . "." . $original_extension;
        $thumbnail_path = $this->thumbnail_dir . $thumbnail_filename;

        if (file_exists($thumbnail_path)) {
            $content_types = Config::getContentTypes();
            $content_type = $content_types[$original_extension] ?? 'image/png';

            return [
                'success' => true,
                'content' => file_get_contents($thumbnail_path),
                'content_type' => $content_type,
                'filename' => $thumbnail_filename,
                'cached' => true
            ];
        }

        return null;
    }

    /**
     * Crée un thumbnail redimensionné avec qualité optimisée
     */
    private function createThumbnail($original_image, $original_width, $original_height) {
        $target_width = $this->thumbnail_config['width'];
        $target_height = $this->thumbnail_config['height'];

        // Calculate dimensions to maintain aspect ratio
        $ratio_w = $target_width / $original_width;
        $ratio_h = $target_height / $original_height;
        $ratio = min($ratio_w, $ratio_h);

        $new_width = (int)($original_width * $ratio);
        $new_height = (int)($original_height * $ratio);

        // Two-step progressive resizing for better quality
        if ($ratio < 0.5) {
            // First resize to 50% if target is smaller than 50%
            $intermediate_width = (int)($original_width * 0.5);
            $intermediate_height = (int)($original_height * 0.5);

            $intermediate = imagecreatetruecolor($intermediate_width, $intermediate_height);
            $white_intermediate = imagecolorallocate($intermediate, 255, 255, 255);
            imagefill($intermediate, 0, 0, $white_intermediate);
            imagealphablending($intermediate, true);

            // First step: original -> 50%
            imagecopyresampled($intermediate, $original_image, 0, 0, 0, 0,
                               $intermediate_width, $intermediate_height, $original_width, $original_height);

            // Then resize to final size from intermediate
            $thumbnail = imagecreatetruecolor($new_width, $new_height);
            $white = imagecolorallocate($thumbnail, 255, 255, 255);
            imagefill($thumbnail, 0, 0, $white);
            imagealphablending($thumbnail, true);

            // Second step: 50% -> final size
            imagecopyresampled($thumbnail, $intermediate, 0, 0, 0, 0,
                               $new_width, $new_height, $intermediate_width, $intermediate_height);

            imagedestroy($intermediate);
        } else {
            // Direct resize if target is >= 50%
            $thumbnail = imagecreatetruecolor($new_width, $new_height);
            $white = imagecolorallocate($thumbnail, 255, 255, 255);
            imagefill($thumbnail, 0, 0, $white);
            imagealphablending($thumbnail, true);

            // Single step resize
            imagecopyresampled($thumbnail, $original_image, 0, 0, 0, 0,
                               $new_width, $new_height, $original_width, $original_height);
        }

        return [
            'image' => $thumbnail,
            'width' => $new_width,
            'height' => $new_height
        ];
    }

    /**
     * Sauve le thumbnail dans le format approprié
     */
    private function saveThumbnail($thumbnail_image, $original_extension) {
        ob_start();
        switch ($original_extension) {
            case 'png':
                imagepng($thumbnail_image, null, $this->thumbnail_config['quality_png']);
                break;
            case 'jpg':
            case 'jpeg':
                imagejpeg($thumbnail_image, null, $this->thumbnail_config['quality_jpeg']);
                break;
            case 'gif':
                imagegif($thumbnail_image, null);
                break;
            case 'webp':
                if (function_exists('imagewebp')) {
                    imagewebp($thumbnail_image, null, $this->thumbnail_config['quality_jpeg']);
                } else {
                    imagepng($thumbnail_image, null, $this->thumbnail_config['quality_png']);
                }
                break;
            default:
                imagepng($thumbnail_image, null, $this->thumbnail_config['quality_png']);
                break;
        }
        $thumbnail_content = ob_get_contents();
        ob_end_clean();

        return $thumbnail_content;
    }

    /**
     * Génère ou récupère un thumbnail pour une image
     */
    public function getThumbnail($filename) {
        try {
            // Vérifier que l'extension GD est installée
            if (!extension_loaded('gd')) {
                throw new Exception('GD extension is not installed');
            }

            $this->validateFilename($filename);
            $this->configureMemory();

            // Check if thumbnail already exists
            $existing_thumbnail = $this->getThumbnailIfExists($filename);
            if ($existing_thumbnail) {
                return $existing_thumbnail;
            }

            // Get original image from FTP
            $original_result = $this->ftpService->getFileFromFtp($filename);
            if (!$original_result['success']) {
                throw new Exception('Cannot retrieve original image: ' . $original_result['error']);
            }

            // Get image info and configure memory accordingly
            $image_info = getimagesizefromstring($original_result['content']);
            if ($image_info === false) {
                throw new Exception('Cannot get image information from content');
            }
            $this->configureMemory($image_info);

            // Create image resource from content
            $original_image = @imagecreatefromstring($original_result['content']);
            if ($original_image === false) {
                throw new Exception('Cannot create image from file content');
            }

            // Get original dimensions
            $original_width = imagesx($original_image);
            $original_height = imagesy($original_image);

            // Create thumbnail
            $thumbnail_result = $this->createThumbnail($original_image, $original_width, $original_height);

            // Save thumbnail in original format
            $original_extension = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
            $thumbnail_content = $this->saveThumbnail($thumbnail_result['image'], $original_extension);

            // Save to file
            if (!is_dir($this->thumbnail_dir)) {
                mkdir($this->thumbnail_dir, 0755, true);
            }
            $thumbnail_filename = pathinfo($filename, PATHINFO_FILENAME) . "." . $original_extension;
            $thumbnail_path = $this->thumbnail_dir . $thumbnail_filename;
            file_put_contents($thumbnail_path, $thumbnail_content);

            // Clean up memory
            imagedestroy($original_image);
            imagedestroy($thumbnail_result['image']);
            unset($original_result);

            if (function_exists('gc_collect_cycles')) {
                gc_collect_cycles();
            }

            // Determine content type for response
            $content_types = Config::getContentTypes();
            $content_type = $content_types[$original_extension] ?? 'image/png';

            return [
                'success' => true,
                'content' => $thumbnail_content,
                'content_type' => $content_type,
                'filename' => $thumbnail_filename,
                'cached' => false,
                'original_size' => ['width' => $original_width, 'height' => $original_height],
                'thumbnail_size' => ['width' => $thumbnail_result['width'], 'height' => $thumbnail_result['height']]
            ];

        } catch (Exception $e) {
            return [
                'success' => false,
                'error' => $e->getMessage()
            ];
        }
    }
}
?>