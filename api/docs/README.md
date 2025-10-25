# WallpaperIA API - Documentation Complète

API REST PHP pour servir des wallpapers AI depuis un serveur FTP sécurisé (FTPES).

## 📋 Table des matières

- [Architecture](#architecture)
- [Endpoints](#endpoints)
- [Services](#services)
- [Sécurité](#sécurité)
- [Configuration](#configuration)
- [Rate Limiting](#rate-limiting)
- [Utilisation](#utilisation)
- [Développement](#développement)

---

## 🏗️ Architecture

### Structure du projet

```
api/
├── index.php                 # Point d'entrée principal
├── config/
│   └── Config.php           # Configuration centralisée
├── core/
│   └── Router.php           # Routage et parsing des requêtes
├── controllers/
│   └── ApiController.php    # Contrôleur principal
├── services/
│   ├── WallpaperService.php # Logique métier wallpapers
│   ├── FtpService.php       # Connexion FTP/FTPES
│   ├── ThumbnailService.php # Génération de miniatures
│   ├── AuthService.php      # Authentification HTTP Basic
│   ├── CsvManager.php       # Gestion du fichier CSV
│   └── RateLimitService.php # Protection rate limiting
├── wallpapers.csv           # Base de données des wallpapers
├── ftp.json                 # Configuration FTP (ignoré par git)
├── .credentials             # Utilisateurs autorisés (ignoré par git)
└── docs/
    └── README.md            # Cette documentation
```

### Flux de requête

```
Requête HTTP
    ↓
index.php (gestion erreurs, CORS)
    ↓
Router (parsing URI, rate limiting niveau 1)
    ↓
ApiController (dispatch selon type)
    ↓
Service approprié (FTP, Wallpaper, Thumbnail, Auth)
    ↓
Réponse JSON ou fichier binaire
```

---

## 🔌 Endpoints

### GET /categories

Retourne la liste de toutes les catégories avec leurs miniatures par défaut.

**Réponse :**
```json
{
  "success": true,
  "categories": [
    {
      "id": "cf",
      "name": "CYBERPUNK FUTURISTIC",
      "thumbnail_url": "https://api.domain.com/mini/wallpaper_123.png",
      "timestamp": 1735171200
    }
  ],
  "timestamp": 1735171200
}
```

**Rate limit :** 100 requêtes/minute par IP

**Cache :** Inclut un timestamp pour détecter les changements (basé sur `filemtime` du CSV)

---

### GET /wallpapers?category={id}&date={DD/MM/YYYY}

Retourne les wallpapers d'une catégorie pour une date donnée.

**Paramètres obligatoires :**
- `category` : ID de catégorie (ex: `cf` pour Cyberpunk Futuristic)
- `date` : Format DD/MM/YYYY (ex: `29/09/2025`)

**Réponse succès :**
```json
{
  "success": true,
  "category": "CYBERPUNK FUTURISTIC",
  "date": "29/09/2025",
  "wallpapers": [
    {
      "id": 1,
      "filename": "neon_city_night.png",
      "download_url": "https://api.domain.com/get/neon_city_night.png",
      "thumbnail_url": "https://api.domain.com/mini/neon_city_night.png"
    }
  ]
}
```

**Fallback intelligent :** Si aucun wallpaper n'existe pour la date, l'API remonte automatiquement jusqu'à 7 jours en arrière.

**Rate limit :** 200 requêtes/minute par IP

---

### GET /get/{filename}

Télécharge un fichier depuis le serveur FTP.

**Exemple :** `GET /get/neon_city_night.png`

**Réponse :** Fichier binaire (image)

**Headers :**
- `Content-Type: image/png` (détecté automatiquement)
- `Content-Disposition: inline; filename="neon_city_night.png"`
- `Cache-Control: public, max-age=3600` (1 heure)

**Sécurité :**
- Validation stricte du nom de fichier (regex `^[a-zA-Z0-9._-]+$`)
- Protection contre path traversal (`../`, `./`)
- Utilisation de `basename()` pour bloquer les tentatives d'injection

**Rate limit :** 300 requêtes/minute par IP

---

### GET /mini/{filename}

Retourne une miniature (204x115px) au format JPEG.

**Exemple :** `GET /mini/neon_city_night.png`

**Comportement :**
1. Si la miniature existe en cache → retour immédiat
2. Sinon → génération à la volée + mise en cache

**Headers :**
- `Content-Type: image/jpeg`
- `Cache-Control: public, max-age=86400` (24 heures)
- `X-Thumbnail-Cached: true/false` (indique si depuis cache)

**Configuration miniatures :**
- Dimensions : 204x115px (ratio 16:9)
- Format : JPEG (optimisé pour le web)
- Qualité : 95 (haute qualité)
- Stockage : `api/miniatures/`

**Rate limit :** 150 requêtes/minute par IP (génération coûteuse)

---

### POST /wallpapers

Ajoute une nouvelle entrée dans le fichier `wallpapers.csv`.

**Authentification requise :** HTTP Basic Auth

**Body JSON :**
```json
{
  "category": "CYBERPUNK FUTURISTIC",
  "filename": "neon_city_night.png",
  "date": "29/09/2025"
}
```

**Exemple curl :**
```bash
curl -X POST https://api.domain.com/wallpapers \
  -u admin:secret123 \
  -H "Content-Type: application/json" \
  -d '{
    "category": "CYBERPUNK FUTURISTIC",
    "filename": "neon_city_night.png",
    "date": "29/09/2025"
  }'
```

**Réponse succès :**
```json
{
  "success": true,
  "message": "Wallpaper entry added successfully",
  "entry": {
    "category": "CYBERPUNK FUTURISTIC",
    "filename": "neon_city_night.png",
    "date": "29/09/2025",
    "id": 1
  },
  "authenticated_as": "admin"
}
```

**Validations :**
- ✅ Format de date : DD/MM/YYYY
- ✅ Détection de doublons (category/filename/date)
- ✅ Protection CSV Injection (utilisation de `fputcsv()`)
- ✅ ID auto-incrémenté par category/date

**Rate limiting :**
- **Par IP :** 20 ajouts/heure (premier niveau)
- **Par utilisateur :** 50 ajouts/heure (second niveau après auth)

**Erreurs possibles :**
- `401` : Authentification invalide
- `400` : Paramètres manquants ou format invalide
- `409` : Entrée déjà existante
- `429` : Rate limit dépassé

---

## 🛠️ Services

### WallpaperService

Gestion de la logique métier des wallpapers.

**Responsabilités :**
- Lecture et parsing du fichier CSV
- Génération des catégories avec miniatures
- Filtrage par catégorie et date
- Fallback intelligent (7 jours)
- Mapping des noms de catégories vers IDs courts

**Méthodes principales :**
```php
getCategories()                              // Liste toutes les catégories
getWallpapersByCategoryAndDate($cat, $date) // Filtre par catégorie/date
```

---

### FtpService

Connexion sécurisée au serveur FTP/FTPES.

**Configuration (ftp.json) :**
```json
{
  "host": "ftpes://freebox@home.domain.fr:51114/",
  "user": "freebox",
  "password": "your_password",
  "passive": true,
  "directory": "/wallpapers"
}
```

**Protocole supporté :** FTPES (FTP over Explicit TLS)

**Méthodes principales :**
```php
getFileFromFtp($filename)  // Télécharge un fichier
```

**Sécurité :**
- Validation stricte des noms de fichiers
- Protection contre path traversal
- Connexion TLS chiffrée

---

### ThumbnailService

Génération et mise en cache de miniatures.

**Workflow :**
1. Vérifier si miniature existe (`api/miniatures/{filename}.jpg`)
2. Si non → télécharger depuis FTP → redimensionner → sauvegarder
3. Retourner la miniature (depuis cache ou fraîchement générée)

**Configuration :**
- Dimensions : 204x115px
- Format : JPEG (qualité 95)
- Cache : Illimité (pas d'expiration automatique)

**Optimisations :**
- Utilisation de GD Library pour le redimensionnement
- Génération à la volée (pas de pré-génération)
- Header `X-Thumbnail-Cached` pour monitoring

---

### AuthService

Authentification HTTP Basic Auth avec hachage SHA256.

**Fichier .credentials :**
```
# Format: username:sha256_hash_of_password
admin:2bb80d537b1da3e38bd30361aa855686bde0eacd7162fef6a25fe97bf527a25b
uploader:89e01536ac207279409d4de1e5253e01f4a1769e696db0d6062ca9b8f56767c8
```

**Générer un hash :**
```bash
echo -n "your_password" | sha256sum
```

**Sécurité :**
- Hachage SHA256 (pas de plaintext)
- Protection .htaccess sur .credentials
- Vérification en temps constant (prévention timing attacks)

---

### CsvManager

Gestion du fichier `wallpapers.csv`.

**Format CSV :**
```csv
category,filename,date,id
CYBERPUNK FUTURISTIC,neon_city_night.png,29/09/2025,1
OCEAN & MARINE,coral_reef_underwater.png,29/09/2025,1
```

**Fonctionnalités :**
- ✅ Ajout d'entrées avec `fputcsv()` (anti-CSV injection)
- ✅ Détection de doublons
- ✅ Auto-incrémentation des IDs par category/date
- ✅ Validation du format de date

**Protection CSV Injection :**
```php
// Détection de formules dangereuses
if (preg_match('/^[=+\-@]/', $value)) {
    throw new Exception("CSV injection detected");
}
```

---

### RateLimitService

Protection contre les abus avec fenêtre glissante (sliding window).

**Configuration des limites :**

| Endpoint | Limite | Fenêtre | Identifiant |
|----------|--------|---------|-------------|
| GET /categories | 100 req | 1 min | IP |
| GET /wallpapers | 200 req | 1 min | IP |
| GET /get/{file} | 300 req | 1 min | IP |
| GET /mini/{file} | 150 req | 1 min | IP |
| POST /wallpapers | 20 req | 1 heure | IP |
| POST /wallpapers (user) | 50 req | 1 heure | Username |

**Stockage :** Fichiers JSON dans `api/rate_limit/` (hachage SHA256)

**Headers retournés :**
```
X-RateLimit-Limit: 100
X-RateLimit-Remaining: 95
Retry-After: 42  (si limite dépassée)
```

**Réponse 429 (Too Many Requests) :**
```json
{
  "success": false,
  "error": "Too many requests. Please slow down.",
  "retry_after": 42,
  "limit": 100,
  "window": 60
}
```

**Nettoyage automatique :** Fichiers expirés supprimés (1% de chance à chaque requête)

---

## 🔒 Sécurité

### Protection Path Traversal

**Validation multi-niveaux :**
1. **Router** : Validation avec `basename()` et regex
2. **FtpService** : Validation supplémentaire
3. **ThumbnailService** : Validation avant génération

**Regex stricte :**
```php
preg_match('/^[a-zA-Z0-9._-]+$/', $filename)
```

**Blocage :**
- `../` → Blocked
- `./` → Blocked
- `\` → Blocked
- Tout caractère spécial → Blocked

---

### Protection CSV Injection

**Méthode sécurisée :**
```php
// ✅ Bon (utilise fputcsv)
fputcsv($handle, [$category, $filename, $date, $id]);

// ❌ Mauvais (vulnérable)
fwrite($handle, "$category,$filename,$date,$id\n");
```

**Validation préventive :**
```php
if (preg_match('/^[=+\-@]/', $category)) {
    return ['success' => false, 'error' => 'CSV injection detected'];
}
```

---

### Protection CORS

**Whitelist de domaines :**
```php
$allowed_origins = [
    'https://kazflow.com',
    'http://localhost',
    'null'  // Fichiers HTML locaux
];
```

**Comportement :**
- Applications desktop (Qt) → Pas de header Origin → **Toujours autorisées**
- Applications web → Vérification de la whitelist

---

### Exposition d'informations

**Mode production :**
```php
$is_production = true;
error_reporting(E_ALL);
ini_set('display_errors', 0);          // Masquer les erreurs
ini_set('log_errors', 1);               // Logger dans fichier
ini_set('error_log', 'logs/php_errors.log');
```

**Mode développement :**
```php
$is_production = false;
ini_set('display_errors', 1);          // Afficher les erreurs
```

---

## ⚙️ Configuration

### Fichier ftp.json

```json
{
  "host": "ftpes://server.domain.fr:21/",
  "user": "username",
  "password": "password",
  "passive": true,
  "directory": "/wallpapers"
}
```

**Emplacement :** `api/ftp.json` (ignoré par `.gitignore`)

---

### Fichier .credentials

```
# username:sha256_password
admin:2bb80d537b1da3e38bd30361aa855686bde0eacd7162fef6a25fe97bf527a25b
```

**Génération :**
```bash
echo -n "mysecretpassword" | sha256sum
```

**Protection :** Bloqué par `.htaccess`

---

### Fichier .htaccess

```apache
# Bloquer l'accès aux fichiers sensibles
<FilesMatch "^(\.credentials|ftp\.json|wallpapers\.csv)$">
    Require all denied
</FilesMatch>

# Configuration PHP
php_flag log_errors on
php_value error_log logs/php_errors.log

# Résoudre conflit MultiViews (endpoint /wallpapers vs fichier wallpapers.csv)
Options -MultiViews

# Redirection vers index.php
RewriteEngine On
RewriteCond %{REQUEST_FILENAME} !-f
RewriteCond %{REQUEST_FILENAME} !-d
RewriteRule ^(.*)$ index.php [QSA,L]
```

---

## 📊 Rate Limiting

### Architecture

**Système de fenêtre glissante** (sliding window) :
- Stockage dans fichiers JSON (`api/rate_limit/`)
- Hachage SHA256 des identifiants (IP ou username)
- Nettoyage automatique des fichiers expirés (24h)

### Détection d'IP

**Support des proxies :**
```php
if (isset($_SERVER['HTTP_CF_CONNECTING_IP'])) {
    return $_SERVER['HTTP_CF_CONNECTING_IP'];  // Cloudflare
}
if (isset($_SERVER['HTTP_X_FORWARDED_FOR'])) {
    return explode(',', $_SERVER['HTTP_X_FORWARDED_FOR'])[0];  // Nginx
}
return $_SERVER['REMOTE_ADDR'];  // Direct
```

### Double protection POST /wallpapers

1. **Niveau IP** : 20 requêtes/heure (avant auth)
2. **Niveau Username** : 50 requêtes/heure (après auth)

**Pourquoi ?**
- Limite par IP empêche les attaques brute-force sur l'auth
- Limite par username empêche l'abus avec un compte compromis

---

## 🚀 Utilisation

### Exemples cURL

**Récupérer les catégories :**
```bash
curl https://api.domain.com/categories
```

**Récupérer les wallpapers :**
```bash
curl "https://api.domain.com/wallpapers?category=cf&date=29/09/2025"
```

**Télécharger un wallpaper :**
```bash
curl https://api.domain.com/get/neon_city_night.png -o wallpaper.png
```

**Ajouter une entrée :**
```bash
curl -X POST https://api.domain.com/wallpapers \
  -u admin:secret123 \
  -H "Content-Type: application/json" \
  -d '{
    "category": "CYBERPUNK FUTURISTIC",
    "filename": "neon_city_night.png",
    "date": "29/09/2025"
  }'
```

---

## 🔧 Développement

### Installation

1. **Copier les fichiers dans le répertoire web**
   ```bash
   cp -r api/ /var/www/html/api/
   ```

2. **Créer ftp.json**
   ```bash
   nano api/ftp.json
   # Ajouter la configuration FTP
   ```

3. **Créer .credentials**
   ```bash
   echo "admin:$(echo -n 'yourpassword' | sha256sum | cut -d' ' -f1)" > api/.credentials
   ```

4. **Définir les permissions**
   ```bash
   chmod 755 api/
   chmod 644 api/*.php
   chmod 600 api/ftp.json api/.credentials
   mkdir api/miniatures api/rate_limit api/logs
   chmod 755 api/miniatures api/rate_limit api/logs
   ```

5. **Tester l'API**
   ```bash
   curl https://yourdomain.com/api/categories
   ```

---

### Debug

**Activer le mode développement :**
```php
// api/index.php
$is_production = false;  // Afficher les erreurs
```

**Vérifier les logs :**
```bash
tail -f api/logs/php_errors.log
```

**Tester l'authentification :**
```bash
php api/debug_auth.php
```

---

## 📝 Notes

### Timestamp de synchronisation

L'endpoint `/categories` retourne un `timestamp` basé sur `filemtime(wallpapers.csv)`.

**Utilisation côté client :**
```cpp
// Qt - WallpaperAI
if (api_timestamp > local_cache_timestamp) {
    // Recharger les catégories
    // Vider le cache des miniatures
}
```

### Fallback intelligent

Si aucun wallpaper n'existe pour une date, l'API remonte automatiquement jusqu'à 7 jours en arrière.

**Exemple :**
- Date demandée : 05/10/2025
- Aucun wallpaper trouvé
- Essai : 04/10, 03/10, 02/10... jusqu'à 28/09
- Si toujours vide → retour liste vide

---

## 📄 Licence

API développée pour le projet WallpaperAI.

---

## 🤝 Contributeurs

Développé avec [Claude Code](https://claude.com/claude-code)
