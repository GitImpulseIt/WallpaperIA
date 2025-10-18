<?php
require_once __DIR__ . '/../config/Config.php';

/**
 * Service simple pour le téléchargement direct depuis FTP sécurisé
 */
class FtpService {
    private $ftp_config;
    private $ftp_connection;

    public function __construct() {
        $this->loadFtpConfig();
    }

    /**
     * Charge la configuration FTP depuis ftp.json
     */
    private function loadFtpConfig() {
        $paths = Config::getPaths();
        $ftp_file = $paths['ftp_config'];

        if (file_exists($ftp_file)) {
            $this->ftp_config = json_decode(file_get_contents($ftp_file), true);
        } else {
            throw new Exception('FTP configuration file not found');
        }
    }

    /**
     * Connexion au serveur FTP/FTPES
     */
    private function connect() {
        if ($this->ftp_connection) {
            return $this->ftp_connection;
        }

        $scheme = parse_url($this->ftp_config['host'], PHP_URL_SCHEME);
        $host = parse_url($this->ftp_config['host'], PHP_URL_HOST) ?: $this->ftp_config['host'];
        $port = parse_url($this->ftp_config['host'], PHP_URL_PORT) ?: 21;

        // Utiliser ftp_ssl_connect pour FTPES (FTP over explicit TLS/SSL)
        $useSsl = ($scheme === 'ftpes' || $scheme === 'ftps');

        if ($useSsl) {
            $connection = @ftp_ssl_connect($host, $port, 30);
        } else {
            $connection = @ftp_connect($host, $port, 30);
        }

        if (!$connection) {
            $protocol = $useSsl ? 'FTPES' : 'FTP';
            throw new Exception("Failed to connect to $protocol server: $host:$port");
        }

        if (!ftp_login($connection, $this->ftp_config['user'], $this->ftp_config['password'])) {
            ftp_close($connection);
            throw new Exception("Failed to login to FTP server");
        }

        // Mode passif configurable (par défaut: true)
        $passiveMode = isset($this->ftp_config['passive']) ? (bool)$this->ftp_config['passive'] : true;
        ftp_pasv($connection, $passiveMode);

        $this->ftp_connection = $connection;
        return $connection;
    }

    /**
     * Déconnexion du serveur FTP
     */
    private function disconnect() {
        if ($this->ftp_connection) {
            ftp_close($this->ftp_connection);
            $this->ftp_connection = null;
        }
    }

    /**
     * Télécharge un fichier directement depuis le FTP
     * @param string $filename Nom du fichier à télécharger
     * @return array ['success' => bool, 'content' => string|null, 'content_type' => string|null, 'filename' => string, 'error' => string|null]
     */
    public function getFileFromFtp($filename) {
        try {
            // Se connecter au FTP
            $ftpConnection = $this->connect();

            // Construire le chemin complet sur le FTP
            $ftpDirectory = $this->ftp_config['directory'] ?? '/wallpapers';
            $remotePath = rtrim($ftpDirectory, '/') . '/' . $filename;

            // Créer un fichier temporaire local
            $tempFile = tempnam(sys_get_temp_dir(), 'ftp_');

            // Télécharger le fichier depuis le FTP
            if (!ftp_get($ftpConnection, $tempFile, $remotePath, FTP_BINARY)) {
                unlink($tempFile);
                throw new Exception("Failed to download file from FTP: $remotePath");
            }

            // Lire le contenu du fichier
            $content = file_get_contents($tempFile);

            // Déterminer le type MIME
            $mimeType = $this->getMimeType($filename);

            // Supprimer le fichier temporaire
            unlink($tempFile);

            return [
                'success' => true,
                'content' => $content,
                'content_type' => $mimeType,
                'filename' => $filename,
                'error' => null
            ];

        } catch (Exception $e) {
            return [
                'success' => false,
                'content' => null,
                'content_type' => null,
                'filename' => $filename,
                'error' => $e->getMessage()
            ];
        } finally {
            $this->disconnect();
        }
    }

    /**
     * Vérifie si un fichier existe sur le FTP
     * @param string $filename Nom du fichier
     * @return bool
     */
    public function fileExists($filename) {
        try {
            $ftpConnection = $this->connect();
            $ftpDirectory = $this->ftp_config['directory'] ?? '/wallpapers';
            $remotePath = rtrim($ftpDirectory, '/') . '/' . $filename;

            // Essayer d'obtenir la taille du fichier (méthode rapide pour vérifier l'existence)
            $size = ftp_size($ftpConnection, $remotePath);

            return $size !== -1;

        } catch (Exception $e) {
            return false;
        } finally {
            $this->disconnect();
        }
    }

    /**
     * Obtient les informations d'un fichier sans le télécharger
     * @param string $filename Nom du fichier
     * @return array|null
     */
    public function getFileInfo($filename) {
        try {
            $ftpConnection = $this->connect();
            $ftpDirectory = $this->ftp_config['directory'] ?? '/wallpapers';
            $remotePath = rtrim($ftpDirectory, '/') . '/' . $filename;

            $size = ftp_size($ftpConnection, $remotePath);

            if ($size === -1) {
                return null;
            }

            $mtime = ftp_mdtm($ftpConnection, $remotePath);

            return [
                'filename' => $filename,
                'size' => $size,
                'mime_type' => $this->getMimeType($filename),
                'modified_time' => $mtime !== -1 ? date('Y-m-d H:i:s', $mtime) : null
            ];

        } catch (Exception $e) {
            return null;
        } finally {
            $this->disconnect();
        }
    }

    /**
     * Détermine le type MIME d'un fichier basé sur son extension
     * @param string $filename
     * @return string
     */
    private function getMimeType($filename) {
        $extension = strtolower(pathinfo($filename, PATHINFO_EXTENSION));

        $mimeTypes = [
            'jpg' => 'image/jpeg',
            'jpeg' => 'image/jpeg',
            'png' => 'image/png',
            'gif' => 'image/gif',
            'webp' => 'image/webp',
            'bmp' => 'image/bmp',
            'svg' => 'image/svg+xml',
            'ico' => 'image/x-icon'
        ];

        return $mimeTypes[$extension] ?? 'application/octet-stream';
    }

    public function __destruct() {
        $this->disconnect();
    }
}
?>
