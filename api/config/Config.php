<?php
/**
 * Configuration centralisée pour l'API Wallpaper
 */
class Config {

    /**
     * Configuration des headers CORS
     *
     * CORS (Cross-Origin Resource Sharing) est une sécurité des NAVIGATEURS WEB uniquement.
     * Les applications desktop (Qt, Electron, etc.) ne sont PAS affectées par CORS.
     *
     * Scénarios supportés :
     * 1. Application Qt Desktop (WallpaperIA.exe) -> Pas de CORS nécessaire, fonctionne toujours
     * 2. Interface web locale (fichier HTML ouvert directement) -> Origin: null
     * 3. Interface web hébergée -> Whitelist de domaines autorisés
     */
    public static function setCorsHeaders() {
        header('Content-Type: application/json');

        // Liste blanche des origines autorisées
        $allowed_origins = [
            'https://kazflow.com',           // Votre domaine principal
            'https://www.kazflow.com',       // Variante www
            'http://localhost',              // Développement local
            'http://127.0.0.1',              // Développement local
            'null'                           // Fichiers HTML locaux (file://)
        ];

        // Récupérer l'origine de la requête
        $origin = isset($_SERVER['HTTP_ORIGIN']) ? $_SERVER['HTTP_ORIGIN'] : '';

        // Vérifier si l'origine est dans la whitelist
        $origin_allowed = false;
        foreach ($allowed_origins as $allowed) {
            // Correspondance exacte ou correspondance de début pour localhost avec port
            if ($origin === $allowed ||
                ($allowed === 'http://localhost' && strpos($origin, 'http://localhost:') === 0) ||
                ($allowed === 'http://127.0.0.1' && strpos($origin, 'http://127.0.0.1:') === 0)) {
                $origin_allowed = true;
                break;
            }
        }

        // Si l'origine est autorisée, la renvoyer. Sinon, pas de header CORS.
        // Note : Les applications desktop (Qt) ne envoient pas de header Origin, donc elles passent toujours
        if ($origin_allowed && $origin !== '') {
            header('Access-Control-Allow-Origin: ' . $origin);
            header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
            header('Access-Control-Allow-Headers: Content-Type, Authorization');
            header('Access-Control-Allow-Credentials: true');
        }
        // Si pas d'origine dans la requête (application desktop), pas de header CORS = autorisé par défaut
    }

    /**
     * Gestion des requêtes OPTIONS (preflight)
     */
    public static function handlePreflightRequest() {
        if ($_SERVER['REQUEST_METHOD'] == 'OPTIONS') {
            exit(0);
        }
    }

    /**
     * Configuration des chemins de fichiers
     */
    public static function getPaths() {
        return [
            'csv_file' => __DIR__ . '/../wallpapers.csv',
            'ftp_config' => __DIR__ . '/../ftp.json',
            'thumbnail_dir' => __DIR__ . '/../miniatures/'
        ];
    }

    /**
     * Mapping des catégories vers des IDs courts
     */
    public static function getCategoryIds() {
        return [
            'ALIEN PLANETS' => 'al',
            'AUTUMN SEASONS' => 'as',
            'CUTE PETS' => 'cp',
            'CYBERPUNK FUTURISTIC' => 'cf',
            'DESERT LANDSCAPES' => 'dl',
            'FANTASY WORLDS' => 'fw',
            'GEOMETRIC PATTERNS' => 'gp',
            'INFERNO & FIRE' => 'if',
            'LANDSCAPE WITH GIRL' => 'lg',
            'MAGICAL FANTASY' => 'mf',
            'MANGA & ANIME' => 'ma',
            'MINIMALIST ABSTRACT' => 'mi',
            'MOUNTAINS & PEAKS' => 'mp',
            'MYSTERIOUS GOTHIC' => 'mg',
            'NATURAL LANDSCAPES' => 'nl',
            'OCEAN & MARINE' => 'om',
            'PARAGUAY' => 'py',
            'RELAXATION' => 'rx',
            'RETRO CARTOONS' => 'rc',
            'SPACE & COSMOS' => 'sc',
            'STEAMPUNK' => 'sp',
            'TROPICAL PARADISE' => 'tp',
            'URBAN ARCHITECTURE' => 'ua',
            'VINTAGE RETRO' => 'vr',
            'WINTER WONDERLAND' => 'ww'
        ];
    }

    /**
     * Configuration des types de contenu supportés
     */
    public static function getContentTypes() {
        return [
            'jpg' => 'image/jpeg',
            'jpeg' => 'image/jpeg',
            'png' => 'image/png',
            'gif' => 'image/gif',
            'webp' => 'image/webp',
            'bmp' => 'image/bmp',
            'svg' => 'image/svg+xml'
        ];
    }

    /**
     * Configuration des dimensions de thumbnails
     */
    public static function getThumbnailConfig() {
        return [
            'width' => 204,
            'height' => 115,
            'quality_jpeg' => 95,
            'quality_png' => 6
        ];
    }
}
?>