<?php

require_once __DIR__ . '/EncryptionService.php';

class ChunkManager
{
    private EncryptionService $encryptionService;

    public function __construct(EncryptionService $encryptionService)
    {
        $this->encryptionService = $encryptionService;
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
}
