<?php
/**
 * Configuration centralisée pour l'API Wallpaper
 */
class Config {

    /**
     * Configuration des headers CORS
     */
    public static function setCorsHeaders() {
        header('Content-Type: application/json');
        header('Access-Control-Allow-Origin: *');
        header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
        header('Access-Control-Allow-Headers: Content-Type');
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
            'CYBERPUNK-FUTURISTIC' => 'cy',
            'CYBERPUNK/FUTURISTIC' => 'cf',
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
            'RETRO CARTOONS 70s-2000s' => 'rc',
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