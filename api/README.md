# WallpaperIA API

API REST pour servir les wallpapers chiffrés depuis le FTP.

## Endpoints

### GET /categories
Retourne la liste des catégories avec miniatures.

### GET /wallpapers?category=X&date=DD/MM/YYYY
Retourne les wallpapers d'une catégorie pour une date donnée.

### GET /mini/{filename}
Retourne la miniature d'un wallpaper.

## Services

- **FtpService** : Téléchargement depuis FTP/FTPES
- **EncryptionService** : Déchiffrement AES-256-GCM
- **ChunkManager** : Téléchargement et réassemblage des chunks
- **MappingDatabase** : Lecture de la base de mappings (lecture seule)
- **WallpaperService** : Logique métier wallpapers
- **ThumbnailService** : Génération de miniatures

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

## Upload

Pour uploader des fichiers, utiliser le système séparé dans `/uploader`.

