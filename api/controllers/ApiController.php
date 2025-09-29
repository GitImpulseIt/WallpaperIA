<?php
require_once __DIR__ . '/../services/WallpaperService.php';
require_once __DIR__ . '/../services/FtpService.php';
require_once __DIR__ . '/../services/ThumbnailService.php';
require_once __DIR__ . '/../core/Router.php';

/**
 * Controller principal de l'API
 */
class ApiController {
    private $wallpaperService;
    private $ftpService;
    private $thumbnailService;

    public function __construct() {
        $this->wallpaperService = new WallpaperService();
        $this->ftpService = new FtpService();
        $this->thumbnailService = new ThumbnailService();
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
                return $this->handleWallpapersRequest($request['category'] ?? null);

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
     * Gère les requêtes de wallpapers
     */
    private function handleWallpapersRequest($category = null) {
        if ($category) {
            return $this->wallpaperService->getWallpapersByCategory($category);
        } else {
            return $this->wallpaperService->getAllWallpapers();
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