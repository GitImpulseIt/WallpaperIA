<?php
/**
 * Détection automatique de la langue et redirection
 * Redirige vers la page d'accueil localisée appropriée
 */

// Langues disponibles sur le site WallpaperAI
$availableLanguages = ['en', 'fr', 'es', 'pt', 'it', 'de', 'ru'];
$defaultLanguage = 'en';

/**
 * Normalise un code de langue (ex: 'fr-FR' -> 'fr')
 */
function normalizeLanguage($lang) {
    return strtolower(substr($lang, 0, 2));
}

/**
 * Détecte la langue préférée du navigateur
 */
function detectBrowserLanguage($availableLanguages, $defaultLanguage) {
    // Récupérer les langues acceptées par le navigateur
    $acceptLanguages = $_SERVER['HTTP_ACCEPT_LANGUAGE'] ?? '';
    
    if (empty($acceptLanguages)) {
        return $defaultLanguage;
    }
    
    // Parser les langues avec leurs priorités
    $languages = [];
    $parts = explode(',', $acceptLanguages);
    
    foreach ($parts as $part) {
        $part = trim($part);
        $langParts = explode(';', $part);
        $lang = normalizeLanguage($langParts[0]);
        
        // Extraire la priorité (q=0.8) ou utiliser 1.0 par défaut
        $priority = 1.0;
        if (isset($langParts[1]) && strpos($langParts[1], 'q=') === 0) {
            $priority = floatval(substr($langParts[1], 2));
        }
        
        $languages[$lang] = $priority;
    }
    
    // Trier par priorité décroissante
    arsort($languages);
    
    // Chercher la première langue disponible
    foreach ($languages as $lang => $priority) {
        if (in_array($lang, $availableLanguages)) {
            return $lang;
        }
    }
    
    return $defaultLanguage;
}

// 1. Vérifier d'abord si l'utilisateur a une préférence stockée en cookie
$detectedLanguage = $defaultLanguage;

if (isset($_COOKIE['preferred_language']) && in_array($_COOKIE['preferred_language'], $availableLanguages)) {
    $detectedLanguage = $_COOKIE['preferred_language'];
    error_log("Language detection: Using stored preference: {$detectedLanguage}");
} else {
    // 2. Sinon, détecter la langue du navigateur
    $detectedLanguage = detectBrowserLanguage($availableLanguages, $defaultLanguage);
    error_log("Language detection: Browser detection: {$detectedLanguage}");
}

// Construire l'URL de redirection
$redirectUrl = "/{$detectedLanguage}/";

// Ajouter le protocole et le domaine si nécessaire
$protocol = isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] === 'on' ? 'https' : 'http';
$host = $_SERVER['HTTP_HOST'];
$fullRedirectUrl = "{$protocol}://{$host}{$redirectUrl}";

// Log pour debug (optionnel, à supprimer en prod)
error_log("Language detection: Accept-Language='" . ($_SERVER['HTTP_ACCEPT_LANGUAGE'] ?? 'none') . "', Detected='{$detectedLanguage}', Redirecting to: {$fullRedirectUrl}");

// Effectuer la redirection
header("Location: {$fullRedirectUrl}", true, 302);
exit;
?> 