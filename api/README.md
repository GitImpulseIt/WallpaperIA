# 🖼️ Wallpaper API Documentation

API REST moderne pour la gestion et distribution de fonds d'écran avec génération automatique de vignettes.

## 📁 Architecture

```
api/
├── README.md                      # Cette documentation
├── index.php                      # Point d'entrée principal
├── wallpapers.csv                 # Base de données des wallpapers
├── ftp.json                       # Configuration FTP (non versionné)
├── miniatures/                    # Cache des vignettes générées
├── config/
│   └── Config.php                 # Configuration centralisée
├── core/
│   └── Router.php                 # Système de routing
├── services/
│   ├── WallpaperService.php       # Gestion données wallpapers
│   ├── FtpService.php             # Communication serveur FTP
│   └── ThumbnailService.php       # Génération vignettes
└── controllers/
    └── ApiController.php          # Contrôleur principal
```

## 🚀 Démarrage rapide

### Configuration FTP

Créez un fichier `ftp.json` avec vos identifiants :

```json
{
    "host": "ftp://votre-serveur.com",
    "user": "utilisateur",
    "password": "motdepasse",
    "directory": "/chemin/vers/images"
}
```

### Démarrage serveur local

```bash
cd api/
php -S localhost:8000
```

### Test de base

```bash
curl http://localhost:8000/
```

## 📊 Endpoints API

### 🏠 Informations générales

**GET** `/`
- **Description** : Informations sur l'API et liste des endpoints
- **Réponse** : JSON avec version et endpoints disponibles

```bash
curl http://localhost:8000/
```

### 📂 Catégories

**GET** `/categories`
- **Description** : Liste toutes les catégories disponibles
- **Réponse** : Tableau des catégories avec ID court et nom complet

```bash
curl http://localhost:8000/categories
```

**Réponse :**
```json
{
  "success": true,
  "data": [
    {
      "id": "as",
      "name": "AUTUMN SEASONS"
    },
    {
      "id": "om",
      "name": "OCEAN & MARINE"
    }
  ],
  "count": 15
}
```

### 🖼️ Wallpapers

**GET** `/wallpapers?category={id}&date={DD/MM/YYYY}`
- **Description** : Wallpapers d'une catégorie et d'une date spécifiques
- **Paramètres obligatoires** :
  - `category` : ID court de la catégorie (utiliser `/categories` pour obtenir la liste)
  - `date` : Date au format DD/MM/YYYY (ex: 29/09/2025)

```bash
# Les deux paramètres sont obligatoires (noter l'encodage URL des '/')
curl "http://localhost:8000/wallpapers?category=om&date=29%2F09%2F2025"
```

**Réponse :**
```json
{
  "success": true,
  "category": "OCEAN & MARINE",
  "category_id": "om",
  "date": "29/09/2025",
  "data": [
    {
      "category": "OCEAN & MARINE",
      "filename": "ocean_sunset.png",
      "date": "29/09/2025",
      "id": "1"
    },
    {
      "category": "OCEAN & MARINE",
      "filename": "ocean_waves.png",
      "date": "29/09/2025",
      "id": "2"
    }
  ],
  "count": 2
}
```

### 📥 Téléchargement fichiers

**GET** `/get/{filename}`
- **Description** : Télécharge un fichier depuis le serveur FTP
- **Paramètres** :
  - `filename` : Nom exact du fichier à télécharger
- **Réponse** : Contenu binaire du fichier avec headers appropriés

```bash
curl http://localhost:8000/get/ocean_sunset.png --output image.png
```

### 🔍 Vignettes

**GET** `/mini/{filename}`
- **Description** : Génère/récupère une vignette (204x115px)
- **Paramètres** :
  - `filename` : Nom du fichier original
- **Cache** : Les vignettes sont mises en cache localement
- **Formats** : Conserve le format original (PNG, JPG, WEBP, GIF)

```bash
curl http://localhost:8000/mini/ocean_sunset.png --output thumbnail.png
```

**Headers de réponse :**
- `X-Thumbnail-Cached` : `true` si récupéré du cache, `false` si généré

## 🗃️ Structure des données

### Format CSV

Le fichier `wallpapers.csv` contient :

| Colonne  | Description | Exemple |
|----------|-------------|---------|
| category | Nom de la catégorie | OCEAN & MARINE |
| filename | Nom du fichier | ocean_sunset.png |
| date     | Date d'ajout (JJ/MM/AAAA) | 29/09/2025 |
| id       | ID séquentiel par catégorie/date | 4 |

### Identification unique

Chaque wallpaper peut être identifié par :
- **ID Catégorie** + **Date** + **ID**
- Exemple : "Wallpaper n°4 du 29/09/2025 catégorie 'om' (OCEAN & MARINE)"

### Catégories supportées

| ID | Nom complet |
|----|-------------|
| as | AUTUMN SEASONS |
| cf | CYBERPUNK/FUTURISTIC |
| dl | DESERT LANDSCAPES |
| fw | FANTASY WORLDS |
| gp | GEOMETRIC PATTERNS |
| ma | MINIMALIST ABSTRACT |
| mp | MOUNTAINS & PEAKS |
| mg | MYSTERIOUS GOTHIC |
| nl | NATURAL LANDSCAPES |
| om | OCEAN & MARINE |
| sc | SPACE & COSMOS |
| tp | TROPICAL PARADISE |
| ua | URBAN ARCHITECTURE |
| vr | VINTAGE RETRO |
| ww | WINTER WONDERLAND |

## 🔧 Configuration

### Variables Config.php

- **Dimensions vignettes** : 204x115px par défaut
- **Qualité JPEG** : 95%
- **Compression PNG** : Niveau 6
- **Cache vignettes** : 24h (navigateur)
- **Cache fichiers** : 1h (navigateur)

### Gestion mémoire

- **Limite base** : 512MB pour traitement images
- **Limite étendue** : 1024MB pour images très grandes
- **Optimisations** : Redimensionnement progressif, libération mémoire

## 🔒 Sécurité

### Validation fichiers

- **Noms autorisés** : `[a-zA-Z0-9._-]+`
- **Extensions supportées** : PNG, JPG, JPEG, GIF, WEBP, BMP, SVG
- **Pas d'exécution** : Aucun code exécutable accepté

### Headers CORS

```php
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, OPTIONS
Access-Control-Allow-Headers: Content-Type
```

## 🚨 Gestion d'erreurs

### Codes de réponse

- **200** : Succès
- **404** : Fichier/endpoint non trouvé
- **500** : Erreur serveur

### Format d'erreur

```json
{
  "success": false,
  "error": "Description de l'erreur"
}
```

### Erreurs courantes

- **FTP indisponible** : Vérifiez `ftp.json`
- **Fichier inexistant** : Nom incorrect ou absent du serveur
- **Extension GD manquante** : Installer `php-gd`
- **Mémoire insuffisante** : Augmenter `memory_limit` PHP
- **Invalid category ID** : Utiliser seulement les IDs courts (ex: 'om', 'as'), pas les noms complets
- **Missing required parameter** : `category` et `date` sont obligatoires pour `/wallpapers`
- **Invalid date format** : Utiliser le format DD/MM/YYYY (ex: 29/09/2025)

## 🔄 Cache et performance

### Vignettes

- **Stockage** : `miniatures/` (local)
- **Naming** : Nom original avec extension conservée
- **Régénération** : Automatique si fichier absent

### Optimisations

- **Redimensionnement progressif** pour images > 50% réduction
- **Libération mémoire** immédiate après traitement
- **Garbage collection** forcé si disponible

## 📈 Monitoring

### Headers debug

- `X-Thumbnail-Cached` : État du cache vignettes
- `Content-Type` : Type MIME détecté automatiquement
- `Cache-Control` : Directives de mise en cache

### Logs

Utilisez les logs du serveur web pour surveiller :
- Requêtes 404 (fichiers manquants)
- Temps de réponse vignettes
- Erreurs FTP

## 🛠️ Développement

### Ajout d'endpoints

1. Modifier `core/Router.php` pour le routing
2. Ajouter méthode dans `controllers/ApiController.php`
3. Créer service si logique métier complexe

### Extension services

- **WallpaperService** : Nouvelles requêtes CSV
- **FtpService** : Autres protocoles (SFTP, S3...)
- **ThumbnailService** : Nouveaux formats, filtres

### Tests

```bash
# Test complet des endpoints
curl -s http://localhost:8000/ | jq .
curl -s http://localhost:8000/categories | jq .
curl -s "http://localhost:8000/wallpapers?category=om&date=29%2F09%2F2025" | jq .

# Test des erreurs
curl -s "http://localhost:8000/wallpapers" | jq .  # Paramètres manquants
curl -s "http://localhost:8000/wallpapers?category=om" | jq .  # Date manquante
curl -s "http://localhost:8000/wallpapers?category=invalid&date=29%2F09%2F2025" | jq .  # ID invalide
curl -s "http://localhost:8000/wallpapers?category=om&date=2025-09-29" | jq .  # Format date invalide
```

---

## 📞 Support

Pour les questions techniques ou bugs :
- Vérifiez les logs PHP
- Testez la connectivité FTP
- Validez la structure CSV
- Contrôlez les permissions dossier `miniatures/`