<?php
/**
 * Service de Rate Limiting pour protéger l'API contre les abus
 * Utilise un système de fichiers avec nettoyage automatique
 */
class RateLimitService {
    private $storage_dir;
    private $limits = [
        // Endpoints publics (par IP)
        'categories' => ['max' => 100, 'window' => 60],      // 100 requêtes par minute
        'wallpapers' => ['max' => 200, 'window' => 60],      // 200 requêtes par minute
        'file' => ['max' => 300, 'window' => 60],            // 300 fichiers par minute
        'thumbnail' => ['max' => 150, 'window' => 60],       // 150 thumbnails par minute (coûteux)

        // Endpoints authentifiés (par IP - premier niveau)
        'add_wallpaper' => ['max' => 20, 'window' => 3600],  // 20 ajouts par heure par IP

        // Endpoints authentifiés (par username - second niveau)
        'add_wallpaper_user' => ['max' => 50, 'window' => 3600],  // 50 ajouts par heure par user
    ];

    public function __construct() {
        $this->storage_dir = __DIR__ . '/../rate_limit/';

        // Créer le répertoire s'il n'existe pas
        if (!is_dir($this->storage_dir)) {
            mkdir($this->storage_dir, 0755, true);
        }

        // Nettoyer les anciens fichiers (exécuté 1% du temps pour éviter la surcharge)
        if (mt_rand(1, 100) === 1) {
            $this->cleanupOldFiles();
        }
    }

    /**
     * Vérifie si une requête est autorisée selon les limites
     * @param string $endpoint Nom de l'endpoint
     * @param string $identifier IP ou username
     * @return array ['allowed' => bool, 'retry_after' => int|null, 'remaining' => int]
     */
    public function checkLimit($endpoint, $identifier) {
        // Si l'endpoint n'a pas de limite configurée, autoriser
        if (!isset($this->limits[$endpoint])) {
            return [
                'allowed' => true,
                'retry_after' => null,
                'remaining' => 999999
            ];
        }

        $limit_config = $this->limits[$endpoint];
        $max_requests = $limit_config['max'];
        $time_window = $limit_config['window'];

        // Créer un hash sécurisé pour le stockage
        $key = hash('sha256', $endpoint . ':' . $identifier);
        $file_path = $this->storage_dir . $key . '.json';

        // Charger l'historique des requêtes
        $history = $this->loadHistory($file_path);

        // Timestamp actuel
        $now = time();

        // Filtrer les requêtes dans la fenêtre de temps
        $recent_requests = array_filter($history, function($timestamp) use ($now, $time_window) {
            return ($now - $timestamp) < $time_window;
        });

        // Compter les requêtes récentes
        $request_count = count($recent_requests);

        // Vérifier si la limite est dépassée
        if ($request_count >= $max_requests) {
            // Calculer le temps d'attente avant la prochaine requête autorisée
            $oldest_request = min($recent_requests);
            $retry_after = $time_window - ($now - $oldest_request);

            return [
                'allowed' => false,
                'retry_after' => max(1, $retry_after),
                'remaining' => 0,
                'limit' => $max_requests,
                'window' => $time_window
            ];
        }

        // Ajouter la requête actuelle à l'historique
        $recent_requests[] = $now;

        // Sauvegarder l'historique mis à jour
        $this->saveHistory($file_path, $recent_requests);

        return [
            'allowed' => true,
            'retry_after' => null,
            'remaining' => $max_requests - count($recent_requests),
            'limit' => $max_requests,
            'window' => $time_window
        ];
    }

    /**
     * Charge l'historique des requêtes depuis un fichier
     * @param string $file_path
     * @return array
     */
    private function loadHistory($file_path) {
        if (!file_exists($file_path)) {
            return [];
        }

        $content = @file_get_contents($file_path);
        if ($content === false) {
            return [];
        }

        $data = json_decode($content, true);
        return is_array($data) ? $data : [];
    }

    /**
     * Sauvegarde l'historique des requêtes dans un fichier
     * @param string $file_path
     * @param array $history
     */
    private function saveHistory($file_path, $history) {
        // Garder seulement les timestamps (pas besoin de plus de données)
        $content = json_encode(array_values($history));
        @file_put_contents($file_path, $content, LOCK_EX);
    }

    /**
     * Nettoie les fichiers de rate limiting expirés (plus vieux que 24h)
     */
    private function cleanupOldFiles() {
        $max_age = 86400; // 24 heures
        $now = time();

        $files = glob($this->storage_dir . '*.json');
        foreach ($files as $file) {
            $file_age = $now - filemtime($file);
            if ($file_age > $max_age) {
                @unlink($file);
            }
        }
    }

    /**
     * Obtient l'IP réelle du client (gère les proxies et load balancers)
     * @return string
     */
    public static function getClientIp() {
        // Vérifier les headers de proxy dans l'ordre de priorité
        $headers = [
            'HTTP_CF_CONNECTING_IP',    // Cloudflare
            'HTTP_X_REAL_IP',            // Nginx proxy
            'HTTP_X_FORWARDED_FOR',      // Proxy standard
            'REMOTE_ADDR'                // Connexion directe
        ];

        foreach ($headers as $header) {
            if (!empty($_SERVER[$header])) {
                // Pour X-Forwarded-For, prendre la première IP (client réel)
                if ($header === 'HTTP_X_FORWARDED_FOR') {
                    $ips = explode(',', $_SERVER[$header]);
                    return trim($ips[0]);
                }
                return $_SERVER[$header];
            }
        }

        return '0.0.0.0'; // Fallback
    }

    /**
     * Envoie les headers de rate limiting standards
     * @param array $limit_info Résultat de checkLimit()
     */
    public static function sendRateLimitHeaders($limit_info) {
        if (isset($limit_info['limit'])) {
            header('X-RateLimit-Limit: ' . $limit_info['limit']);
        }
        if (isset($limit_info['remaining'])) {
            header('X-RateLimit-Remaining: ' . $limit_info['remaining']);
        }
        if (isset($limit_info['window'])) {
            header('X-RateLimit-Window: ' . $limit_info['window'] . 's');
        }
        if (isset($limit_info['retry_after']) && $limit_info['retry_after'] !== null) {
            header('Retry-After: ' . $limit_info['retry_after']);
        }
    }

    /**
     * Retourne une réponse d'erreur 429 Too Many Requests
     * @param array $limit_info
     * @return array
     */
    public static function getTooManyRequestsResponse($limit_info) {
        http_response_code(429);

        return [
            'success' => false,
            'error' => 'Too many requests. Please slow down.',
            'retry_after' => $limit_info['retry_after'],
            'limit' => $limit_info['limit'],
            'window' => $limit_info['window'] . ' seconds'
        ];
    }
}
?>
