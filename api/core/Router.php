<?php
require_once __DIR__ . '/../services/RateLimitService.php';

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
     * Valide le nom de fichier contre les attaques de path traversal
     * @param string $filename
     * @return bool
     */
    private function isValidFilename($filename) {
        if (empty($filename)) {
            return false;
        }

        // Utiliser basename pour extraire uniquement le nom de fichier
        $sanitized = basename($filename);

        // Vérifier que le fichier n'a pas été modifié par basename
        if ($sanitized !== $filename) {
            return false;
        }

        // Interdire les caractères dangereux
        if (preg_match('/[\/\\\\]/', $filename)) {
            return false;
        }

        // Vérifier le format avec regex stricte
        if (!preg_match('/^[a-zA-Z0-9._-]+$/', $filename)) {
            return false;
        }

        return true;
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
     * Applique le rate limiting pour un endpoint
     * @param string $endpoint_type Type d'endpoint pour le rate limiting
     * @return array|null Retourne un tableau d'erreur si rate limit dépassé, null sinon
     */
    private function applyRateLimit($endpoint_type) {
        $rateLimiter = new RateLimitService();
        $client_ip = RateLimitService::getClientIp();

        $limit_check = $rateLimiter->checkLimit($endpoint_type, $client_ip);

        // Toujours envoyer les headers de rate limiting
        RateLimitService::sendRateLimitHeaders($limit_check);

        // Si la limite est dépassée, retourner une erreur
        if (!$limit_check['allowed']) {
            return [
                'type' => 'error',
                'response' => RateLimitService::getTooManyRequestsResponse($limit_check)
            ];
        }

        return null; // Pas de limite dépassée
    }

    /**
     * Gère les requêtes GET
     */
    private function handleGetRequest() {
        if ($this->is_file_request && $this->requested_filename) {
            // Valider le filename contre les path traversal
            if (!$this->isValidFilename($this->requested_filename)) {
                return [
                    'type' => 'error',
                    'response' => [
                        'success' => false,
                        'error' => 'Invalid filename: path traversal or invalid characters detected'
                    ]
                ];
            }

            // Appliquer le rate limiting
            $rate_limit_result = $this->applyRateLimit('file');
            if ($rate_limit_result !== null) {
                return $rate_limit_result;
            }

            return [
                'type' => 'file',
                'filename' => $this->requested_filename
            ];
        } elseif ($this->is_mini_request && $this->requested_filename) {
            // Valider le filename contre les path traversal
            if (!$this->isValidFilename($this->requested_filename)) {
                return [
                    'type' => 'error',
                    'response' => [
                        'success' => false,
                        'error' => 'Invalid filename: path traversal or invalid characters detected'
                    ]
                ];
            }

            // Appliquer le rate limiting (thumbnail est plus coûteux)
            $rate_limit_result = $this->applyRateLimit('thumbnail');
            if ($rate_limit_result !== null) {
                return $rate_limit_result;
            }

            return [
                'type' => 'thumbnail',
                'filename' => $this->requested_filename
            ];
        } elseif ($this->endpoint === 'categories') {
            // Appliquer le rate limiting
            $rate_limit_result = $this->applyRateLimit('categories');
            if ($rate_limit_result !== null) {
                return $rate_limit_result;
            }

            return ['type' => 'categories'];
        } elseif ($this->endpoint === 'wallpapers') {
            // Appliquer le rate limiting
            $rate_limit_result = $this->applyRateLimit('wallpapers');
            if ($rate_limit_result !== null) {
                return $rate_limit_result;
            }

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
            // Pour les endpoints authentifiés, on utilise l'IP comme identifiant initial
            // L'ApiController appliquera un rate limit supplémentaire par username après auth
            $rate_limit_result = $this->applyRateLimit('add_wallpaper');
            if ($rate_limit_result !== null) {
                return $rate_limit_result;
            }

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