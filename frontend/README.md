# WallpaperAI Landing Page

Landing page multilingue pour l'application WallpaperAI, construite avec Astro.

## 🌍 Langues supportées

- 🇬🇧 Anglais (par défaut)
- 🇫🇷 Français
- 🇪🇸 Espagnol
- 🇵🇹 Portugais
- 🇮🇹 Italien
- 🇩🇪 Allemand
- 🇷🇺 Russe

## 🎯 Fonctionnalités

- **Design moderne et responsive** : Interface adaptée à tous les écrans
- **Navigation multilingue** : Sélecteur de langue dans l'en-tête
- **Carrousel de screenshots** : Présentation interactive de l'application
- **Catégories dynamiques** : Chargement via l'API REST
- **Optimisé pour la vente** : Contenu orienté conversion

## 📂 Structure du projet

```
frontend/
├── src/
│   ├── components/         # Composants réutilisables
│   │   ├── Header.astro    # En-tête avec sélecteur de langues
│   │   ├── Hero.astro      # Bannière principale
│   │   ├── Features.astro  # Fonctionnalités + carrousel
│   │   ├── Categories.astro # Liste des catégories (API)
│   │   ├── Download.astro  # Section de téléchargement
│   │   └── Footer.astro    # Pied de page
│   ├── layouts/
│   │   └── Layout.astro    # Layout principal avec meta tags
│   ├── pages/
│   │   ├── [lang].astro    # Pages localisées dynamiques
│   │   └── index.astro     # Redirection vers /en
│   ├── i18n/               # Fichiers de traduction JSON
│   │   ├── en.json
│   │   ├── fr.json
│   │   ├── es.json
│   │   ├── pt.json
│   │   ├── it.json
│   │   ├── de.json
│   │   └── ru.json
│   └── config.ts           # Configuration (API URL, langues)
├── public/
│   └── img/                # Images et screenshots
└── package.json
```

## 🚀 Démarrage

### Prérequis

- Node.js 18+
- npm ou yarn

### Installation

```bash
npm install
```

### Développement

```bash
npm run dev
```

Le site sera accessible sur `http://localhost:4321`

### Build de production

```bash
npm run build
```

Les fichiers statiques seront générés dans `dist/`

### Prévisualisation du build

```bash
npm run preview
```

## ⚙️ Configuration

Le fichier [src/config.ts](src/config.ts) contient :

- `apiBaseUrl` : URL de base de l'API REST (actuellement `https://kazflow.com`)
- `thumbnailsPath` : Chemin des miniatures (actuellement `/wallpaperai/api/miniatures/`)
- `supportedLanguages` : Liste des langues supportées
- `defaultLanguage` : Langue par défaut (`en`)

## 🎨 Sections de la landing page

### 1. Header
- Logo WallpaperAI
- Sélecteur de langues (dropdown)
- Bouton "Télécharger"

### 2. Hero (Bannière principale)
- Titre accrocheur
- Sous-titre et description
- Bouton CTA de téléchargement
- Badges : "100% Gratuit", "Sans Publicité", "Illimité"

### 3. Screenshots
- Carrousel interactif avec 3 captures d'écran
- Navigation avec flèches et points
- Auto-play toutes les 5 secondes

### 4. Features (Fonctionnalités)
Grille de 6 fonctionnalités principales :
- Variété infinie d'images IA
- Mises à jour automatiques quotidiennes
- Centaines de catégories
- Support multi-écrans
- Totalement gratuit et sans publicité
- Léger et rapide

### 5. Categories
- Chargement dynamique via API (`GET /categories`)
- Affichage des miniatures (depuis `/wallpaperai/api/miniatures/`)
- Notation par étoiles (1-5)
- Effet hover avec agrandissement

### 6. Download (Appel à l'action final)
- Titre de conversion
- Bouton de téléchargement principal
- Informations : Version, OS supporté, Taille

### 7. Footer
- Logo et nom
- Lien vers Kazflow
- Copyright dynamique

## 🌐 URLs générées

Le build génère les pages suivantes :

- `/` → Redirige vers `/en`
- `/en/` → Version anglaise
- `/fr/` → Version française
- `/es/` → Version espagnole
- `/pt/` → Version portugaise
- `/it/` → Version italienne
- `/de/` → Version allemande
- `/ru/` → Version russe

## 🎨 Palette de couleurs

Conforme au design de l'application :

- **Bleu principal** : `#2196F3` (boutons, accents)
- **Bleu foncé** : `#1976D2` (dégradés)
- **Rouge-orange** : `#d14836` (éléments secondaires)
- **Marron** : `#8b4513` (accents subtils)
- **Fond sombre** : `#0f172a` / `#1e293b`

## 📝 Ajout d'une nouvelle langue

1. Créer le fichier de traduction dans `src/i18n/XX.json`
2. Ajouter la langue dans `src/config.ts` :
   ```ts
   supportedLanguages: ['en', 'fr', ..., 'XX']
   ```
3. Rebuild l'application

Le système générera automatiquement la page `/XX/`

## 🔧 Modification du contenu

Les traductions sont dans `src/i18n/*.json`. Structure :

```json
{
  "lang": "en",
  "langName": "English",
  "meta": { "title": "...", "description": "..." },
  "header": { "downloadNow": "..." },
  "hero": { "title": "...", "subtitle": "...", ... },
  "features": { "title": "...", "feature1": { ... }, ... },
  "screenshots": { "title": "...", ... },
  "categories": { "title": "...", ... },
  "download": { "title": "...", ... },
  "footer": { "madeWith": "...", ... }
}
```

## 🚀 Déploiement

Le site est statique et peut être déployé sur :

- **Apache/Nginx** : Copier le contenu de `dist/` dans le répertoire web
- **Vercel/Netlify** : Déploiement automatique depuis Git
- **GitHub Pages** : Build automatique avec GitHub Actions

### Configuration Apache

Si déployé dans un sous-répertoire (`/wallpaperai/`), ajouter dans `.htaccess` :

```apache
RewriteEngine On
RewriteBase /wallpaperai/

# Redirection vers /en si accès à la racine
RewriteRule ^$ en/ [R=301,L]

# Support des URLs propres
RewriteCond %{REQUEST_FILENAME} !-f
RewriteCond %{REQUEST_FILENAME} !-d
RewriteRule ^([^/]+)/?$ $1/index.html [L]
```

## 📊 Performance

- **Lighthouse Score** : Optimisé pour 90+ sur tous les critères
- **Taille du build** : ~100 KB (HTML/CSS/JS compressé)
- **Images** : Lazy loading pour les catégories
- **SEO** : Meta tags optimisés pour chaque langue

## 🔗 Intégration API

L'application utilise l'API REST WallpaperAI :

- **Endpoint** : `GET https://kazflow.com/categories`
- **Format** : JSON avec `categories[]` contenant :
  - `id` : Identifiant unique
  - `name` : Nom de la catégorie
  - `rating` : Note de 1 à 5 étoiles
  - `thumbnail` : Nom du fichier miniature

Les miniatures sont chargées depuis :
`/wallpaperai/api/miniatures/{thumbnail}`

## 📱 Responsive Design

Breakpoints :

- **Desktop** : > 768px (grille complète, navigation étendue)
- **Mobile** : ≤ 768px (grille adaptée, boutons compacts)

## 🎯 Objectifs marketing

Le contenu est orienté pour :

1. **Mettre en avant la gratuité** : "100% gratuit, sans publicité"
2. **Souligner la diversité** : "Infinité de fonds d'écran IA"
3. **Promouvoir l'automatisation** : "Change chaque jour automatiquement"
4. **Valoriser la qualité IA** : "Images hyper-réalistes et imaginaires"
5. **Rassurer sur la simplicité** : "Interface intuitive, installation facile"

---

**Développé avec [Astro](https://astro.build/) 🚀**
