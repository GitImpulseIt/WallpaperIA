<?php
require_once __DIR__ . '/../config/Config.php';

/**
 * Service pour la gestion FTP
 */
class FtpService {
    private $ftp_config;

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
     * Valide le nom de fichier
     */
    private function validateFilename($filename) {
        if (!preg_match('/^[a-zA-Z0-9._-]+$/', $filename)) {
            throw new Exception('Invalid filename format');
        }
    }

    /**
     * Obtient un fichier depuis le serveur FTP
     */
    public function getFileFromFtp($filename) {
        try {
            if (!$this->ftp_config) {
                throw new Exception('FTP configuration not loaded');
            }

            $this->validateFilename($filename);

            // Create FTP context
            $context = stream_context_create([
                'ftp' => [
                    'method' => 'ftp',
                    'protocol_version' => 1.0,
                ]
            ]);

            // Build the full FTP URL with credentials
            $parsed_url = parse_url($this->ftp_config['host']);
            $ftp_url_with_auth = $parsed_url['scheme'] . '://' .
                                urlencode($this->ftp_config['user']) . ':' .
                                urlencode($this->ftp_config['password']) . '@' .
                                $parsed_url['host'] .
                                (isset($parsed_url['port']) ? ':' . $parsed_url['port'] : '') .
                                $this->ftp_config['directory'] . '/' . $filename;

            // Try to get file content
            $file_content = file_get_contents($ftp_url_with_auth, false, $context);

            if ($file_content === false) {
                throw new Exception('File not found or FTP error');
            }

            // Determine content type
            $ext = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
            $content_types = Config::getContentTypes();
            $content_type = $content_types[$ext] ?? 'application/octet-stream';

            return [
                'success' => true,
                'content' => $file_content,
                'content_type' => $content_type,
                'filename' => $filename
            ];

        } catch (Exception $e) {
            return [
                'success' => false,
                'error' => $e->getMessage()
            ];
        }
    }
}
?>