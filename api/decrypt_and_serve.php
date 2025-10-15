<?php

require_once __DIR__ . '/services/EncryptionService.php';
require_once __DIR__ . '/services/ChunkManager.php';
require_once __DIR__ . '/services/MappingDatabase.php';

function printUsage(): void
{
    echo "Usage: php decrypt_and_serve.php <file_id_or_filename> [options]\n\n";
    echo "Arguments:\n";
    echo "  file_id_or_filename   File ID or original filename to retrieve\n\n";
    echo "Options:\n";
    echo "  --key <key>           Encryption key (hex format, 64 chars)\n";
    echo "                        If not provided, uses ENCRYPTION_KEY env variable\n";
    echo "  --output <file>       Output file path (default: stdout)\n";
    echo "  --ftp-config <file>   FTP configuration file (default: ftp.json)\n";
    echo "  --local-dir <dir>     Local directory for chunks (default: ./chunks)\n";
    echo "  --info                Show file information without decrypting\n";
    echo "  --list                List all files in database\n";
    echo "  --stats               Show database statistics\n";
    echo "  --help                Show this help message\n\n";
    echo "Examples:\n";
    echo "  php decrypt_and_serve.php abc123def456 --output image.png\n";
    echo "  php decrypt_and_serve.php image.png --output decrypted.png\n";
    echo "  php decrypt_and_serve.php abc123def456 > output.png\n";
    echo "  php decrypt_and_serve.php --list\n";
    echo "  php decrypt_and_serve.php --stats\n";
    echo "  php decrypt_and_serve.php abc123def456 --info\n";
    exit(1);
}

function parseArguments(array $argv): array
{
    $options = [
        'identifier' => null,
        'key' => null,
        'output' => null,
        'ftp_config' => 'ftp.json',
        'local_dir' => './chunks',
        'info_only' => false,
        'list_files' => false,
        'show_stats' => false
    ];

    if (count($argv) < 2) {
        printUsage();
    }

    for ($i = 1; $i < count($argv); $i++) {
        switch ($argv[$i]) {
            case '--help':
                printUsage();
                break;
            case '--key':
                $options['key'] = $argv[++$i] ?? null;
                break;
            case '--output':
                $options['output'] = $argv[++$i] ?? null;
                break;
            case '--ftp-config':
                $options['ftp_config'] = $argv[++$i] ?? 'ftp.json';
                break;
            case '--local-dir':
                $options['local_dir'] = $argv[++$i] ?? './chunks';
                break;
            case '--info':
                $options['info_only'] = true;
                break;
            case '--list':
                $options['list_files'] = true;
                break;
            case '--stats':
                $options['show_stats'] = true;
                break;
            default:
                if ($options['identifier'] === null && !str_starts_with($argv[$i], '--')) {
                    $options['identifier'] = $argv[$i];
                }
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

    ftp_pasv($connection, true);

    return $connection;
}

function listFiles(MappingDatabase $db): void
{
    $mappings = $db->getAllMappings();

    if (empty($mappings)) {
        echo "No files found in database.\n";
        return;
    }

    echo "Files in database:\n";
    echo str_repeat("-", 100) . "\n";
    printf("%-64s %-30s %12s %8s\n", "File ID", "Original Filename", "Size", "Chunks");
    echo str_repeat("-", 100) . "\n";

    foreach ($mappings as $fileId => $mapping) {
        printf(
            "%-64s %-30s %12s %8d\n",
            $fileId,
            basename($mapping['original_filename']),
            number_format($mapping['original_size']) . ' B',
            $mapping['chunk_count']
        );
    }

    echo str_repeat("-", 100) . "\n";
    echo "Total files: " . count($mappings) . "\n";
}

function showStats(MappingDatabase $db): void
{
    $stats = $db->getStatistics();

    echo "Database Statistics:\n";
    echo str_repeat("-", 50) . "\n";
    echo "Total files:     {$stats['total_files']}\n";
    echo "Total size:      " . number_format($stats['total_size']) . " bytes (" .
         round($stats['total_size'] / 1048576, 2) . " MB)\n";
    echo "Total chunks:    {$stats['total_chunks']}\n";
    echo "Database path:   {$stats['database_path']}\n";
    echo "Database size:   " . number_format($stats['database_size']) . " bytes\n";
    echo str_repeat("-", 50) . "\n";
}

function showFileInfo(array $mapping): void
{
    echo "File Information:\n";
    echo str_repeat("-", 50) . "\n";
    echo "File ID:         {$mapping['file_id']}\n";
    echo "Original name:   {$mapping['original_filename']}\n";
    echo "MIME type:       {$mapping['mime_type']}\n";
    echo "Original size:   " . number_format($mapping['original_size']) . " bytes\n";
    echo "Chunk count:     {$mapping['chunk_count']}\n";
    echo "Chunk size:      " . number_format($mapping['chunk_size']) . " bytes\n";
    echo "Created at:      {$mapping['created_at']}\n";

    if (!empty($mapping['metadata'])) {
        echo "\nMetadata:\n";
        foreach ($mapping['metadata'] as $key => $value) {
            echo "  $key: " . (is_array($value) ? json_encode($value) : $value) . "\n";
        }
    }

    echo "\nChunks:\n";
    foreach ($mapping['chunks'] as $chunk) {
        echo "  [{$chunk['index']}] {$chunk['hash']} - " .
             number_format($chunk['size']) . " bytes (encrypted: " .
             number_format($chunk['encrypted_size']) . " bytes)\n";
    }
    echo str_repeat("-", 50) . "\n";
}

try {
    $options = parseArguments($argv);
    $mappingDb = new MappingDatabase();

    if ($options['list_files']) {
        listFiles($mappingDb);
        exit(0);
    }

    if ($options['show_stats']) {
        showStats($mappingDb);
        exit(0);
    }

    if ($options['identifier'] === null) {
        echo "ERROR: No file identifier provided.\n\n";
        printUsage();
    }

    $mapping = $mappingDb->getMappingByFileId($options['identifier']);
    if ($mapping === null) {
        $mapping = $mappingDb->getMappingByOriginalFilename($options['identifier']);
    }

    if ($mapping === null) {
        throw new Exception("File not found: {$options['identifier']}");
    }

    if ($options['info_only']) {
        showFileInfo($mapping);
        exit(0);
    }

    fprintf(STDERR, "Retrieving file: {$mapping['original_filename']}\n");
    fprintf(STDERR, "File ID: {$mapping['file_id']}\n");
    fprintf(STDERR, "Chunks to retrieve: {$mapping['chunk_count']}\n\n");

    $encryptionService = new EncryptionService($options['key']);
    $chunkManager = new ChunkManager($encryptionService);

    $storageLocation = $mapping['metadata']['storage_location'] ?? 'SanDisk/data';

    fprintf(STDERR, "Step 1: Downloading chunks from FTP...\n");

    $ftpConnection = connectFtp($options['ftp_config']);
    try {
        $chunks = $chunkManager->downloadChunksFromFtp(
            $mapping['chunks'],
            $ftpConnection,
            $storageLocation
        );
    } finally {
        ftp_close($ftpConnection);
    }

    fprintf(STDERR, "  Downloaded {$mapping['chunk_count']} chunks\n\n");

    fprintf(STDERR, "Step 2: Decrypting and reassembling file...\n");
    $decryptedData = $chunkManager->decryptAndReassemble($chunks);
    fprintf(STDERR, "  Decrypted size: " . number_format(strlen($decryptedData)) . " bytes\n\n");

    if ($options['output'] !== null) {
        file_put_contents($options['output'], $decryptedData);
        fprintf(STDERR, "SUCCESS! File saved to: {$options['output']}\n");
    } else {
        echo $decryptedData;
        fprintf(STDERR, "SUCCESS! File sent to stdout\n");
    }

} catch (Exception $e) {
    fprintf(STDERR, "ERROR: " . $e->getMessage() . "\n");
    exit(1);
}
