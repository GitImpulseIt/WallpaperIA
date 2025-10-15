# Uploader - Système de chiffrement et upload FTP

Interface locale pour uploader des fichiers chiffrés vers le FTP.

## Démarrage

```bash
start_server.bat
```

Puis ouvrir : http://localhost:8080/uploader.html

## Configuration

Créer un fichier `ftp.json` :
```json
{
  "host": "ftpes://freebox@home.kazuya.fr:51114/",
  "user": "freebox",
  "password": "votre_mot_de_passe",
  "passive": true
}
```

## Documentation

- [ENCRYPTION_GUIDE.md](ENCRYPTION_GUIDE.md) - Guide complet du système de chiffrement
- [FILE_MAPPINGS_FORMAT.md](FILE_MAPPINGS_FORMAT.md) - Format de la base de données

## Scripts CLI

```bash
# Upload un fichier
php encrypt_and_upload.php image.png

# Récupérer un fichier
php decrypt_and_serve.php <file_id> --output image.png

# Lister les fichiers
php decrypt_and_serve.php --list
```

