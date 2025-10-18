<?php
/**
 * ComfyUI Wallpaper Uploader
 *
 * Script automatique qui :
 * 1. Parcourt le répertoire de sortie ComfyUI
 * 2. Extrait la catégorie (nom du sous-répertoire)
 * 3. Extrait le nom de base de l'image (sans l'indice de suffixe)
 * 4. Upload l'image vers le FTP
 * 5. Enregistre l'image dans l'API de production
 */

class ComfyUIUploader {
    private $config;
    private $ftpConnection;
    private $uploadedFiles = [];
    private $logFile;

    public function __construct($configFile = 'config.json') {
        $this->loadConfig($configFile);
        $this->logFile = __DIR__ . '/upload.log';
    }

    /**
     * Charge la configuration depuis le fichier JSON
     */
    private function loadConfig($configFile) {
        if (!file_exists($configFile)) {
            throw new Exception("Configuration file not found: $configFile");
        }

        $this->config = json_decode(file_get_contents($configFile), true);

        if (!$this->config) {
            throw new Exception("Invalid JSON in configuration file");
        }

        // Vérifier les clés requises
        $required = ['comfyui_output_dir', 'ftp', 'api'];
        foreach ($required as $key) {
            if (!isset($this->config[$key])) {
                throw new Exception("Missing required configuration: $key");
            }
        }
    }

    /**
     * Log un message dans le fichier de log
     */
    private function log($message, $level = 'INFO') {
        $timestamp = date('Y-m-d H:i:s');
        $logMessage = "[$timestamp] [$level] $message\n";
        file_put_contents($this->logFile, $logMessage, FILE_APPEND);
        echo $logMessage;
    }

    /**
     * Connexion au serveur FTP
     */
    private function connectFtp() {
        if ($this->ftpConnection) {
            return $this->ftpConnection;
        }

        $ftpConfig = $this->config['ftp'];
        $scheme = parse_url($ftpConfig['host'], PHP_URL_SCHEME);
        $host = parse_url($ftpConfig['host'], PHP_URL_HOST) ?: $ftpConfig['host'];
        $port = parse_url($ftpConfig['host'], PHP_URL_PORT) ?: 21;

        // Utiliser ftp_ssl_connect pour FTPES
        $useSsl = ($scheme === 'ftpes' || $scheme === 'ftps');

        if ($useSsl) {
            $connection = @ftp_ssl_connect($host, $port, 30);
        } else {
            $connection = @ftp_connect($host, $port, 30);
        }

        if (!$connection) {
            throw new Exception("Failed to connect to FTP server: $host:$port");
        }

        if (!ftp_login($connection, $ftpConfig['user'], $ftpConfig['password'])) {
            ftp_close($connection);
            throw new Exception("Failed to login to FTP server");
        }

        // Mode passif
        $passiveMode = isset($ftpConfig['passive']) ? (bool)$ftpConfig['passive'] : true;
        ftp_pasv($connection, $passiveMode);

        $this->ftpConnection = $connection;
        $this->log("Connected to FTP server: $host");

        return $connection;
    }

    /**
     * Déconnexion du serveur FTP
     */
    private function disconnectFtp() {
        if ($this->ftpConnection) {
            ftp_close($this->ftpConnection);
            $this->ftpConnection = null;
            $this->log("Disconnected from FTP server");
        }
    }

    /**
     * Upload un fichier vers le FTP
     * Tous les fichiers sont uploadés dans le même répertoire (pas de sous-répertoires)
     */
    private function uploadToFtp($localFile, $filename) {
        $ftpConnection = $this->connectFtp();
        $ftpConfig = $this->config['ftp'];
        $remoteDirectory = $ftpConfig['remote_directory'] ?? '/wallpapers';

        // Construire le chemin distant complet (fichier directement dans le répertoire racine)
        $remotePath = rtrim($remoteDirectory, '/') . '/' . $filename;

        // Upload du fichier
        if (!ftp_put($ftpConnection, $remotePath, $localFile, FTP_BINARY)) {
            throw new Exception("Failed to upload file to FTP: $remotePath");
        }

        $this->log("Uploaded to FTP: $remotePath");
        return true;
    }

    /**
     * Enregistre l'image dans l'API de production
     */
    private function registerInApi($category, $filename, $date) {
        $apiConfig = $this->config['api'];

        $data = [
            'category' => $category,
            'filename' => $filename,
            'date' => $date
        ];

        $ch = curl_init($apiConfig['url']);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_POST, true);
        curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($data));
        curl_setopt($ch, CURLOPT_HTTPHEADER, [
            'Content-Type: application/json'
        ]);
        curl_setopt($ch, CURLOPT_USERPWD, $apiConfig['username'] . ':' . $apiConfig['password']);
        curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
        curl_setopt($ch, CURLOPT_TIMEOUT, 30);

        $response = curl_exec($ch);
        $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        $curlError = curl_error($ch);
        curl_close($ch);

        if ($response === false || $httpCode === 0) {
            throw new Exception("API connection failed: $curlError");
        }

        if ($httpCode !== 200) {
            throw new Exception("API registration failed (HTTP $httpCode): $response");
        }

        $result = json_decode($response, true);

        if (!$result || !isset($result['success']) || !$result['success']) {
            $errorMsg = $result['message'] ?? 'Unknown error';
            $fullResponse = $response ? " (Response: $response)" : '';
            throw new Exception("API registration failed: $errorMsg$fullResponse");
        }

        $this->log("Registered in API: $category / $filename");
        return $result;
    }

    /**
     * Parse le nom de fichier pour extraire le nom de base (sans l'indice de suffixe)
     * Exemples:
     *   "neon_city_night_00001_.png" -> "neon_city_night.png"
     *   "cyberpunk_street_00042_.png" -> "cyberpunk_street.png"
     */
    private function extractBaseName($filename) {
        // Pattern pour détecter les indices ComfyUI: _XXXXX_ avant l'extension
        $pattern = '/^(.+?)_\d{5}_\.([a-z]+)$/i';

        if (preg_match($pattern, $filename, $matches)) {
            return $matches[1] . '.' . $matches[2];
        }

        // Si le pattern ne correspond pas, retourner le nom original
        return $filename;
    }

    /**
     * Parcourt le répertoire ComfyUI et traite les images
     */
    public function processImages($dryRun = false) {
        $outputDir = $this->config['comfyui_output_dir'];

        if (!is_dir($outputDir)) {
            throw new Exception("ComfyUI output directory not found: $outputDir");
        }

        $this->log("Starting image processing from: $outputDir");
        if ($dryRun) {
            $this->log("DRY RUN MODE - No actual upload will occur");
        }

        $currentDate = date('d/m/Y');
        $processedCount = 0;
        $errorCount = 0;

        // Parcourir les sous-répertoires (catégories)
        $categories = array_diff(scandir($outputDir), ['.', '..']);

        foreach ($categories as $category) {
            $categoryPath = $outputDir . DIRECTORY_SEPARATOR . $category;

            // Ignorer si ce n'est pas un répertoire
            if (!is_dir($categoryPath)) {
                continue;
            }

            $this->log("Processing category: $category");

            // Parcourir les images dans la catégorie
            $files = array_diff(scandir($categoryPath), ['.', '..']);

            foreach ($files as $file) {
                $filePath = $categoryPath . DIRECTORY_SEPARATOR . $file;

                // Ignorer si ce n'est pas un fichier
                if (!is_file($filePath)) {
                    continue;
                }

                // Vérifier que c'est une image
                $extension = strtolower(pathinfo($file, PATHINFO_EXTENSION));
                $validExtensions = ['png', 'jpg', 'jpeg', 'webp'];

                if (!in_array($extension, $validExtensions)) {
                    $this->log("Skipping non-image file: $file", 'WARN');
                    continue;
                }

                try {
                    // Extraire le nom de base (sans l'indice)
                    $baseName = $this->extractBaseName($file);

                    // Vérifier si déjà uploadé dans cette session
                    if (in_array($baseName, $this->uploadedFiles)) {
                        $this->log("Already uploaded in this session: $baseName", 'SKIP');
                        continue;
                    }

                    $this->log("Processing: $file -> $baseName (category: $category)");

                    if (!$dryRun) {
                        // Upload vers le FTP (tous les fichiers dans le même répertoire)
                        $this->uploadToFtp($filePath, $baseName);

                        // Enregistrer dans l'API avec la catégorie
                        $this->registerInApi($category, $baseName, $currentDate);

                        // Marquer comme uploadé
                        $this->uploadedFiles[] = $baseName;
                    } else {
                        $this->log("[DRY RUN] Would upload: $baseName (category: $category)");
                    }

                    $processedCount++;

                } catch (Exception $e) {
                    $this->log("Error processing $file: " . $e->getMessage(), 'ERROR');
                    $errorCount++;
                }
            }
        }

        $this->log("Processing completed: $processedCount files processed, $errorCount errors");

        return [
            'processed' => $processedCount,
            'errors' => $errorCount,
            'uploaded_files' => $this->uploadedFiles
        ];
    }

    /**
     * Destructeur - ferme la connexion FTP
     */
    public function __destruct() {
        $this->disconnectFtp();
    }
}

// Point d'entrée du script
if (php_sapi_name() === 'cli') {
    try {
        // Vérifier les arguments
        $dryRun = in_array('--dry-run', $argv);
        $configFile = 'config.json';

        // Chercher un fichier de config personnalisé
        foreach ($argv as $arg) {
            if (strpos($arg, '--config=') === 0) {
                $configFile = substr($arg, 9);
            }
        }

        echo "ComfyUI Wallpaper Uploader\n";
        echo "==========================\n\n";

        $uploader = new ComfyUIUploader($configFile);
        $result = $uploader->processImages($dryRun);

        echo "\n";
        echo "Summary:\n";
        echo "- Processed: {$result['processed']} files\n";
        echo "- Errors: {$result['errors']} files\n";
        echo "- Uploaded: " . count($result['uploaded_files']) . " unique files\n";

        exit($result['errors'] > 0 ? 1 : 0);

    } catch (Exception $e) {
        echo "FATAL ERROR: " . $e->getMessage() . "\n";
        exit(1);
    }
} else {
    echo "This script must be run from the command line.\n";
    exit(1);
}
?>
