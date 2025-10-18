<?php
/**
 * Script de debug pour tester l'authentification
 * Utilisation : curl -u test:test https://kazflow.com/debug_auth.php
 */

header('Content-Type: application/json');

$debug = [
    'php_auth_user' => $_SERVER['PHP_AUTH_USER'] ?? 'NOT SET',
    'php_auth_pw' => isset($_SERVER['PHP_AUTH_PW']) ? 'SET (hidden)' : 'NOT SET',
    'http_authorization' => $_SERVER['HTTP_AUTHORIZATION'] ?? 'NOT SET',
    'redirect_http_authorization' => $_SERVER['REDIRECT_HTTP_AUTHORIZATION'] ?? 'NOT SET',
    'all_server_vars_with_auth' => []
];

// Trouver toutes les variables contenant "AUTH"
foreach ($_SERVER as $key => $value) {
    if (stripos($key, 'AUTH') !== false || stripos($key, 'AUTHORIZATION') !== false) {
        if (stripos($key, 'PW') !== false || stripos($key, 'PASS') !== false) {
            $debug['all_server_vars_with_auth'][$key] = 'SET (hidden)';
        } else {
            $debug['all_server_vars_with_auth'][$key] = $value;
        }
    }
}

// Tester le décodage du header Authorization
if (isset($_SERVER['HTTP_AUTHORIZATION']) && strpos($_SERVER['HTTP_AUTHORIZATION'], 'Basic ') === 0) {
    $decoded = base64_decode(substr($_SERVER['HTTP_AUTHORIZATION'], 6));
    $parts = explode(':', $decoded, 2);
    if (count($parts) === 2) {
        $debug['decoded_from_http_authorization'] = [
            'username' => $parts[0],
            'password' => 'SET (hidden)'
        ];
    }
}

echo json_encode($debug, JSON_PRETTY_PRINT);
?>
