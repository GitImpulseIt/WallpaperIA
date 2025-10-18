<?php
/**
 * Router pour gérer les requêtes et endpoints
 */
class Router {
    private $request_uri;
    private $request_method;
    private $endpoint;
    private $is_file_request = false;
    private $is_mini_request = false;
    private $requested_filename = null;

    public function __construct() {
        $this->request_uri = $_SERVER['REQUEST_URI'];
        $this->request_method = $_SERVER['REQUEST_METHOD'];
        $this->parseRequest();
    }

    /**
     * Parse la requête URI pour extraire l'endpoint
     */
    private function parseRequest() {
        // Parse the URI to get the endpoint
        // Remove query string first
        $uri_without_query = strtok($this->request_uri, '?');

        // Split by / and filter empty parts
        $uri_parts = array_values(array_filter(explode('/', $uri_without_query)));

        // Remove 'api' from path if present (for /api/wallpapers structure)
        $uri_parts = array_values(array_filter($uri_parts, function($part) {
            return $part !== 'api' && $part !== 'index.php';
        }));

        // Get the last part as endpoint
        $this->endpoint = end($uri_parts);

        // Handle empty endpoint (root path)
        if (empty($this->endpoint) || $this->endpoint === 'index.php') {
            $this->endpoint = '';
        }

        // Handle file retrieval endpoint pattern /get/{filename} and /mini/{filename}
        if (count($uri_parts) >= 2) {
            $second_last = $uri_parts[count($uri_parts) - 2];
            if ($second_last === 'get') {
                $this->is_file_request = true;
                $this->requested_filename = $this->endpoint;
            } elseif ($second_last === 'mini') {
                $this->is_mini_request = true;
                $this->requested_filename = $this->endpoint;
            }
        }
    }

    /**
     * Route la requête vers le bon controller
     */
    public function route() {
        switch ($this->request_method) {
            case 'GET':
                return $this->handleGetRequest();
            case 'POST':
                return $this->handlePostRequest();
            default:
                return $this->methodNotAllowed();
        }
    }

    /**
     * Gère les requêtes GET
     */
    private function handleGetRequest() {
        if ($this->is_file_request && $this->requested_filename) {
            return [
                'type' => 'file',
                'filename' => $this->requested_filename
            ];
        } elseif ($this->is_mini_request && $this->requested_filename) {
            return [
                'type' => 'thumbnail',
                'filename' => $this->requested_filename
            ];
        } elseif ($this->endpoint === 'categories') {
            return ['type' => 'categories'];
        } elseif ($this->endpoint === 'wallpapers') {
            $category = $_GET['category'] ?? null;
            $date = $_GET['date'] ?? null;
            return [
                'type' => 'wallpapers',
                'category' => $category,
                'date' => $date
            ];
        } elseif ($this->endpoint === 'index.php' || $this->endpoint === '' || $this->endpoint === 'api') {
            return ['type' => 'info'];
        } else {
            return $this->endpointNotFound();
        }
    }

    /**
     * Endpoint non trouvé
     */
    private function endpointNotFound() {
        return [
            'type' => 'error',
            'response' => [
                'success' => false,
                'error' => 'Endpoint not found',
                'available_endpoints' => ['categories', 'wallpapers', 'get/{filename}', 'mini/{filename}']
            ]
        ];
    }

    /**
     * Gère les requêtes POST
     */
    private function handlePostRequest() {
        if ($this->endpoint === 'wallpapers') {
            // Parse JSON body
            $json = file_get_contents('php://input');
            $data = json_decode($json, true);

            return [
                'type' => 'add_wallpaper',
                'data' => $data
            ];
        } else {
            return $this->endpointNotFound();
        }
    }

    /**
     * Méthode non autorisée
     */
    private function methodNotAllowed() {
        return [
            'type' => 'error',
            'response' => [
                'success' => false,
                'error' => 'Method not allowed',
                'allowed_methods' => ['GET', 'POST']
            ]
        ];
    }

    /**
     * Retourne les informations de l'API
     */
    public static function getApiInfo() {
        return [
            'success' => true,
            'message' => 'Wallpaper API',
            'version' => '1.0',
            'endpoints' => [
                'GET /categories' => 'Get all available categories',
                'GET /wallpapers?category={id}&date={DD/MM/YYYY}' => 'Get wallpapers by category and date (both required)',
                'GET /get/{filename}' => 'Get file from FTP server',
                'GET /mini/{filename}' => 'Get thumbnail (204x115 JPG) - generates if not exists',
                'POST /wallpapers' => 'Add new wallpaper entry (requires HTTP Basic Auth)'
            ]
        ];
    }
}
?>