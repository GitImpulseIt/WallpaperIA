# Format optimisé de file_mappings.json

## 🎯 Objectif

Réduction de **~65%** de l'espace disque en supprimant les redondances et informations calculables.

## 📊 Comparaison des formats

### Format ANCIEN (verbose)
```json
{
  "a1b2c3d4e5f6...": {
    "file_id": "a1b2c3d4e5f6...",
    "original_filename": "image.png",
    "mime_type": "image/png",
    "original_size": 2097152,
    "chunk_count": 2,
    "chunk_size": 1048576,
    "created_at": "2025-10-14T10:30:00+00:00",
    "metadata": {
      "upload_mode": "ftp",
      "storage_location": "SanDisk/data",
      "category": "nature"
    },
    "chunks": [
      {
        "index": 0,
        "hash": "abc123...",
        "size": 1048576,
        "encrypted_size": 1048604
      },
      {
        "index": 1,
        "hash": "xyz987...",
        "size": 1048576,
        "encrypted_size": 1048604
      }
    ]
  }
}
```
**Taille : ~1100 bytes**

### Format NOUVEAU (compact)
```json
{
  "a1b2c3d4e5f6...": {
    "n": "image.png",
    "s": 2097152,
    "c": 1048576,
    "t": 1728900600,
    "m": "image/png",
    "l": "SanDisk/data",
    "h": ["abc123...", "xyz987..."],
    "x": {"category": "nature"}
  }
}
```
**Taille : ~392 bytes** (-64.4%)

## 🔑 Mapping des champs

| Champ compact | Signification | Ancien champ | Notes |
|--------------|---------------|--------------|-------|
| `n` | Name | `original_filename` | Nom du fichier original |
| `s` | Size | `original_size` | Taille en bytes |
| `c` | Chunk size | `chunk_size` | Taille de chaque chunk |
| `t` | Timestamp | `created_at` | Unix timestamp (au lieu de ISO 8601) |
| `m` | MIME type | `mime_type` | Type MIME |
| `l` | Location | `metadata.storage_location` | Chemin de stockage |
| `h` | Hashes | `chunks[].hash` | Array simple de hashes |
| `x` | eXtra metadata | `metadata.*` | Métadonnées additionnelles |

## ❌ Champs supprimés (redondants ou calculables)

| Champ ancien | Raison de la suppression |
|-------------|-------------------------|
| `file_id` | Redondant (déjà la clé du tableau) |
| `chunk_count` | Calculable (`count($h)`) |
| `chunks[].index` | Redondant (index de l'array) |
| `chunks[].size` | Calculable (chunk_size sauf dernier) |
| `chunks[].encrypted_size` | Calculable (size + 28 bytes overhead) |
| `metadata.upload_mode` | Calculable (depuis `l`) |

## 🔄 Conversion automatique

La classe `MappingDatabase` gère automatiquement :

### En lecture (expand)
```php
$compact = ["n" => "image.png", "s" => 2097152, ...];
↓
$full = [
    "file_id" => "abc...",
    "original_filename" => "image.png",
    "original_size" => 2097152,
    "chunk_count" => 2,  // calculé
    "chunks" => [        // reconstruit
        ["index" => 0, "hash" => "...", "size" => 1048576, "encrypted_size" => 1048604],
        ["index" => 1, "hash" => "...", "size" => 1048576, "encrypted_size" => 1048604]
    ]
];
```

### En écriture (compact)
```php
$full = ["original_filename" => "image.png", "chunks" => [...], ...];
↓
$compact = ["n" => "image.png", "h" => ["hash1", "hash2"], ...];
```

## 💾 Optimisations supplémentaires

1. **JSON sans indentation** : `JSON_UNESCAPED_SLASHES` (sans `JSON_PRETTY_PRINT`)
2. **Timestamp Unix** : 10 chiffres au lieu de chaîne ISO 8601 (~20 caractères)
3. **Métadonnées nulles** : `x` = `null` si aucune métadonnée personnalisée
4. **Cache en mémoire** : Les mappings complets sont générés à la demande

## 📈 Gains d'espace par fichier

Pour un fichier avec 10 chunks :

| Élément | Ancien | Nouveau | Gain |
|---------|--------|---------|------|
| Structure de base | ~200 bytes | ~120 bytes | 40% |
| Par chunk | ~150 bytes | ~65 bytes | 57% |
| **Total (10 chunks)** | **~1700 bytes** | **~770 bytes** | **55%** |

Pour 1000 fichiers : **~930 KB économisés** !

## 🔧 Compatibilité

✅ **Tous les consommateurs sont compatibles** :
- `upload_endpoint.php` : Ajoute des mappings
- `decrypt_and_serve.php` : Lit des mappings
- `encrypt_and_upload.php` : Ajoute des mappings
- API endpoints : Transparent (MappingDatabase gère la conversion)

## 🚀 Migration

**Automatique** : Les anciens fichiers `file_mappings.json` au format verbose seront automatiquement lus et convertis au format compact lors de la première écriture.

Aucune action requise !
