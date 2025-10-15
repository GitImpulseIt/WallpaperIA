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

    /**
     * Convertit une structure complète en structure compacte
     * Réduit la taille de ~70% en supprimant les redondances
     */
    private function compactMapping(array $full): array
    {
        // Extraire les hashes uniquement
        $hashes = array_column($full['chunks'], 'hash');

        // Nettoyer les métadonnées
        $metadata = $full['metadata'] ?? [];
        $location = $metadata['storage_location'] ?? '';
        unset($metadata['storage_location'], $metadata['mime_type'], $metadata['upload_mode']);

        // Supprimer les métadonnées vides ou calculables
        $metadata = array_filter($metadata, fn($v) => $v !== null && $v !== '');

        $compact = [
            'n' => $full['original_filename'],              // name
            's' => $full['original_size'],                  // size
            'c' => $full['chunk_size'],                     // chunk_size
            't' => isset($full['created_at']) ? strtotime($full['created_at']) : time(), // timestamp
            'm' => $full['mime_type'],                      // mime
            'l' => $location,                               // location
            'h' => $hashes                                  // hashes array
        ];

        // Ajouter 'x' seulement si des métadonnées existent
        if (!empty($metadata)) {
            $compact['x'] = $metadata;
        }

        return $compact;
    }

    public function addMapping(string $originalFilename, array $chunkMetadata, array $fileMetadata = []): string
    {
        $fileId = hash('sha256', $originalFilename . microtime(true) . random_bytes(16));

        // Créer la structure complète
        $mapping = [
            'file_id' => $fileId,
            'original_filename' => $originalFilename,
            'mime_type' => $fileMetadata['mime_type'] ?? 'application/octet-stream',
            'original_size' => $chunkMetadata['original_size'],
            'chunk_count' => $chunkMetadata['chunk_count'],
            'chunk_size' => $chunkMetadata['chunk_size'],
            'chunks' => $chunkMetadata['chunks'],
            'created_at' => date('c'),
            'metadata' => $fileMetadata
        ];

        // Stocker en mémoire au format complet
        $this->mappings[$fileId] = $mapping;

        // Convertir en format compact pour le disque
        $this->compactMappings[$fileId] = $this->compactMapping($mapping);

        $this->save();

        return $fileId;
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

    public function removeMapping(string $fileId): bool
    {
        if (isset($this->compactMappings[$fileId])) {
            unset($this->compactMappings[$fileId]);
            unset($this->mappings[$fileId]);
            $this->save();
            return true;
        }
        return false;
    }

    public function updateMetadata(string $fileId, array $metadata): bool
    {
        $mapping = $this->getMappingByFileId($fileId);
        if ($mapping === null) {
            return false;
        }

        $mapping['metadata'] = array_merge($mapping['metadata'] ?? [], $metadata);
        $this->mappings[$fileId] = $mapping;
        $this->compactMappings[$fileId] = $this->compactMapping($mapping);
        $this->save();

        return true;
    }

    public function searchByMetadata(string $key, $value): array
    {
        $results = [];
        foreach ($this->compactMappings as $fileId => $compact) {
            $extra = $compact['x'] ?? [];
            if (isset($extra[$key]) && $extra[$key] === $value) {
                $results[$fileId] = $this->getMappingByFileId($fileId);
            }
        }
        return $results;
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

    private function save(): bool
    {
        $directory = dirname($this->databasePath);
        if (!is_dir($directory)) {
            if (!mkdir($directory, 0755, true)) {
                throw new Exception("Failed to create directory: $directory");
            }
        }

        // Sauvegarder au format compact sans pretty print pour économiser de l'espace
        $json = json_encode($this->compactMappings, JSON_UNESCAPED_SLASHES);
        if ($json === false) {
            throw new Exception("Failed to encode mappings to JSON");
        }

        $tempFile = $this->databasePath . '.tmp';
        $result = file_put_contents($tempFile, $json, LOCK_EX);
        if ($result === false) {
            throw new Exception("Failed to write mapping database");
        }

        if (!rename($tempFile, $this->databasePath)) {
            unlink($tempFile);
            throw new Exception("Failed to update mapping database");
        }

        return true;
    }

    public function exportToArray(): array
    {
        return [
            'version' => '2.0',
            'format' => 'compact',
            'exported_at' => date('c'),
            'file_count' => count($this->compactMappings),
            'mappings' => $this->getAllMappings() // Export au format complet
        ];
    }

    public function importFromArray(array $data, bool $overwrite = false): int
    {
        if (!isset($data['mappings']) || !is_array($data['mappings'])) {
            throw new Exception("Invalid import data: 'mappings' key not found");
        }

        $imported = 0;
        foreach ($data['mappings'] as $fileId => $mapping) {
            if (!$overwrite && isset($this->compactMappings[$fileId])) {
                continue;
            }

            // Convertir en format compact
            $this->mappings[$fileId] = $mapping;
            $this->compactMappings[$fileId] = $this->compactMapping($mapping);
            $imported++;
        }

        if ($imported > 0) {
            $this->save();
        }

        return $imported;
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
