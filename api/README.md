# WallpaperIA API

API REST pour servir les wallpapers chiffrés depuis le FTP.

## Endpoints

### GET /categories
Retourne la liste des catégories avec miniatures.

### GET /wallpapers?category=X&date=DD/MM/YYYY
Retourne les wallpapers d'une catégorie pour une date donnée.

### GET /mini/{filename}
Retourne la miniature d'un wallpaper.

### POST /wallpapers (Authentification requise)
Ajoute une nouvelle entrée dans le fichier wallpapers.csv.

**Authentification** : HTTP Basic Auth (username:password)

**Body (JSON)** :
```json
{
  "category": "CYBERPUNK/FUTURISTIC",
  "filename": "neon_city_night.png",
  "date": "29/09/2025"
}
```

**Réponse succès** :
```json
{
  "success": true,
  "message": "Wallpaper entry added successfully",
  "entry": {
    "category": "CYBERPUNK/FUTURISTIC",
    "filename": "neon_city_night.png",
    "date": "29/09/2025",
    "id": 1
  },
  "authenticated_as": "username"
}
```

## Services

- **FtpService** : Téléchargement depuis FTP/FTPES
- **EncryptionService** : Déchiffrement AES-256-GCM
- **ChunkManager** : Téléchargement et réassemblage des chunks
- **MappingDatabase** : Lecture de la base de mappings (lecture seule)
- **WallpaperService** : Logique métier wallpapers
- **ThumbnailService** : Génération de miniatures
- **AuthService** : Authentification HTTP Basic Auth
- **CsvManager** : Gestion du fichier wallpapers.csv (ajout d'entrées)

## Configuration

### .htaccess
Protection des fichiers sensibles, configuration PHP.

### ftp.json (exemple)
```json
{
  "host": "ftpes://freebox@home.kazuya.fr:51114/",
  "user": "freebox",
  "password": "votre_mot_de_passe",
  "passive": true,
  "directory": "/wallpapers"
}
```

### encryption.key
Clé de déchiffrement (256 bits hex).

### .credentials
Fichier d'authentification pour POST /wallpapers.

Format : `username:sha256_hash_of_password`

**Exemple** :
```
# User admin with password "secret123"
admin:2bb80d537b1da3e38bd30361aa855686bde0eacd7162fef6a25fe97bf527a25b

# User uploader with password "mypassword"
uploader:89e01536ac207279409d4de1e5253e01f4a1769e696db0d6062ca9b8f56767c8
```

**Générer un hash SHA256** :
```bash
echo -n "your_password" | sha256sum
```

## Upload

Pour uploader des fichiers, utiliser le système séparé dans `/uploader`.

