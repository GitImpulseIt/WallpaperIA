<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Handle preflight requests
if ($_SERVER['REQUEST_METHOD'] == 'OPTIONS') {
    exit(0);
}

// Get the request URI and method
$request_uri = $_SERVER['REQUEST_URI'];
$request_method = $_SERVER['REQUEST_METHOD'];

// Parse the URI to get the endpoint
$uri_parts = explode('/', trim($request_uri, '/'));
$endpoint = end($uri_parts);

// Remove query parameters
$endpoint = strtok($endpoint, '?');

// Handle file retrieval endpoint pattern /get/{filename} and /mini/{filename}
$is_file_request = false;
$is_mini_request = false;
$requested_filename = null;
if (count($uri_parts) >= 2) {
    $second_last = $uri_parts[count($uri_parts) - 2];
    if ($second_last === 'get') {
        $is_file_request = true;
        $requested_filename = $endpoint;
    } elseif ($second_last === 'mini') {
        $is_mini_request = true;
        $requested_filename = $endpoint;
    }
}

require_once __DIR__ . '/WallpaperAPI.class.php';

// Classe déplacée vers WallpaperAPI.class.php pour éviter les conflits d'inclusion

// Initialize API
$api = new WallpaperAPI();

// Route handling
switch ($request_method) {
    case 'GET':
        if ($is_file_request && $requested_filename) {
            // Handle file retrieval from FTP
            $result = $api->getFileFromFtp($requested_filename);
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
                $response = [
                    'success' => false,
                    'error' => $result['error']
                ];
            }
        } elseif ($is_mini_request && $requested_filename) {
            // Handle thumbnail generation/retrieval
            $result = $api->getThumbnail($requested_filename);
            if ($result['success']) {
                // Set appropriate headers for thumbnail
                header('Content-Type: ' . $result['content_type']);
                header('Content-Disposition: inline; filename="' . $result['filename'] . '"');
                header('Cache-Control: public, max-age=86400'); // Cache for 24 hours (thumbnails change less often)
                
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
                $response = [
                    'success' => false,
                    'error' => $result['error']
                ];
            }
        } elseif ($endpoint === 'categories') {
            $response = $api->getCategories();
        } elseif ($endpoint === 'wallpapers') {
            // Check for category parameter
            $category = $_GET['category'] ?? null;
            if ($category) {
                $response = $api->getWallpapersByCategory($category);
            } else {
                $response = $api->getAllWallpapers();
            }
        } elseif ($endpoint === 'index.php' || $endpoint === '') {
            // API info endpoint
            $response = [
                'success' => true,
                'message' => 'Wallpaper API',
                'version' => '1.0',
                'endpoints' => [
                    'GET /categories' => 'Get all available categories',
                    'GET /wallpapers' => 'Get all wallpapers',
                    'GET /wallpapers?category={name}' => 'Get wallpapers by category',
                    'GET /get/{filename}' => 'Get file from FTP server',
                    'GET /mini/{filename}' => 'Get thumbnail (200x120 in original format) - generates if not exists'
                ]
            ];
        } else {
            $response = [
                'success' => false,
                'error' => 'Endpoint not found',
                'available_endpoints' => ['categories', 'wallpapers', 'get/{filename}', 'mini/{filename}']
            ];
        }
        break;
        
    default:
        $response = [
            'success' => false,
            'error' => 'Method not allowed',
            'allowed_methods' => ['GET']
        ];
        break;
}

// Send response
echo json_encode($response, JSON_PRETTY_PRINT);
?>