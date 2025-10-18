# ComfyUI Wallpaper Uploader

Script PHP automatique pour uploader les wallpapers générés par ComfyUI vers le serveur de production.

## Fonctionnalités

- Parcours automatique du répertoire de sortie ComfyUI
- Détection des catégories (sous-répertoires)
- Extraction du nom de base des images (suppression des indices ComfyUI `_00001_`)
- Upload FTP sécurisé (FTPES)
- Enregistrement automatique dans l'API de production
- Logging détaillé des opérations
- Mode dry-run pour tester sans upload

## Installation

1. Copier le fichier de configuration exemple :
```bash
cp config.json.example config.json
```

2. Éditer `config.json` avec vos paramètres :
```json
{
  "comfyui_output_dir": "C:\\Installation\\ComfyUI\\output",
  "ftp": {
    "host": "ftpes://example.com:21",
    "user": "your_ftp_username",
    "password": "your_ftp_password",
    "passive": true,
    "remote_directory": "/wallpapers"
  },
  "api": {
    "url": "https://kazflow.com/wallpapers",
    "username": "uploader",
    "password": "your_api_password"
  }
}
```

## Utilisation

### Upload normal
```bash
php uploader.php
```

### Mode dry-run (test sans upload réel)
```bash
php uploader.php --dry-run
```

### Fichier de configuration personnalisé
```bash
php uploader.php --config=mon_config.json
```

## Fonctionnement

### 1. Structure des fichiers ComfyUI

ComfyUI génère des fichiers avec des noms comme :
```
C:\Installation\ComfyUI\output\
├── CYBERPUNK/FUTURISTIC\
│   ├── neon_city_night_00001_.png
│   ├── neon_city_night_00002_.png
│   └── cyberpunk_street_00001_.png
└── NATURE/LANDSCAPES\
    ├── mountain_sunset_00001_.png
    └── forest_path_00001_.png
```

### 2. Traitement

Le script :
- Détecte la catégorie : `CYBERPUNK/FUTURISTIC`
- Extrait le nom de base : `neon_city_night_00001_.png` → `neon_city_night.png`
- Upload vers FTP : `/wallpapers/neon_city_night.png` (tous les fichiers regroupés dans le même répertoire)
- Enregistre dans l'API avec la catégorie et la date du jour

### 3. Pattern de nom de fichier

Le script reconnaît le pattern ComfyUI : `nom_00XXX_.extension`

Exemples de conversion :
- `neon_city_night_00001_.png` → `neon_city_night.png`
- `cyberpunk_street_00042_.png` → `cyberpunk_street.png`
- `simple_name.png` → `simple_name.png` (pas de conversion)

### 4. Évitement des doublons

Le script vérifie si un fichier a déjà été uploadé dans la session en cours pour éviter les doublons (si plusieurs variantes `_00001_`, `_00002_` existent).

**Important** : Tous les fichiers sont uploadés dans le même répertoire FTP (pas de sous-répertoires par catégorie). La catégorie est uniquement conservée pour l'enregistrement dans l'API.

## Logs

Les opérations sont enregistrées dans `upload.log` :
```
[2025-01-15 14:30:00] [INFO] Starting image processing from: C:\Installation\ComfyUI\output
[2025-01-15 14:30:01] [INFO] Connected to FTP server: example.com
[2025-01-15 14:30:02] [INFO] Processing category: CYBERPUNK/FUTURISTIC
[2025-01-15 14:30:03] [INFO] Processing: neon_city_night_00001_.png -> neon_city_night.png (category: CYBERPUNK/FUTURISTIC)
[2025-01-15 14:30:04] [INFO] Uploaded to FTP: /wallpapers/neon_city_night.png
[2025-01-15 14:30:05] [INFO] Registered in API: CYBERPUNK/FUTURISTIC / neon_city_night.png
[2025-01-15 14:30:06] [INFO] Processing completed: 15 files processed, 0 errors
```

## Codes de sortie

- `0` : Succès (aucune erreur)
- `1` : Erreur(s) rencontrée(s)

## Automatisation

### Windows Task Scheduler

Créer une tâche planifiée qui exécute :
```bash
php C:\Code\WallpaperIA\ComfyUIBot\uploader.php
```

### Cron (Linux/macOS)

Ajouter dans crontab :
```cron
# Upload toutes les heures
0 * * * * cd /path/to/ComfyUIBot && php uploader.php
```

## Sécurité

- Le fichier `config.json` contient des credentials sensibles
- Ajouter `config.json` au `.gitignore`
- Ne jamais commiter les vrais credentials
- Utiliser FTPES (FTP over TLS) pour la sécurité

## Dépendances

- PHP 7.4+
- Extension PHP `ftp`
- Extension PHP `curl`
- Extension PHP `json`

## Troubleshooting

### Erreur de connexion FTP

Vérifier :
- Host, port, username, password dans `config.json`
- Mode passif activé si derrière un firewall
- Extension PHP FTP installée : `php -m | grep ftp`

### Erreur d'authentification API

Vérifier :
- URL de l'API correcte
- Username et password valides
- Le serveur accepte HTTP Basic Auth

### Fichiers non traités

Vérifier :
- Extensions valides : `.png`, `.jpg`, `.jpeg`, `.webp`
- Pattern de nom reconnu par `extractBaseName()`
- Permissions de lecture sur les fichiers
