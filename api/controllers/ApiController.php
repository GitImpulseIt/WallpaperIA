<?php
require_once __DIR__ . '/../services/WallpaperService.php';
require_once __DIR__ . '/../services/FtpService.php';
require_once __DIR__ . '/../services/ThumbnailService.php';
require_once __DIR__ . '/../services/AuthService.php';
require_once __DIR__ . '/../services/CsvManager.php';
require_once __DIR__ . '/../core/Router.php';

/**
 * Controller principal de l'API
 */
class ApiController {
    private $wallpaperService;
    private $ftpService;
    private $thumbnailService;
    private $authService;
    private $csvManager;

    public function __construct() {
        $this->wallpaperService = new WallpaperService();
        $this->ftpService = new FtpService();
        $this->thumbnailService = new ThumbnailService();
        $this->authService = new AuthService();
        $this->csvManager = new CsvManager();
    }

    /**
     * Traite une requête selon son type
     */
    public function handleRequest($request) {
        switch ($request['type']) {
            case 'file':
                return $this->handleFileRequest($request['filename']);

            case 'thumbnail':
                return $this->handleThumbnailRequest($request['filename']);

            case 'categories':
                return $this->handleCategoriesRequest();

            case 'wallpapers':
                return $this->handleWallpapersRequest($request['category'] ?? null, $request['date'] ?? null);

            case 'add_wallpaper':
                return $this->handleAddWallpaperRequest($request['data'] ?? []);

            case 'info':
                return $this->handleInfoRequest();

            case 'error':
                return $request['response'];

            default:
                return [
                    'success' => false,
                    'error' => 'Unknown request type'
                ];
        }
    }

    /**
     * Gère les requêtes de fichiers depuis FTP
     */
    private function handleFileRequest($filename) {
        $result = $this->ftpService->getFileFromFtp($filename);

        if ($result['success']) {
            // Set appropriate headers for file download
            header('Content-Type: ' . $result['content_type']);
            header('Content-Disposition: inline; filename="' . $result['filename'] . '"');
            header('Cache-Control: public, max-age=3600'); // Cache for 1 hour

            // Output the file content directly
            echo $result['content'];
            exit;
        } else {
            // File not found or error
            header('HTTP/1.0 404 Not Found');
            return [
                'success' => false,
                'error' => $result['error']
            ];
        }
    }

    /**
     * Gère les requêtes de thumbnails
     */
    private function handleThumbnailRequest($filename) {
        $result = $this->thumbnailService->getThumbnail($filename);

        if ($result['success']) {
            // Set appropriate headers for thumbnail
            header('Content-Type: ' . $result['content_type']);
            header('Content-Disposition: inline; filename="' . $result['filename'] . '"');
            header('Cache-Control: public, max-age=86400'); // Cache for 24 hours

            // Add custom headers to indicate if thumbnail was cached
            if (isset($result['cached'])) {
                header('X-Thumbnail-Cached: ' . ($result['cached'] ? 'true' : 'false'));
            }

            // Output the thumbnail content directly
            echo $result['content'];
            exit;
        } else {
            // Thumbnail generation failed
            header('HTTP/1.0 404 Not Found');
            return [
                'success' => false,
                'error' => $result['error']
            ];
        }
    }

    /**
     * Gère les requêtes de catégories
     */
    private function handleCategoriesRequest() {
        return $this->wallpaperService->getCategories();
    }

    /**
     * Gère les requêtes de wallpapers (category et date obligatoires)
     */
    private function handleWallpapersRequest($category = null, $date = null) {
        // Vérifier que les paramètres obligatoires sont présents
        if (!$category) {
            return [
                'success' => false,
                'error' => 'Missing required parameter: category. Use /categories endpoint to get valid category IDs.'
            ];
        }

        if (!$date) {
            return [
                'success' => false,
                'error' => 'Missing required parameter: date. Use DD/MM/YYYY format (e.g., 29/09/2025).'
            ];
        }

        return $this->wallpaperService->getWallpapersByCategoryAndDate($category, $date);
    }

    /**
     * Gère les requêtes d'ajout de wallpaper
     */
    private function handleAddWallpaperRequest($data) {
        // Vérifier l'authentification
        $authResult = $this->authService->authenticate();

        if (!$authResult['success']) {
            // Envoyer les headers pour demander l'authentification
            $this->authService->requireAuth();
            return [
                'success' => false,
                'error' => $authResult['error']
            ];
        }

        // Extraire les données
        $category = $data['category'] ?? null;
        $filename = $data['filename'] ?? null;
        $date = $data['date'] ?? null;

        // Valider les paramètres requis
        if (!$category || !$filename || !$date) {
            return [
                'success' => false,
                'error' => 'Missing required parameters: category, filename, date',
                'required_format' => [
                    'category' => 'string (e.g., "CYBERPUNK/FUTURISTIC")',
                    'filename' => 'string with extension (e.g., "neon_city_night.png")',
                    'date' => 'string DD/MM/YYYY (e.g., "29/09/2025")'
                ]
            ];
        }

        // Vérifier si l'entrée existe déjà
        if ($this->csvManager->entryExists($category, $filename, $date)) {
            return [
                'success' => false,
                'error' => 'Entry already exists',
                'entry' => [
                    'category' => $category,
                    'filename' => $filename,
                    'date' => $date
                ]
            ];
        }

        // Ajouter l'entrée au CSV
        $result = $this->csvManager->addEntry($category, $filename, $date);

        if ($result['success']) {
            return [
                'success' => true,
                'message' => 'Wallpaper entry added successfully',
                'entry' => $result['entry'],
                'authenticated_as' => $authResult['username']
            ];
        } else {
            return [
                'success' => false,
                'error' => $result['error']
            ];
        }
    }

    /**
     * Gère les requêtes d'information sur l'API
     */
    private function handleInfoRequest() {
        return Router::getApiInfo();
    }
}
?>