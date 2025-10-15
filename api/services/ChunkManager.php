<?php

require_once __DIR__ . '/EncryptionService.php';

class ChunkManager
{
    private const DEFAULT_CHUNK_SIZE = 1048576;

    private int $chunkSize;
    private EncryptionService $encryptionService;

    public function __construct(EncryptionService $encryptionService, int $chunkSize = self::DEFAULT_CHUNK_SIZE)
    {
        $this->encryptionService = $encryptionService;
        $this->chunkSize = $chunkSize;
    }

    public function splitAndEncrypt(string $inputPath): array
    {
        if (!file_exists($inputPath)) {
            throw new Exception("Input file not found: $inputPath");
        }

        $fileSize = filesize($inputPath);
        $handle = fopen($inputPath, 'rb');
        if ($handle === false) {
            throw new Exception("Failed to open input file: $inputPath");
        }

        $chunks = [];
        $chunkIndex = 0;

        try {
            while (!feof($handle)) {
                $data = fread($handle, $this->chunkSize);
                if ($data === false) {
                    throw new Exception("Failed to read chunk $chunkIndex from file");
                }

                if (strlen($data) === 0) {
                    break;
                }

                $encryptedData = $this->encryptionService->encrypt($data);

                $chunkHash = hash('sha256', random_bytes(32));

                $chunks[] = [
                    'index' => $chunkIndex,
                    'hash' => $chunkHash,
                    'size' => strlen($data),
                    'encrypted_size' => strlen($encryptedData),
                    'data' => $encryptedData
                ];

                $chunkIndex++;
            }
        } finally {
            fclose($handle);
        }

        return [
            'chunks' => $chunks,
            'original_size' => $fileSize,
            'chunk_count' => count($chunks),
            'chunk_size' => $this->chunkSize
        ];
    }

    public function decryptAndReassemble(array $chunks): string
    {
        usort($chunks, fn($a, $b) => $a['index'] <=> $b['index']);

        $decryptedData = '';

        foreach ($chunks as $chunk) {
            $decrypted = $this->encryptionService->decrypt($chunk['data']);
            $decryptedData .= $decrypted;
        }

        return $decryptedData;
    }

    public function saveChunksToDirectory(array $chunksData, string $outputDir): array
    {
        if (!is_dir($outputDir)) {
            if (!mkdir($outputDir, 0755, true)) {
                throw new Exception("Failed to create output directory: $outputDir");
            }
        }

        $savedChunks = [];

        foreach ($chunksData['chunks'] as $chunk) {
            $chunkPath = $outputDir . DIRECTORY_SEPARATOR . $chunk['hash'];

            $result = file_put_contents($chunkPath, $chunk['data']);
            if ($result === false) {
                throw new Exception("Failed to write chunk: {$chunk['hash']}");
            }

            $savedChunks[] = [
                'index' => $chunk['index'],
                'hash' => $chunk['hash'],
                'size' => $chunk['size'],
                'encrypted_size' => $chunk['encrypted_size']
            ];
        }

        return [
            'chunks' => $savedChunks,
            'original_size' => $chunksData['original_size'],
            'chunk_count' => $chunksData['chunk_count'],
            'chunk_size' => $chunksData['chunk_size']
        ];
    }

    public function loadChunksFromDirectory(array $chunkMetadata, string $inputDir): array
    {
        $chunks = [];

        foreach ($chunkMetadata as $meta) {
            $chunkPath = $inputDir . DIRECTORY_SEPARATOR . $meta['hash'];

            if (!file_exists($chunkPath)) {
                throw new Exception("Chunk file not found: {$meta['hash']}");
            }

            $encryptedData = file_get_contents($chunkPath);
            if ($encryptedData === false) {
                throw new Exception("Failed to read chunk: {$meta['hash']}");
            }

            $chunks[] = [
                'index' => $meta['index'],
                'hash' => $meta['hash'],
                'data' => $encryptedData
            ];
        }

        return $chunks;
    }

    public function uploadChunksToFtp(array $chunksData, $ftpConnection, string $remoteDir): array
    {
        // Créer le répertoire distant s'il n'existe pas
        $this->ensureFtpDirectory($ftpConnection, $remoteDir);

        $savedChunks = [];

        foreach ($chunksData['chunks'] as $chunk) {
            $tempFile = tempnam(sys_get_temp_dir(), 'chunk_');
            file_put_contents($tempFile, $chunk['data']);

            $remotePath = rtrim($remoteDir, '/') . '/' . $chunk['hash'];

            $uploaded = @ftp_put($ftpConnection, $remotePath, $tempFile, FTP_BINARY);
            unlink($tempFile);

            if (!$uploaded) {
                throw new Exception("Failed to upload chunk to FTP: {$chunk['hash']}");
            }

            $savedChunks[] = [
                'index' => $chunk['index'],
                'hash' => $chunk['hash'],
                'size' => $chunk['size'],
                'encrypted_size' => $chunk['encrypted_size']
            ];
        }

        return [
            'chunks' => $savedChunks,
            'original_size' => $chunksData['original_size'],
            'chunk_count' => $chunksData['chunk_count'],
            'chunk_size' => $chunksData['chunk_size']
        ];
    }

    public function downloadChunksFromFtp(array $chunkMetadata, $ftpConnection, string $remoteDir): array
    {
        $chunks = [];

        foreach ($chunkMetadata as $meta) {
            $remotePath = rtrim($remoteDir, '/') . '/' . $meta['hash'];
            $tempFile = tempnam(sys_get_temp_dir(), 'chunk_');

            $downloaded = ftp_get($ftpConnection, $tempFile, $remotePath, FTP_BINARY);

            if (!$downloaded) {
                if (file_exists($tempFile)) {
                    unlink($tempFile);
                }
                throw new Exception("Failed to download chunk from FTP: {$meta['hash']}");
            }

            $encryptedData = file_get_contents($tempFile);
            unlink($tempFile);

            if ($encryptedData === false) {
                throw new Exception("Failed to read downloaded chunk: {$meta['hash']}");
            }

            $chunks[] = [
                'index' => $meta['index'],
                'hash' => $meta['hash'],
                'data' => $encryptedData
            ];
        }

        return $chunks;
    }

    private function ensureFtpDirectory($ftpConnection, string $directory): void
    {
        // Normaliser le chemin
        $directory = trim($directory, '/');
        if (empty($directory)) {
            return;
        }

        // Découper le chemin en parties
        $parts = explode('/', $directory);
        $currentPath = '';

        // Créer chaque niveau de répertoire
        foreach ($parts as $part) {
            $currentPath .= '/' . $part;

            // Vérifier si le répertoire existe déjà
            $contents = @ftp_nlist($ftpConnection, $currentPath);

            // Si ftp_nlist retourne false, le répertoire n'existe pas
            if ($contents === false) {
                // Créer le répertoire
                if (!@ftp_mkdir($ftpConnection, $currentPath)) {
                    // Ignorer l'erreur si le répertoire existe déjà
                    // (peut arriver en cas de race condition)
                    continue;
                }

                // Définir les permissions (facultatif)
                @ftp_chmod($ftpConnection, 0755, $currentPath);
            }
        }
    }
}
