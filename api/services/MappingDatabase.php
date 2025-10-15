<?php

class MappingDatabase
{
    private string $databasePath;
    private array $mappings = [];
    private array $compactMappings = []; // Format compact stocké sur disque

    public function __construct(?string $databasePath = null)
    {
        if ($databasePath === null) {
            $databasePath = __DIR__ . '/../file_mappings.json';
        }

        $this->databasePath = $databasePath;
        $this->load();
    }

    /**
     * Convertit une structure compacte en structure complète
     * Format compact: {n, s, c, t, m, l, h, x}
     * Format complet: {file_id, original_filename, mime_type, original_size, chunk_count, chunk_size, created_at, metadata, chunks}
     */
    private function expandMapping(string $fileId, array $compact): array
    {
        $chunkCount = count($compact['h'] ?? []);
        $chunks = [];

        foreach (($compact['h'] ?? []) as $index => $hash) {
            // Le dernier chunk peut avoir une taille différente
            $isLastChunk = ($index === $chunkCount - 1);
            $chunkSize = $isLastChunk
                ? ($compact['s'] - ($index * $compact['c']))
                : $compact['c'];

            $chunks[] = [
                'index' => $index,
                'hash' => $hash,
                'size' => $chunkSize,
                'encrypted_size' => $chunkSize + 28  // IV (12) + TAG (16) = 28 bytes overhead
            ];
        }

        // Extraire les métadonnées
        $metadata = $compact['x'] ?? [];
        $metadata['storage_location'] = $compact['l'] ?? '';

        // Détecter upload_mode depuis storage_location
        if (!isset($metadata['upload_mode'])) {
            $metadata['upload_mode'] = str_starts_with($compact['l'] ?? '', './') ? 'local' : 'ftp';
        }

        return [
            'file_id' => $fileId,
            'original_filename' => $compact['n'],
            'mime_type' => $compact['m'] ?? 'application/octet-stream',
            'original_size' => $compact['s'],
            'chunk_count' => $chunkCount,
            'chunk_size' => $compact['c'],
            'created_at' => date('c', $compact['t']),
            'metadata' => $metadata,
            'chunks' => $chunks
        ];
    }


    public function getMappingByFileId(string $fileId): ?array
    {
        // Retourner depuis le cache mémoire si disponible
        if (isset($this->mappings[$fileId])) {
            return $this->mappings[$fileId];
        }

        // Sinon, charger depuis le format compact
        if (isset($this->compactMappings[$fileId])) {
            $this->mappings[$fileId] = $this->expandMapping($fileId, $this->compactMappings[$fileId]);
            return $this->mappings[$fileId];
        }

        return null;
    }

    public function getMappingByOriginalFilename(string $filename): ?array
    {
        // Parcourir les mappings compacts
        foreach ($this->compactMappings as $fileId => $compact) {
            if ($compact['n'] === $filename) {
                return $this->getMappingByFileId($fileId);
            }
        }
        return null;
    }

    public function getAllMappings(): array
    {
        // Charger tous les mappings depuis le format compact
        $all = [];
        foreach ($this->compactMappings as $fileId => $compact) {
            $all[$fileId] = $this->getMappingByFileId($fileId);
        }
        return $all;
    }


    public function fileExists(string $originalFilename): bool
    {
        foreach ($this->compactMappings as $compact) {
            if ($compact['n'] === $originalFilename) {
                return true;
            }
        }
        return false;
    }

    public function getChunkHashes(string $fileId): array
    {
        if (isset($this->compactMappings[$fileId])) {
            return $this->compactMappings[$fileId]['h'] ?? [];
        }
        return [];
    }

    private function load(): void
    {
        if (file_exists($this->databasePath)) {
            $content = file_get_contents($this->databasePath);
            if ($content !== false) {
                $data = json_decode($content, true);
                if (json_last_error() === JSON_ERROR_NONE && is_array($data)) {
                    $this->compactMappings = $data;
                    $this->mappings = []; // Le cache sera peuplé à la demande
                    return;
                }
            }
        }

        $this->compactMappings = [];
        $this->mappings = [];
    }


    public function getDatabasePath(): string
    {
        return $this->databasePath;
    }

    public function getTotalStorageSize(): int
    {
        $total = 0;
        foreach ($this->compactMappings as $compact) {
            $total += $compact['s'] ?? 0;
        }
        return $total;
    }

    public function getStatistics(): array
    {
        $totalChunks = 0;
        foreach ($this->compactMappings as $compact) {
            $totalChunks += count($compact['h'] ?? []);
        }

        return [
            'total_files' => count($this->compactMappings),
            'total_size' => $this->getTotalStorageSize(),
            'total_chunks' => $totalChunks,
            'database_path' => $this->databasePath,
            'database_size' => file_exists($this->databasePath) ? filesize($this->databasePath) : 0
        ];
    }
}
