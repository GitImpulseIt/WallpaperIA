<?php
/**
 * Service d'authentification
 * Gère la vérification des credentials via HTTP Basic Auth
 */
class AuthService {
    private $credentialsFile;

    public function __construct() {
        $this->credentialsFile = __DIR__ . '/../.credentials';
    }

    /**
     * Vérifie les credentials HTTP Basic Auth
     * @return array ['success' => bool, 'username' => string|null, 'error' => string|null]
     */
    public function authenticate() {
        // Méthode 1 : Variables PHP_AUTH_USER et PHP_AUTH_PW (configuration standard)
        if (isset($_SERVER['PHP_AUTH_USER']) && isset($_SERVER['PHP_AUTH_PW'])) {
            $username = $_SERVER['PHP_AUTH_USER'];
            $password = $_SERVER['PHP_AUTH_PW'];
        }
        // Méthode 2 : Header Authorization (pour Apache avec mod_rewrite)
        elseif (isset($_SERVER['HTTP_AUTHORIZATION'])) {
            $auth = $_SERVER['HTTP_AUTHORIZATION'];
            if (strpos($auth, 'Basic ') === 0) {
                $decoded = base64_decode(substr($auth, 6));
                $parts = explode(':', $decoded, 2);
                if (count($parts) === 2) {
                    $username = $parts[0];
                    $password = $parts[1];
                }
            }
        }
        // Méthode 3 : Header REDIRECT_HTTP_AUTHORIZATION (certaines configs Apache)
        elseif (isset($_SERVER['REDIRECT_HTTP_AUTHORIZATION'])) {
            $auth = $_SERVER['REDIRECT_HTTP_AUTHORIZATION'];
            if (strpos($auth, 'Basic ') === 0) {
                $decoded = base64_decode(substr($auth, 6));
                $parts = explode(':', $decoded, 2);
                if (count($parts) === 2) {
                    $username = $parts[0];
                    $password = $parts[1];
                }
            }
        }

        // Si aucune méthode n'a fonctionné
        if (!isset($username) || !isset($password)) {
            return [
                'success' => false,
                'username' => null,
                'error' => 'Authentication required'
            ];
        }

        // Vérifier que le fichier .credentials existe
        if (!file_exists($this->credentialsFile)) {
            return [
                'success' => false,
                'username' => null,
                'error' => 'Credentials file not found'
            ];
        }

        // Charger les credentials
        $credentials = $this->loadCredentials();

        // Vérifier si l'utilisateur existe
        if (!isset($credentials[$username])) {
            return [
                'success' => false,
                'username' => null,
                'error' => 'Invalid username or password'
            ];
        }

        // Calculer le hash SHA256 du mot de passe fourni
        $passwordHash = hash('sha256', $password);

        // Vérifier le hash
        if ($credentials[$username] !== $passwordHash) {
            return [
                'success' => false,
                'username' => null,
                'error' => 'Invalid username or password'
            ];
        }

        // Authentification réussie
        return [
            'success' => true,
            'username' => $username,
            'error' => null
        ];
    }

    /**
     * Charge les credentials depuis le fichier .credentials
     * Format: username:sha256_hash
     * @return array Associative array [username => hash]
     */
    private function loadCredentials() {
        $credentials = [];
        $lines = file($this->credentialsFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

        foreach ($lines as $line) {
            $line = trim($line);
            // Ignorer les commentaires
            if (empty($line) || $line[0] === '#') {
                continue;
            }

            $parts = explode(':', $line, 2);
            if (count($parts) === 2) {
                $username = trim($parts[0]);
                $hash = trim($parts[1]);
                $credentials[$username] = $hash;
            }
        }

        return $credentials;
    }

    /**
     * Envoie les headers pour demander l'authentification
     */
    public function requireAuth() {
        header('WWW-Authenticate: Basic realm="WallpaperIA API"');
        header('HTTP/1.0 401 Unauthorized');
    }
}
?>
