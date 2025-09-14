<?php
/**
 * Script d'information sur la mémoire PHP
 * Utilisez ce script pour diagnostiquer les problèmes de mémoire
 */

header('Content-Type: application/json');

echo json_encode([
    'current_memory_usage' => memory_get_usage(true),
    'current_memory_usage_mb' => round(memory_get_usage(true) / (1024 * 1024), 2) . ' MB',
    'peak_memory_usage' => memory_get_peak_usage(true),
    'peak_memory_usage_mb' => round(memory_get_peak_usage(true) / (1024 * 1024), 2) . ' MB',
    'memory_limit' => ini_get('memory_limit'),
    'max_execution_time' => ini_get('max_execution_time'),
    'gd_info' => function_exists('gd_info') ? gd_info() : 'GD extension not available',
    'php_version' => phpversion(),
    'system_info' => [
        'os' => PHP_OS,
        'architecture' => php_uname('m'),
        'server_software' => $_SERVER['SERVER_SOFTWARE'] ?? 'Unknown'
    ]
], JSON_PRETTY_PRINT);
?>