<?php
/**
 * Point d'entrée principal de l'API Wallpaper
 * Version refactorisée avec architecture modulaire
 */

// Activer l'affichage des erreurs pour le debug
error_reporting(E_ALL);
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);

// Configuration des limites PHP (car php_value n'est pas supporté par tous les hébergeurs)
ini_set('memory_limit', '1024M');
ini_set('max_execution_time', '300');
ini_set('max_input_time', '300');

require_once __DIR__ . '/config/Config.php';
require_once __DIR__ . '/core/Router.php';
require_once __DIR__ . '/controllers/ApiController.php';

// Configuration des headers CORS
Config::setCorsHeaders();

// Handle preflight requests
Config::handlePreflightRequest();

try {
    // Initialize router
    $router = new Router();

    // Route the request
    $request = $router->route();

    // Initialize controller
    $controller = new ApiController();

    // Handle the request
    $response = $controller->handleRequest($request);

    // Send JSON response if not already sent (file/image responses exit early)
    if ($response !== null) {
        echo json_encode($response, JSON_PRETTY_PRINT);
    }

} catch (Exception $e) {
    // Global error handler avec détails complets pour le debug
    $error_response = [
        'success' => false,
        'error' => 'Internal server error: ' . $e->getMessage(),
        'debug' => [
            'file' => $e->getFile(),
            'line' => $e->getLine(),
            'trace' => $e->getTraceAsString()
        ]
    ];

    http_response_code(500);
    echo json_encode($error_response, JSON_PRETTY_PRINT);
}
?>