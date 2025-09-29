<?php
/**
 * Point d'entrée principal de l'API Wallpaper
 * Version refactorisée avec architecture modulaire
 */

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
    // Global error handler
    $error_response = [
        'success' => false,
        'error' => 'Internal server error: ' . $e->getMessage()
    ];

    echo json_encode($error_response, JSON_PRETTY_PRINT);
}
?>