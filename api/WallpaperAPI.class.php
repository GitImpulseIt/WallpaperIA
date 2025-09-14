<?php
/**
 * Classe API pour la gestion des wallpapers et miniatures
 */
class WallpaperAPI {
    private $csv_file;
    private $category_ids;
    private $ftp_config;
    
    public function __construct() {
        $this->csv_file = __DIR__ . '/wallpapers.csv';
        $this->loadFtpConfig();
        
        // Mapping des catégories vers des IDs courts et uniques
        $this->category_ids = [
            'AUTUMN SEASONS' => 'as',
            'CYBERPUNK/FUTURISTIC' => 'cf',
            'DESERT LANDSCAPES' => 'dl',
            'FANTASY WORLDS' => 'fw',
            'GEOMETRIC PATTERNS' => 'gp',
            'MINIMALIST ABSTRACT' => 'ma',
            'MOUNTAINS & PEAKS' => 'mp',
            'MYSTERIOUS GOTHIC' => 'mg',
            'NATURAL LANDSCAPES' => 'nl',
            'OCEAN & MARINE' => 'om',
            'PARAGUAY' => 'py',
            'SPACE & COSMOS' => 'sc',
            'TROPICAL PARADISE' => 'tp',
            'URBAN ARCHITECTURE' => 'ua',
            'VINTAGE RETRO' => 'vr',
            'WINTER WONDERLAND' => 'ww'
        ];
    }
    
    /**
     * Load FTP configuration from ftp.json
     */
    private function loadFtpConfig() {
        $ftp_file = __DIR__ . '/ftp.json';
        if (file_exists($ftp_file)) {
            $this->ftp_config = json_decode(file_get_contents($ftp_file), true);
        } else {
            throw new Exception('FTP configuration file not found');
        }
    }
    
    /**
     * Read and parse the CSV file
     */
    private function loadWallpapers() {
        $wallpapers = [];
        
        if (!file_exists($this->csv_file)) {
            throw new Exception('Wallpapers CSV file not found');
        }
        
        $handle = fopen($this->csv_file, 'r');
        if ($handle !== FALSE) {
            // Skip header row
            fgetcsv($handle, 1000, ",", '"', "\\");
            
            while (($data = fgetcsv($handle, 1000, ",", '"', "\\")) !== FALSE) {
                if (count($data) >= 2) {
                    $wallpapers[] = [
                        'category' => trim($data[0]),
                        'filename' => trim($data[1])
                    ];
                }
            }
            fclose($handle);
        }
        
        return $wallpapers;
    }
    
    /**
     * Get unique categories from wallpapers with IDs
     */
    public function getCategories() {
        try {
            $wallpapers = $this->loadWallpapers();
            $categories = [];
            $found_categories = [];
            
            foreach ($wallpapers as $wallpaper) {
                $category = $wallpaper['category'];
                if (!in_array($category, $found_categories) && $category !== 'Unknown') {
                    $found_categories[] = $category;
                }
            }
            
            // Sort categories alphabetically
            sort($found_categories);
            
            // Build response with IDs
            foreach ($found_categories as $category) {
                $categories[] = [
                    'id' => $this->category_ids[$category] ?? strtolower(substr(str_replace(' ', '', $category), 0, 2)),
                    'name' => $category
                ];
            }
            
            return [
                'success' => true,
                'data' => $categories,
                'count' => count($categories)
            ];
            
        } catch (Exception $e) {
            return [
                'success' => false,
                'error' => $e->getMessage()
            ];
        }
    }
    
    /**
     * Get wallpapers by category (name or ID)
     */
    public function getWallpapersByCategory($category) {
        try {
            $wallpapers = $this->loadWallpapers();
            $filtered = [];
            $target_category = $category;
            
            // Check if category is an ID, convert to name
            $id_to_name = array_flip($this->category_ids);
            if (isset($id_to_name[$category])) {
                $target_category = $id_to_name[$category];
            }
            
            foreach ($wallpapers as $wallpaper) {
                if (strcasecmp($wallpaper['category'], $target_category) === 0) {
                    $filtered[] = $wallpaper;
                }
            }
            
            return [
                'success' => true,
                'category' => $target_category,
                'category_id' => $this->category_ids[$target_category] ?? null,
                'data' => $filtered,
                'count' => count($filtered)
            ];
            
        } catch (Exception $e) {
            return [
                'success' => false,
                'error' => $e->getMessage()
            ];
        }
    }
    
    /**
     * Get all wallpapers
     */
    public function getAllWallpapers() {
        try {
            $wallpapers = $this->loadWallpapers();
            
            return [
                'success' => true,
                'data' => $wallpapers,
                'count' => count($wallpapers)
            ];
            
        } catch (Exception $e) {
            return [
                'success' => false,
                'error' => $e->getMessage()
            ];
        }
    }
    
    /**
     * Generate or get thumbnail for an image
     */
    public function getThumbnail($filename) {
        try {
            // Vérifier que l'extension GD est installée
            if (!extension_loaded('gd')) {
                throw new Exception('GD extension is not installed');
            }
            
            // Augmenter la limite de mémoire pour le traitement d'images
            $current_limit = ini_get('memory_limit');
            $required_memory = '512M';
            if (intval($current_limit) < 512) {
                ini_set('memory_limit', $required_memory);
            }
            
            if (!$this->ftp_config) {
                throw new Exception('FTP configuration not loaded');
            }
            
            // Sanitize filename - only allow alphanumeric, dots, dashes and underscores
            if (!preg_match('/^[a-zA-Z0-9._-]+$/', $filename)) {
                throw new Exception('Invalid filename format');
            }
            
            // Check if thumbnail already exists - keep original format
            $thumbnail_dir = __DIR__ . '/miniatures/';
            $original_extension = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
            $thumbnail_filename = pathinfo($filename, PATHINFO_FILENAME) . "." . $original_extension;
            $thumbnail_path = $thumbnail_dir . $thumbnail_filename;
            
            // If thumbnail exists, return it
            if (file_exists($thumbnail_path)) {
                // Determine content type based on original extension
                $content_types = [
                    'png' => 'image/png',
                    'jpg' => 'image/jpeg',
                    'jpeg' => 'image/jpeg',
                    'gif' => 'image/gif',
                    'webp' => 'image/webp'
                ];
                $content_type = $content_types[$original_extension] ?? 'image/png';
                
                return [
                    'success' => true,
                    'content' => file_get_contents($thumbnail_path),
                    'content_type' => $content_type,
                    'filename' => $thumbnail_filename,
                    'cached' => true
                ];
            }
            
            // Get original image from FTP
            $original_result = $this->getFileFromFtp($filename);
            if (!$original_result['success']) {
                throw new Exception('Cannot retrieve original image: ' . $original_result['error']);
            }
            
            // Vérifier la taille de l'image pour estimer les besoins en mémoire
            $image_info = getimagesizefromstring($original_result['content']);
            if ($image_info === false) {
                throw new Exception('Cannot get image information from content');
            }
            
            // Estimer la mémoire nécessaire (width * height * 4 bytes per pixel * 3 for working copies)
            $estimated_memory = ($image_info[0] * $image_info[1] * 4 * 3);
            $memory_mb = round($estimated_memory / (1024 * 1024));
            
            // Si l'image est très grande, ajuster la limite de mémoire
            if ($memory_mb > 256) {
                ini_set('memory_limit', '1024M');
            }
            
            // Create image resource from content
            $original_image = @imagecreatefromstring($original_result['content']);
            if ($original_image === false) {
                throw new Exception('Cannot create image from file content');
            }
            
            // Get original dimensions
            $original_width = imagesx($original_image);
            $original_height = imagesy($original_image);
            
            // Calculate dimensions to maintain aspect ratio within 204x115
            $target_width = 204;
            $target_height = 115;
            
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
                             
            // Apply enhanced sharpening filters (method 3 parameters)
            if (function_exists('imagefilter')) {
                // No additional filters for two-step progressive - the technique itself provides better quality
            }
            
            // Save thumbnail in original format
            ob_start();
            switch ($original_extension) {
                case 'png':
                    imagepng($thumbnail, null, 6); // PNG compression level 6 (good quality/size balance)
                    break;
                case 'jpg':
                case 'jpeg':
                    imagejpeg($thumbnail, null, 95); // 95% quality for JPEG
                    break;
                case 'gif':
                    imagegif($thumbnail, null);
                    break;
                case 'webp':
                    if (function_exists('imagewebp')) {
                        imagewebp($thumbnail, null, 95);
                    } else {
                        // Fallback to PNG if WebP not supported
                        imagepng($thumbnail, null, 6);
                    }
                    break;
                default:
                    imagepng($thumbnail, null, 6); // Default to PNG
                    break;
            }
            $thumbnail_content = ob_get_contents();
            ob_end_clean();
            
            // Save to file
            if (!is_dir($thumbnail_dir)) {
                mkdir($thumbnail_dir, 0755, true);
            }
            file_put_contents($thumbnail_path, $thumbnail_content);
            
            // Clean up memory immediately
            imagedestroy($original_image);
            imagedestroy($thumbnail);
            
            // Libérer la mémoire des variables lourdes
            unset($original_result);
            
            // Forcer le garbage collector si disponible
            if (function_exists('gc_collect_cycles')) {
                gc_collect_cycles();
            }
            
            // Determine content type for response
            $content_types = [
                'png' => 'image/png',
                'jpg' => 'image/jpeg',
                'jpeg' => 'image/jpeg',
                'gif' => 'image/gif',
                'webp' => 'image/webp'
            ];
            $content_type = $content_types[$original_extension] ?? 'image/png';
            
            return [
                'success' => true,
                'content' => $thumbnail_content,
                'content_type' => $content_type,
                'filename' => $thumbnail_filename,
                'cached' => false,
                'original_size' => ['width' => $original_width, 'height' => $original_height],
                'thumbnail_size' => ['width' => $new_width, 'height' => $new_height]
            ];
            
        } catch (Exception $e) {
            return [
                'success' => false,
                'error' => $e->getMessage()
            ];
        }
    }
    
    /**
     * Get file from FTP server
     */
    public function getFileFromFtp($filename) {
        try {
            if (!$this->ftp_config) {
                throw new Exception('FTP configuration not loaded');
            }
            
            // Sanitize filename - only allow alphanumeric, dots, dashes and underscores
            if (!preg_match('/^[a-zA-Z0-9._-]+$/', $filename)) {
                throw new Exception('Invalid filename format');
            }
            
            $ftp_url = rtrim($this->ftp_config['host'], '/') . $this->ftp_config['directory'] . '/' . $filename;
            
            // Create FTP context with credentials
            $context = stream_context_create([
                'ftp' => [
                    'method' => 'ftp',
                    'protocol_version' => 1.0,
                ]
            ]);
            
            // Build the full FTP URL with credentials
            $parsed_url = parse_url($this->ftp_config['host']);
            $ftp_url_with_auth = $parsed_url['scheme'] . '://' . 
                                urlencode($this->ftp_config['user']) . ':' . 
                                urlencode($this->ftp_config['password']) . '@' . 
                                $parsed_url['host'] . 
                                (isset($parsed_url['port']) ? ':' . $parsed_url['port'] : '') .
                                $this->ftp_config['directory'] . '/' . $filename;
            
            // Try to get file content
            $file_content = file_get_contents($ftp_url_with_auth, false, $context);
            
            if ($file_content === false) {
                throw new Exception('File not found or FTP error');
            }
            
            // Determine content type based on file extension
            $ext = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
            $content_types = [
                'jpg' => 'image/jpeg',
                'jpeg' => 'image/jpeg',
                'png' => 'image/png',
                'gif' => 'image/gif',
                'webp' => 'image/webp',
                'bmp' => 'image/bmp',
                'svg' => 'image/svg+xml'
            ];
            
            $content_type = $content_types[$ext] ?? 'application/octet-stream';
            
            return [
                'success' => true,
                'content' => $file_content,
                'content_type' => $content_type,
                'filename' => $filename
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