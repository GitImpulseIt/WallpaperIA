<?php
require_once __DIR__ . '/../config/Config.php';
require_once __DIR__ . '/EncryptionService.php';
require_once __DIR__ . '/ChunkManager.php';
require_once __DIR__ . '/MappingDatabase.php';

/**
 * Service pour le téléchargement sécurisé depuis FTP avec déchiffrement
 */
class FtpService {
    private $ftp_config;
    private $ftp_connection;
    private $encryptionService;
    private $chunkManager;
    private $mappingDb;

    public function __construct() {
        $this->loadFtpConfig();
        $this->encryptionService = new EncryptionService();
        $this->chunkManager = new ChunkManager($this->encryptionService);
        $this->mappingDb = new MappingDatabase();
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
     * Télécharge et déchiffre un fichier depuis le FTP
     * Utilise obligatoirement le mapping database et le déchiffrement
     */
    public function getFileFromFtp($filename) {
        try {
            // 1. Rechercher le fichier dans la base de mapping
            $mapping = $this->mappingDb->getMappingByOriginalFilename($filename);

            if (!$mapping) {
                throw new Exception('File not found in mapping database');
            }

            // 2. Se connecter au FTP
            $ftpConnection = $this->connect();

            // 3. Déterminer le répertoire de stockage
            $storageLocation = $mapping['metadata']['storage_location'] ?? 'SanDisk/data';

            // 4. Télécharger les chunks depuis le FTP
            $chunks = $this->chunkManager->downloadChunksFromFtp(
                $mapping['chunks'],
                $ftpConnection,
                $storageLocation
            );

            // 5. Déchiffrer et réassembler le fichier
            $decryptedContent = $this->chunkManager->decryptAndReassemble($chunks);

            // 6. Retourner le contenu déchiffré
            return [
                'success' => true,
                'content' => $decryptedContent,
                'content_type' => $mapping['mime_type'],
                'filename' => $filename,
                'original_size' => $mapping['original_size']
            ];

        } catch (Exception $e) {
            return [
                'success' => false,
                'error' => $e->getMessage()
            ];
        } finally {
            $this->disconnect();
        }
    }

    /**
     * Vérifie si un fichier existe dans la base de mapping
     */
    public function fileExists($filename) {
        return $this->mappingDb->fileExists($filename);
    }

    /**
     * Obtient les informations d'un fichier sans le télécharger
     */
    public function getFileInfo($filename) {
        $mapping = $this->mappingDb->getMappingByOriginalFilename($filename);

        if (!$mapping) {
            return null;
        }

        return [
            'filename' => $mapping['original_filename'],
            'size' => $mapping['original_size'],
            'mime_type' => $mapping['mime_type'],
            'chunk_count' => $mapping['chunk_count'],
            'created_at' => $mapping['created_at']
        ];
    }

    public function __destruct() {
        $this->disconnect();
    }
}
?>
