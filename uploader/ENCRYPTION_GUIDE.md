# 🔐 Guide du système de chiffrement et chunking

Ce système permet de sécuriser vos fichiers sur un serveur FTP non sécurisé en les découpant en chunks chiffrés avec des noms de fichiers aléatoires.

## 🎯 Objectifs de sécurité

### Protection contre l'interception FTP
- **Problème** : FTP (non-SFTP) transmet les mots de passe en clair
- **Solution** : Même si un attaquant intercepte vos identifiants FTP, il ne peut pas :
  - Identifier le type de fichiers stockés (pas d'extension)
  - Connaître les noms originaux des fichiers
  - Déterminer la taille réelle des fichiers
  - Lire le contenu (tout est chiffré)
  - Reconstituer les fichiers (table de correspondance sur serveur PHP sécurisé)

### Architecture de sécurité
```
┌─────────────────┐      FTP non sécurisé      ┌──────────────────┐
│   Serveur PHP   │ ──────────────────────────> │   Serveur FTP    │
│   (OVH HTTPS)   │                             │   (Box Internet) │
│                 │                             │                  │
│ - Clé de chiff. │                             │ - Chunks chiffrés│
│ - Table mapping │                             │ - Noms en hash   │
│ - file_mappings │                             │ - Pas d'extension│
└─────────────────┘                             └──────────────────┘
        ↑
        │ HTTPS sécurisé
        │
   ┌────────────┐
   │   Client   │
   │ WallpaperIA│
   └────────────┘
```

## 🔧 Installation

### 1. Génération de la clé de chiffrement

```bash
php -r "echo bin2hex(random_bytes(32)) . PHP_EOL;" > encryption.key
```

Cette commande génère une clé de 64 caractères hexadécimaux (256 bits).

### 2. Configuration de la variable d'environnement

**Linux/Mac :**
```bash
export ENCRYPTION_KEY=$(cat encryption.key)
```

**Windows :**
```cmd
set /p ENCRYPTION_KEY=<encryption.key
```

**Ou ajoutez-la dans votre fichier `.env` :**
```env
ENCRYPTION_KEY=votre_cle_de_64_caracteres_hex
```

### 3. Vérification des dépendances PHP

Le système nécessite :
- PHP 7.4+ (8.0+ recommandé)
- Extension `openssl` (activée par défaut)
- Extension `ftp` pour l'upload FTP
- Extension `fileinfo` pour la détection MIME

```bash
php -m | grep -E "openssl|ftp|fileinfo"
```

## 📤 Upload de fichiers

### Utilisation de base

```bash
# Upload avec FTP (défaut)
php api/encrypt_and_upload.php /path/to/image.png

# Upload local (pour tests sans FTP)
php api/encrypt_and_upload.php /path/to/image.png --local-only
```

### Options avancées

```bash
# Spécifier la taille des chunks (2 MB au lieu de 1 MB par défaut)
php api/encrypt_and_upload.php image.png --chunk-size 2097152

# Spécifier une clé de chiffrement différente
php api/encrypt_and_upload.php image.png --key 0123456789abcdef...

# Ajouter des métadonnées personnalisées
php api/encrypt_and_upload.php image.png --metadata '{"category":"nature","date":"2025-10-14"}'

# Changer le répertoire distant sur le FTP
php api/encrypt_and_upload.php image.png --remote-dir /my_custom_folder

# Utiliser un fichier de config FTP différent
php api/encrypt_and_upload.php image.png --ftp-config /path/to/my_ftp.json
```

### Sortie du script

```
Starting encryption and upload process...
Input file: image.png
File size: 1,234,567 bytes
Chunk size: 1,048,576 bytes

Step 1: Splitting and encrypting file...
  Created 2 chunks

Step 2: Uploading chunks to FTP...
  Uploaded 2 chunks

Step 3: Saving mapping to database...
  File ID: a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0u1v2w3x4y5z6a7b8c9d0e1f2
  Original filename: image.png

Database statistics:
  Total files: 1
  Total size: 1,234,567 bytes
  Total chunks: 2
  Database path: /path/to/api/file_mappings.json

SUCCESS! File encrypted and uploaded successfully.
File ID: a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0u1v2w3x4y5z6a7b8c9d0e1f2
Use this ID to retrieve the file later.
```

**Important** : Conservez le File ID retourné, c'est l'identifiant unique pour récupérer le fichier.

## 📥 Récupération de fichiers

### Utilisation de base

```bash
# Récupérer par File ID
php api/decrypt_and_serve.php a1b2c3d4e5f6... --output decrypted.png

# Récupérer par nom de fichier original
php api/decrypt_and_serve.php image.png --output restored.png

# Envoyer vers stdout (pour pipe ou redirection)
php api/decrypt_and_serve.php a1b2c3d4e5f6... > output.png
```

### Commandes d'information

```bash
# Lister tous les fichiers dans la base
php api/decrypt_and_serve.php --list

# Afficher les statistiques de la base
php api/decrypt_and_serve.php --stats

# Afficher les infos détaillées d'un fichier (sans déchiffrer)
php api/decrypt_and_serve.php a1b2c3d4e5f6... --info
```

### Exemples de sortie

**Liste des fichiers :**
```
Files in database:
----------------------------------------------------------------------------------------------------
File ID                                                          Original Filename              Size   Chunks
----------------------------------------------------------------------------------------------------
a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0u1v2w3x4y5z6a7b8c9d0e1f2 image.png                1,234,567 B        2
----------------------------------------------------------------------------------------------------
Total files: 1
```

**Statistiques :**
```
Database Statistics:
--------------------------------------------------
Total files:     1
Total size:      1,234,567 bytes (1.18 MB)
Total chunks:    2
Database path:   /path/to/api/file_mappings.json
Database size:   1,234 bytes
--------------------------------------------------
```

## 🔐 Algorithmes de chiffrement

### AES-256-GCM
- **Algorithme** : AES (Advanced Encryption Standard)
- **Mode** : GCM (Galois/Counter Mode)
- **Taille de clé** : 256 bits
- **Avantages** :
  - Chiffrement authentifié (détecte les modifications)
  - Très performant sur processeurs modernes (instructions AES-NI)
  - Standard de sécurité moderne (approuvé NSA pour TOP SECRET)
  - Parallélisable (rapide sur chunks)

### Structure des données chiffrées
```
[IV (12 bytes)] [TAG (16 bytes)] [Ciphertext (variable)]
```

- **IV (Initialization Vector)** : Vecteur d'initialisation aléatoire unique par chunk
- **TAG** : Tag d'authentification GCM pour vérifier l'intégrité
- **Ciphertext** : Données chiffrées

## 📁 Structure des fichiers

### Sur le serveur PHP (OVH)
```
api/
├── file_mappings.json          # Base de données des correspondances
├── encryption.key              # Clé de chiffrement (ne JAMAIS versionner)
├── ftp.json                    # Config FTP (ne JAMAIS versionner)
├── encrypt_and_upload.php      # Script d'upload
├── decrypt_and_serve.php       # Script de récupération
└── services/
    ├── EncryptionService.php   # Service de chiffrement/déchiffrement
    ├── ChunkManager.php        # Gestion des chunks
    └── MappingDatabase.php     # Gestion de la base de données
```

### Sur le FTP (Box Internet)
```
/encrypted_chunks/
├── 3f7b2c1a9e8d4f6b5c7a8d9e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1
├── 8d9e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0
├── 1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2
└── ...
```

**Remarques importantes :**
- Aucune extension de fichier (impossible de deviner le type)
- Noms en SHA-256 aléatoires (impossible de deviner le contenu)
- Taille variable (padding aléatoire dans le chiffrement GCM)
- Aucun ordre logique (index dans la base de données)

### Format de file_mappings.json

```json
{
  "a1b2c3d4e5f6...": {
    "file_id": "a1b2c3d4e5f6...",
    "original_filename": "image.png",
    "mime_type": "image/png",
    "original_size": 1234567,
    "chunk_count": 2,
    "chunk_size": 1048576,
    "created_at": "2025-10-14T10:30:00+00:00",
    "metadata": {
      "upload_mode": "ftp",
      "storage_location": "/encrypted_chunks",
      "category": "nature"
    },
    "chunks": [
      {
        "index": 0,
        "hash": "3f7b2c1a9e8d4f6b5c7a8d9e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1",
        "size": 1048576,
        "encrypted_size": 1048604
      },
      {
        "index": 1,
        "hash": "8d9e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0",
        "size": 185991,
        "encrypted_size": 186019
      }
    ]
  }
}
```

## ⚙️ Configuration FTP

Le fichier `ftp.json` doit contenir :

```json
{
  "host": "ftp://192.168.1.1",
  "user": "username",
  "password": "password",
  "directory": "/encrypted_chunks"
}
```

**Sécurité** : Ce fichier doit être protégé sur le serveur PHP (HTTPS), il ne transite JAMAIS vers le client.

## 🔒 Bonnes pratiques de sécurité

### 1. Protection de la clé de chiffrement
```bash
# Permissions strictes sur Linux/Mac
chmod 600 encryption.key

# Ajouter au .gitignore
echo "encryption.key" >> .gitignore
echo "ftp.json" >> .gitignore
echo "file_mappings.json" >> .gitignore
```

### 2. Sauvegarde de la base de données
```bash
# Sauvegarde quotidienne automatique
crontab -e
# Ajouter :
0 2 * * * cp /path/to/api/file_mappings.json /path/to/backups/file_mappings_$(date +\%Y\%m\%d).json
```

### 3. Rotation des clés (avancé)
Pour changer la clé de chiffrement :
1. Générer une nouvelle clé
2. Télécharger tous les fichiers avec l'ancienne clé
3. Re-uploader avec la nouvelle clé
4. Supprimer les anciens chunks du FTP

### 4. Permissions serveur PHP
```bash
# Permissions recommandées
chmod 750 api/
chmod 640 api/file_mappings.json
chmod 600 api/encryption.key
chmod 600 api/ftp.json
```

## 🚀 Intégration avec l'API existante

### Modification de FtpService.php

Pour intégrer le système de chiffrement dans votre API REST existante :

```php
// Dans FtpService.php
require_once __DIR__ . '/EncryptionService.php';
require_once __DIR__ . '/ChunkManager.php';
require_once __DIR__ . '/MappingDatabase.php';

class FtpService {
    private $encryptionService;
    private $chunkManager;
    private $mappingDb;

    public function __construct() {
        // Initialisation des services de chiffrement
        $this->encryptionService = new EncryptionService();
        $this->chunkManager = new ChunkManager($this->encryptionService);
        $this->mappingDb = new MappingDatabase();
    }

    public function getFile($filename) {
        // Rechercher le mapping
        $mapping = $this->mappingDb->getMappingByOriginalFilename($filename);

        if (!$mapping) {
            throw new Exception("File not found");
        }

        // Télécharger et déchiffrer les chunks
        $ftpConn = $this->connect();
        $chunks = $this->chunkManager->downloadChunksFromFtp(
            $mapping['chunks'],
            $ftpConn,
            $mapping['metadata']['storage_location']
        );

        // Reconstituer le fichier
        return $this->chunkManager->decryptAndReassemble($chunks);
    }
}
```

## 📊 Performances

### Benchmarks approximatifs
- **Chiffrement** : ~500 MB/s (avec AES-NI)
- **Déchiffrement** : ~500 MB/s (avec AES-NI)
- **Overhead de stockage** : ~28 bytes par chunk (IV + TAG)
- **Taille recommandée de chunk** : 1-4 MB

### Optimisations
- Les chunks sont traités indépendamment (parallélisable)
- Mode GCM très performant sur CPU modernes
- Pas de padding (GCM = stream cipher)

## 🆘 Dépannage

### "ENCRYPTION_KEY environment variable not set"
```bash
# Définir la variable d'environnement
export ENCRYPTION_KEY=$(cat encryption.key)
```

### "Failed to connect to FTP server"
- Vérifier que le serveur FTP est accessible
- Tester avec un client FTP standard (FileZilla)
- Vérifier les identifiants dans `ftp.json`

### "Decryption failed: invalid key or corrupted data"
- La clé de déchiffrement est incorrecte
- Les données ont été corrompues pendant le transfert
- Le chunk a été modifié sur le FTP

### "Chunk file not found"
- Un chunk est manquant sur le FTP
- Le chemin `storage_location` est incorrect
- Vérifier avec `--info` que tous les chunks existent

## 🔄 Migration depuis l'ancien système

Si vous avez déjà des fichiers non chiffrés sur votre FTP :

```bash
#!/bin/bash
# Script de migration (migrate_to_encrypted.sh)

# Lister tous les fichiers actuels
php api/list_ftp_files.php > files_to_migrate.txt

# Pour chaque fichier
while read filename; do
    echo "Migrating: $filename"

    # Télécharger
    php api/download_file.php "$filename" > "/tmp/$filename"

    # Chiffrer et uploader dans le nouveau système
    php api/encrypt_and_upload.php "/tmp/$filename" \
        --remote-dir /encrypted_chunks \
        --metadata "{\"migrated\": true, \"migration_date\": \"$(date -I)\"}"

    # Supprimer le fichier temporaire
    rm "/tmp/$filename"

    echo "Migrated: $filename"
done < files_to_migrate.txt

echo "Migration complete!"
```

## 📚 Ressources supplémentaires

- [AES-GCM sur Wikipedia](https://en.wikipedia.org/wiki/Galois/Counter_Mode)
- [OpenSSL PHP Documentation](https://www.php.net/manual/en/book.openssl.php)
- [NIST AES Standard](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197.pdf)

---

**Note de sécurité finale** : Ce système protège vos données sur un FTP non sécurisé, mais il est TOUJOURS recommandé d'utiliser SFTP/FTPS quand c'est possible. Ce système est une solution de contournement pour les cas où SFTP n'est pas disponible (comme certaines box Internet).