<?php

require_once __DIR__ . '/services/EncryptionService.php';
require_once __DIR__ . '/services/ChunkManager.php';
require_once __DIR__ . '/services/MappingDatabase.php';

function printUsage(): void
{
    echo "Usage: php encrypt_and_upload.php <input_file> [options]\n\n";
    echo "Options:\n";
    echo "  --key <key>           Encryption key (hex format, 64 chars)\n";
    echo "                        If not provided, uses encryption.key file\n";
    echo "  --chunk-size <bytes>  Chunk size in bytes (default: 1048576 = 1MB)\n";
    echo "  --ftp-config <file>   FTP configuration file (default: ftp.json)\n";
    echo "  --remote-dir <dir>    Remote directory on FTP (default: SanDisk/data)\n";
    echo "  --metadata <json>     Additional metadata as JSON string\n";
    echo "  --help                Show this help message\n\n";
    echo "Examples:\n";
    echo "  php encrypt_and_upload.php image.png\n";
    echo "  php encrypt_and_upload.php image.png --chunk-size 524288\n";
    echo "  php encrypt_and_upload.php image.png --metadata '{\"category\":\"nature\"}'\n";
    exit(1);
}

function parseArguments(array $argv): array
{
    $options = [
        'input_file' => null,
        'key' => null,
        'chunk_size' => 1048576,
        'ftp_config' => 'ftp.json',
        'remote_dir' => 'SanDisk/data',
        'metadata' => []
    ];

    if (count($argv) < 2) {
        printUsage();
    }

    $options['input_file'] = $argv[1];

    for ($i = 2; $i < count($argv); $i++) {
        switch ($argv[$i]) {
            case '--help':
                printUsage();
                break;
            case '--key':
                $options['key'] = $argv[++$i] ?? null;
                break;
            case '--chunk-size':
                $options['chunk_size'] = (int)($argv[++$i] ?? 1048576);
                break;
            case '--ftp-config':
                $options['ftp_config'] = $argv[++$i] ?? 'ftp.json';
                break;
            case '--remote-dir':
                $options['remote_dir'] = $argv[++$i] ?? '/encrypted_chunks';
                break;
            case '--metadata':
                $json = $argv[++$i] ?? '{}';
                $options['metadata'] = json_decode($json, true) ?? [];
                break;
        }
    }

    return $options;
}

function connectFtp(string $configFile)
{
    if (!file_exists($configFile)) {
        throw new Exception("FTP config file not found: $configFile");
    }

    $config = json_decode(file_get_contents($configFile), true);
    if (!$config) {
        throw new Exception("Invalid FTP config file");
    }

    $scheme = parse_url($config['host'], PHP_URL_SCHEME);
    $host = parse_url($config['host'], PHP_URL_HOST) ?: $config['host'];
    $port = parse_url($config['host'], PHP_URL_PORT) ?: 21;

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

    if (!ftp_login($connection, $config['user'], $config['password'])) {
        ftp_close($connection);
        throw new Exception("Failed to login to FTP server");
    }

    // Mode passif configurable (par défaut: true)
    $passiveMode = isset($config['passive']) ? (bool)$config['passive'] : true;
    ftp_pasv($connection, $passiveMode);

    return $connection;
}

try {
    $options = parseArguments($argv);

    if (!file_exists($options['input_file'])) {
        throw new Exception("Input file not found: {$options['input_file']}");
    }

    echo "Starting encryption and upload process...\n";
    echo "Input file: {$options['input_file']}\n";
    echo "File size: " . number_format(filesize($options['input_file'])) . " bytes\n";
    echo "Chunk size: " . number_format($options['chunk_size']) . " bytes\n\n";

    $encryptionService = new EncryptionService($options['key']);
    $chunkManager = new ChunkManager($encryptionService, $options['chunk_size']);
    $mappingDb = new MappingDatabase();

    echo "Step 1: Splitting and encrypting file...\n";
    $chunksData = $chunkManager->splitAndEncrypt($options['input_file']);
    echo "  Created {$chunksData['chunk_count']} chunks\n\n";

    echo "Step 2: Uploading chunks to FTP...\n";
    $ftpConnection = connectFtp($options['ftp_config']);

    try {
        $savedChunks = $chunkManager->uploadChunksToFtp(
            $chunksData,
            $ftpConnection,
            $options['remote_dir']
        );
        echo "  Uploaded {$savedChunks['chunk_count']} chunks\n\n";
    } finally {
        ftp_close($ftpConnection);
    }

    echo "Step 3: Saving mapping to database...\n";
    $fileMetadata = array_merge($options['metadata'], [
        'mime_type' => mime_content_type($options['input_file']),
        'storage_location' => $options['remote_dir']
    ]);

    $fileId = $mappingDb->addMapping($options['input_file'], $savedChunks, $fileMetadata);
    echo "  File ID: $fileId\n";
    echo "  Original filename: {$options['input_file']}\n\n";

    $stats = $mappingDb->getStatistics();
    echo "Database statistics:\n";
    echo "  Total files: {$stats['total_files']}\n";
    echo "  Total size: " . number_format($stats['total_size']) . " bytes\n";
    echo "  Total chunks: {$stats['total_chunks']}\n";
    echo "  Database path: {$stats['database_path']}\n\n";

    echo "SUCCESS! File encrypted and uploaded successfully.\n";
    echo "File ID: $fileId\n";
    echo "Use this ID to retrieve the file later.\n";

} catch (Exception $e) {
    echo "ERROR: " . $e->getMessage() . "\n";
    exit(1);
}
