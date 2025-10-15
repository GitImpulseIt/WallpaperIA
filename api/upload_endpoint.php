<?php

@ini_set('memory_limit', '1024M');
@ini_set('post_max_size', '500M');
@ini_set('upload_max_filesize', '500M');
@ini_set('max_execution_time', '300');
@ini_set('max_input_time', '300');

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    exit(0);
}

require_once __DIR__ . '/services/EncryptionService.php';
require_once __DIR__ . '/services/ChunkManager.php';
require_once __DIR__ . '/services/MappingDatabase.php';

function jsonResponse($data, $statusCode = 200) {
    http_response_code($statusCode);
    echo json_encode($data);
    exit;
}

function jsonError($message, $statusCode = 400) {
    jsonResponse(['success' => false, 'error' => $message], $statusCode);
}

function connectFtp(string $configFile) {
    if (!file_exists($configFile)) {
        throw new Exception("FTP config file not found");
    }

    $config = json_decode(file_get_contents($configFile), true);
    if (!$config) {
        throw new Exception("Invalid FTP config file");
    }

    $host = parse_url($config['host'], PHP_URL_HOST) ?: $config['host'];
    $port = parse_url($config['host'], PHP_URL_PORT) ?: 21;

    $connection = ftp_connect($host, $port, 30);
    if (!$connection) {
        throw new Exception("Failed to connect to FTP server");
    }

    if (!ftp_login($connection, $config['user'], $config['password'])) {
        ftp_close($connection);
        throw new Exception("Failed to login to FTP server");
    }

    ftp_pasv($connection, true);

    return $connection;
}

try {
    if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
        jsonError('Method not allowed', 405);
    }

    if (!isset($_FILES['file']) || $_FILES['file']['error'] !== UPLOAD_ERR_OK) {
        $errorMsg = isset($_FILES['file']) ? 'Upload error: ' . $_FILES['file']['error'] : 'No file uploaded';
        jsonError($errorMsg);
    }

    $uploadedFile = $_FILES['file'];
    $originalFilename = $uploadedFile['name'];
    $tmpPath = $uploadedFile['tmp_name'];
    $fileSize = $uploadedFile['size'];

    $chunkSize = isset($_POST['chunk_size']) ? (int)$_POST['chunk_size'] : 1048576;
    $metadata = isset($_POST['metadata']) ? json_decode($_POST['metadata'], true) : [];
    $remoteDir = isset($_POST['remote_dir']) ? $_POST['remote_dir'] : 'SanDisk/encrypted_chunks';

    $encryptionService = new EncryptionService();
    $chunkManager = new ChunkManager($encryptionService, $chunkSize);
    $mappingDb = new MappingDatabase();

    $chunksData = $chunkManager->splitAndEncrypt($tmpPath);

    // Upload vers FTP uniquement
    $ftpConnection = connectFtp(__DIR__ . '/ftp.json');
    try {
        $savedChunks = $chunkManager->uploadChunksToFtp($chunksData, $ftpConnection, $remoteDir);
    } finally {
        ftp_close($ftpConnection);
    }

    // Préparer les métadonnées avec storage_location et mime_type
    $fileMetadata = $metadata;
    $fileMetadata['storage_location'] = $remoteDir;
    $fileMetadata['mime_type'] = mime_content_type($tmpPath);

    $fileId = $mappingDb->addMapping($originalFilename, $savedChunks, $fileMetadata);

    $stats = $mappingDb->getStatistics();

    jsonResponse([
        'success' => true,
        'file_id' => $fileId,
        'original_filename' => $originalFilename,
        'original_size' => $fileSize,
        'chunk_count' => $savedChunks['chunk_count'],
        'encrypted_size' => array_sum(array_column($savedChunks['chunks'], 'encrypted_size')),
        'stats' => $stats
    ]);

} catch (Exception $e) {
    jsonError($e->getMessage(), 500);
}
