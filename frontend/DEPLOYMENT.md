# Guide de déploiement - WallpaperAI Landing Page

Ce guide explique comment déployer la landing page WallpaperAI sur différentes plateformes.

## 📋 Prérequis

- Node.js 18+ installé localement
- Accès au serveur web de production
- Support PHP 7.4+ sur le serveur (pour détection de langue)

## 🚀 Build de production

### 1. Installation des dépendances

```bash
cd frontend
npm install
```

### 2. Build du site

```bash
npm run build
```

Cette commande génère le site dans `dist/` avec :
- 8 pages HTML statiques (une par langue + index)
- Assets optimisés (CSS, JS, images)
- `index.php` pour la détection de langue
- `.htaccess` pour la configuration Apache

### 3. Vérification du build

```bash
# Structure attendue dans dist/
dist/
├── .htaccess          # Configuration Apache
├── index.php          # Détection de langue
├── index.html         # Redirection Astro (backup)
├── en/index.html      # Page anglaise
├── fr/index.html      # Page française
├── es/index.html      # Page espagnole
├── pt/index.html      # Page portugaise
├── it/index.html      # Page italienne
├── de/index.html      # Page allemande
├── ru/index.html      # Page russe
├── _astro/            # Assets générés
├── img/               # Images et screenshots
└── favicon.ico        # Icône du site
```

## 🌐 Déploiement sur Apache (recommandé)

### Configuration requise

- Apache 2.4+
- PHP 7.4+ avec `mod_rewrite` activé
- Support des fichiers `.htaccess`

### Étapes de déploiement

1. **Transférer les fichiers**

```bash
# Via FTP/SFTP, copier tout le contenu de dist/ vers le répertoire web
# Par exemple: /var/www/html/ ou /home/user/public_html/
```

2. **Vérifier les permissions**

```bash
# Sur le serveur
cd /var/www/html
chmod 644 index.php .htaccess
chmod 755 en fr es pt it de ru _astro img
```

3. **Vérifier la configuration Apache**

Assurez-vous que `mod_rewrite` est activé :

```bash
sudo a2enmod rewrite
sudo systemctl restart apache2
```

Vérifiez que `.htaccess` est autorisé dans la configuration Apache :

```apache
<Directory /var/www/html>
    AllowOverride All
    Require all granted
</Directory>
```

### Test du déploiement

1. Accédez à `https://votredomaine.com/`
   - Doit détecter votre langue et rediriger vers `/en/`, `/fr/`, etc.

2. Testez le changement de langue
   - Le cookie `preferred_language` doit être défini
   - Les visites suivantes doivent mémoriser votre choix

3. Testez les URLs directes
   - `https://votredomaine.com/en/` → Page anglaise
   - `https://votredomaine.com/fr/` → Page française
   - etc.

## 🐳 Déploiement avec Nginx

Si vous utilisez Nginx au lieu d'Apache, remplacez `.htaccess` par cette configuration :

```nginx
server {
    listen 80;
    server_name votredomaine.com;
    root /var/www/html;
    index index.php index.html;

    # Gzip compression
    gzip on;
    gzip_types text/css application/javascript image/svg+xml;

    # Cache pour assets statiques
    location ~* \.(jpg|jpeg|png|gif|ico|css|js|woff2|woff)$ {
        expires 1y;
        add_header Cache-Control "public, immutable";
    }

    # Racine → index.php pour détection langue
    location = / {
        try_files /index.php =404;
        fastcgi_pass unix:/var/run/php/php8.1-fpm.sock;
        fastcgi_index index.php;
        include fastcgi_params;
        fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
    }

    # URLs de langues → fichiers HTML statiques
    location ~ ^/(en|fr|es|pt|it|de|ru)/?$ {
        try_files /$1/index.html =404;
    }

    location ~ ^/(en|fr|es|pt|it|de|ru)/(.*)$ {
        try_files /$1/$2 =404;
    }

    # Traiter les fichiers PHP
    location ~ \.php$ {
        try_files $uri =404;
        fastcgi_pass unix:/var/run/php/php8.1-fpm.sock;
        fastcgi_index index.php;
        include fastcgi_params;
        fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
    }

    # Headers de sécurité
    add_header X-Frame-Options "SAMEORIGIN" always;
    add_header X-XSS-Protection "1; mode=block" always;
    add_header X-Content-Type-Options "nosniff" always;
    add_header Referrer-Policy "strict-origin-when-cross-origin" always;
}
```

## ☁️ Déploiement sur Netlify/Vercel (sans PHP)

Si vous ne pouvez pas utiliser PHP, la détection automatique de langue ne fonctionnera pas.

### Option 1 : Redirection statique

Modifiez `src/pages/index.astro` pour toujours rediriger vers `/en/` :

```astro
---
return Astro.redirect('/en/');
---
```

### Option 2 : Détection JavaScript côté client

Créez un fichier `public/redirect.html` :

```html
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Redirecting...</title>
  <script>
    const langs = ['en', 'fr', 'es', 'pt', 'it', 'de', 'ru'];
    const userLang = navigator.language.substring(0, 2);
    const targetLang = langs.includes(userLang) ? userLang : 'en';

    // Vérifier cookie
    const cookie = document.cookie.split('; ')
      .find(row => row.startsWith('preferred_language='));

    if (cookie) {
      const savedLang = cookie.split('=')[1];
      if (langs.includes(savedLang)) {
        window.location.href = `/${savedLang}/`;
      }
    } else {
      window.location.href = `/${targetLang}/`;
    }
  </script>
</head>
<body>
  <p>Redirecting...</p>
</body>
</html>
```

Puis dans `netlify.toml` ou `vercel.json` :

```toml
# netlify.toml
[[redirects]]
  from = "/"
  to = "/redirect.html"
  status = 200
```

```json
// vercel.json
{
  "rewrites": [
    { "source": "/", "destination": "/redirect.html" }
  ]
}
```

## 📊 Monitoring et analytics

### 1. Vérifier les redirections de langue

Dans `public/index.php`, les logs sont envoyés à `error_log` :

```bash
# Sur le serveur, voir les logs PHP
tail -f /var/log/apache2/error.log | grep "Language detection"
```

### 2. Ajouter Google Analytics

Dans `src/layouts/Layout.astro`, ajoutez avant `</head>` :

```astro
<!-- Google Analytics -->
<script async src="https://www.googletagmanager.com/gtag/js?id=GA_MEASUREMENT_ID"></script>
<script is:inline>
  window.dataLayer = window.dataLayer || [];
  function gtag(){dataLayer.push(arguments);}
  gtag('js', new Date());
  gtag('config', 'GA_MEASUREMENT_ID');
</script>
```

## 🔧 Dépannage

### Problème : Les redirections ne fonctionnent pas

**Solution 1** : Vérifier que `mod_rewrite` est activé (Apache)

```bash
sudo a2enmod rewrite
sudo systemctl restart apache2
```

**Solution 2** : Vérifier que `.htaccess` est lu

```apache
# Dans la config Apache
<Directory /var/www/html>
    AllowOverride All
</Directory>
```

### Problème : index.php ne s'exécute pas

**Vérification** : PHP est installé et configuré

```bash
php -v
```

**Solution** : Vérifier la configuration PHP dans Apache

```bash
# Activer le module PHP
sudo a2enmod php8.1
sudo systemctl restart apache2
```

### Problème : Les assets (images/CSS) ne se chargent pas

**Cause** : Chemins incorrects ou permissions

**Solution** :

```bash
# Vérifier les permissions
chmod -R 755 /var/www/html/_astro
chmod -R 755 /var/www/html/img

# Vérifier les logs Apache
tail -f /var/log/apache2/error.log
```

### Problème : Le cookie de langue n'est pas mémorisé

**Cause** : Domaine ou SameSite incorrect

**Solution** : Dans `src/components/Header.astro`, ajuster le cookie :

```javascript
// Pour un sous-domaine spécifique
document.cookie = `preferred_language=${lang}; path=/; max-age=31536000; domain=.votredomaine.com; SameSite=Lax`;
```

## 🔄 Mise à jour du site

Pour mettre à jour le contenu :

1. Modifier les fichiers sources dans `src/`
2. Rebuild : `npm run build`
3. Transférer uniquement les fichiers modifiés de `dist/` vers le serveur

Pour mettre à jour uniquement les traductions :

1. Modifier les fichiers JSON dans `src/i18n/`
2. Rebuild et redéployer

## 📈 Performance

### Optimisations incluses

- ✅ Compression gzip (texte, CSS, JS)
- ✅ Cache navigateur (1 an pour images, 1 mois pour CSS/JS)
- ✅ Lazy loading des images de catégories
- ✅ Assets optimisés par Vite
- ✅ HTML minifié

### Optimisations supplémentaires recommandées

1. **CDN** : Utiliser Cloudflare ou AWS CloudFront
2. **WebP** : Convertir les images JPEG/PNG en WebP
3. **HTTP/2** : Activer sur le serveur
4. **Brotli** : Compression supérieure à gzip

```apache
# .htaccess - Ajouter compression Brotli si disponible
<IfModule mod_brotli.c>
    AddOutputFilterByType BROTLI_COMPRESS text/html text/plain text/xml text/css text/javascript application/javascript
</IfModule>
```

## 🔒 Sécurité

Les headers de sécurité sont déjà configurés dans `.htaccess` :

- ✅ XSS Protection
- ✅ Clickjacking protection (X-Frame-Options)
- ✅ MIME sniffing protection
- ✅ Referrer Policy

Pour une sécurité renforcée, ajoutez HTTPS :

```apache
# Forcer HTTPS
RewriteEngine On
RewriteCond %{HTTPS} off
RewriteRule ^(.*)$ https://%{HTTP_HOST}/$1 [R=301,L]
```

## 📞 Support

Pour tout problème de déploiement, consultez :

- [Documentation Astro](https://docs.astro.build/en/guides/deploy/)
- [Documentation Apache mod_rewrite](https://httpd.apache.org/docs/current/mod/mod_rewrite.html)
- [PHP Documentation](https://www.php.net/manual/en/)

---

**Déploiement testé avec succès sur** :
- ✅ Apache 2.4 + PHP 8.1
- ✅ Nginx 1.20 + PHP-FPM 8.1
- ⚠️ Netlify/Vercel (sans détection PHP, redirection statique)
