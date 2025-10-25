# WallpaperAI - Documentation Complète

Application desktop Qt6 C++ pour changer automatiquement les fonds d'écran avec des wallpapers générés par IA.

## 📋 Table des matières

- [Vue d'ensemble](#vue-densemble)
- [Architecture](#architecture)
- [Fonctionnalités](#fonctionnalités)
- [Compilation](#compilation)
- [Structure du projet](#structure-du-projet)
- [Composants](#composants)
- [Système multi-écrans](#système-multi-écrans)
- [Système multilingue](#système-multilingue)
- [Configuration](#configuration)
- [Développement](#développement)

---

## 🎯 Vue d'ensemble

**WallpaperAI** est une application desktop Windows qui permet de :
- Télécharger et appliquer automatiquement des fonds d'écran générés par IA
- Gérer plusieurs catégories de wallpapers (Cyberpunk, Ocean, Mountains, etc.)
- Supporter les configurations multi-écrans avec wallpapers différents par écran
- Changer automatiquement les fonds d'écran selon une fréquence configurable
- Fonctionner en arrière-plan via le system tray
- S'intégrer avec le démarrage automatique de Windows

**Technologies :**
- **Framework** : Qt 6.9.2 (C++17)
- **Compilateur** : MinGW GCC 13.1.0
- **Build system** : CMake 3.16+
- **API Backend** : REST PHP (voir `api/docs/README.md`)

---

## 🏗️ Architecture

### Architecture globale

```
┌─────────────────────────────────────────────────────────┐
│                    WallpaperAI.exe                      │
│                                                         │
│  ┌──────────────┐  ┌──────────────┐  ┌─────────────┐  │
│  │ Main Window  │  │  System Tray │  │   Timers    │  │
│  │  (3 Tabs)    │  │   (Hidden)   │  │ (Auto-swap) │  │
│  └──────┬───────┘  └──────┬───────┘  └──────┬──────┘  │
│         │                 │                  │         │
│         └─────────┬───────┴──────────────────┘         │
│                   │                                     │
│  ┌────────────────▼────────────────────────────────┐   │
│  │         WallpaperBuilder (Core)                 │   │
│  │  - Multi-screen composite generation           │   │
│  │  - Windows coordinate wrapping (4 quadrants)    │   │
│  │  - Native resolution handling                   │   │
│  └────────────────┬────────────────────────────────┘   │
│                   │                                     │
│  ┌────────────────▼────────────────────────────────┐   │
│  │         WallpaperManager (System)               │   │
│  │  - Windows API integration                      │   │
│  │  - Registry manipulation (SystemParametersInfo) │   │
│  └────────────────┬────────────────────────────────┘   │
│                   │                                     │
│  ┌────────────────▼────────────────────────────────┐   │
│  │          Network Layer (Qt Network)             │   │
│  │  - API calls (categories, wallpapers)           │   │
│  │  - Thumbnail cache management                   │   │
│  │  - Timestamp synchronization                    │   │
│  └─────────────────────────────────────────────────┘   │
│                   │                                     │
└───────────────────┼─────────────────────────────────────┘
                    │
         ┌──────────▼──────────┐
         │   REST API (PHP)    │
         │   - /categories     │
         │   - /wallpapers     │
         │   - /get/{file}     │
         │   - /mini/{file}    │
         └──────────┬──────────┘
                    │
         ┌──────────▼──────────┐
         │   FTP Server        │
         │   (FTPES secure)    │
         └─────────────────────┘
```

### Architecture modulaire

```
app/
├── main.cpp                      # Point d'entrée + MainWindow
├── src/
│   ├── core/
│   │   └── wallpaper_builder.cpp    # Génération d'images composites
│   ├── system/
│   │   └── wallpaper_manager.cpp    # Interaction Windows API
│   ├── config/
│   │   └── startup_manager.cpp      # Démarrage Windows (registre)
│   ├── utils/
│   │   ├── utils.cpp                # Utilitaires généraux
│   │   ├── path_helper.cpp          # Gestion chemins (AppData)
│   │   └── date_helper.cpp          # Formatage dates pour API
│   └── widgets/
│       ├── toggle_switch.cpp        # Toggle personnalisé
│       ├── screen_selector.cpp      # Sélecteur d'écrans
│       ├── countdown_widget.cpp     # Affichage timer circulaire
│       ├── hover_filters.cpp        # Effets hover sur boutons
│       └── apply_category_dialog.cpp # Popup appliquer catégorie
├── language.h                    # Système multilingue (header switch)
├── lang_*.h                      # Fichiers de langue (FR, EN, ES, PT, IT, DE, RU)
└── CMakeLists.txt               # Configuration build
```

---

## ⚡ Fonctionnalités

### 1. Gestion des fonds d'écran

#### Changement automatique
- **Fréquences disponibles** :
  - Changement manuel uniquement
  - Au démarrage (si démarrage Windows activé)
  - Toutes les 1, 3, 6, 12 heures
  - Tous les 1, 2, 7 jours
  - Fréquence personnalisée (minutes, heures, jours)

#### Modes d'ajustement
- **Fill** : Remplir l'écran (crop si nécessaire)
- **Fit** : Ajuster sans déformer (bandes noires possibles)
- **Span** : Étendre sur tous les écrans
- **Stretch** : Étirer pour remplir (peut déformer)
- **Tile** : Mosaïque répétée

#### Sélection intelligente
- **Pondération par étoiles** : Les catégories avec plus d'étoiles ont une probabilité plus élevée
- **Fallback en cascade** :
  1. Date actuelle
  2. Remontée jusqu'à 7 jours en arrière
  3. Exclusion des catégories vides
  4. Fallback sur l'historique local si API inaccessible
- **Évitement des doublons** : Ne pas afficher le même wallpaper deux fois de suite

---

### 2. Système multi-écrans

#### Détection automatique
- Nombre d'écrans détectés via `QGuiApplication::screens()`
- Rafraîchissement automatique si changement de configuration

#### Modes disponibles

**Mode single-screen (Image identique sur tous les écrans)**
- Un seul wallpaper appliqué à tous les écrans
- Mode par défaut

**Mode multi-screen (Image différente par écran)**
- Sélection manuelle des écrans cibles
- Génération d'une image composite qui couvre tout le bureau virtuel
- Support des configurations complexes (écrans en haut/bas/gauche/droite)

#### Architecture multi-écrans résolu

**Problème initial** : Tiling sur écrans haute résolution avec scaling Windows

**Solution implémentée** :
```cpp
struct ScreenMapping {
    QRect logicalGeometry;      // Coordonnées logiques Windows (position)
    QSize nativeResolution;     // Résolution native écran (taille réelle)
    int screenIndex;
};
```

**Fonctions clés** :
- `calculateVirtualDesktopBounds()` : Calcul des limites du bureau virtuel en résolution native
- `generateScreenMappings()` : Mapping de chaque écran (coordonnées + résolution)
- `createCompositeImageFromMappings()` : Génération de l'image composite
- `wrapCoordinatesForWindows()` : Système de wrapping pour Windows (quadrants A/B/C/D)

**Résultat** : Aucun tiling, wallpapers haute qualité sur tous les écrans

---

### 3. Gestion des catégories

#### Catégories disponibles

| ID | Nom | Description |
|----|-----|-------------|
| al | ALIEN PLANETS | Planètes extraterrestres |
| as | AUTUMN SEASONS | Paysages d'automne |
| cp | CUTE PETS | Animaux mignons |
| cf | CYBERPUNK FUTURISTIC | Cyberpunk et futuriste |
| dl | DESERT LANDSCAPES | Paysages désertiques |
| fw | FANTASY WORLDS | Mondes fantastiques |
| gp | GEOMETRIC PATTERNS | Motifs géométriques |
| if | INFERNO & FIRE | Feu et enfers |
| lg | LANDSCAPE WITH GIRL | Paysages avec personnages |
| mf | MAGICAL FANTASY | Magie et fantasy |
| ma | MANGA & ANIME | Manga et anime |
| mi | MINIMALIST ABSTRACT | Minimaliste et abstrait |
| mp | MOUNTAINS & PEAKS | Montagnes et sommets |
| mg | MYSTERIOUS GOTHIC | Gothique mystérieux |
| nl | NATURAL LANDSCAPES | Paysages naturels |
| om | OCEAN & MARINE | Océan et marin |
| py | PARAGUAY | Paraguay |
| rx | RELAXATION | Relaxation |
| rc | RETRO CARTOONS | Dessins animés rétro |
| sc | SPACE & COSMOS | Espace et cosmos |
| sp | STEAMPUNK | Steampunk |
| tp | TROPICAL PARADISE | Paradis tropical |
| ua | URBAN ARCHITECTURE | Architecture urbaine |
| vr | VINTAGE RETRO | Vintage et rétro |
| ww | WINTER WONDERLAND | Paysages d'hiver |

#### Système de notation (étoiles)
- **1 à 5 étoiles** par catégorie
- Plus d'étoiles = probabilité de sélection plus élevée
- Calcul de pondération lors de la sélection aléatoire

#### Interface catégories
- **Grille scrollable** avec miniatures
- **Popup détaillée** au clic sur une catégorie
  - Carrousel d'images avec navigation (précédent/suivant)
  - Bouton "Appliquer" pour changer immédiatement
  - Sélection d'écran (si multi-screen activé)
- **Toggle actif/inactif** : Activer/désactiver une catégorie

---

### 4. Historique

#### Stockage
- Sauvegarde dans `AppData/Local/WallpaperAI/history.json`
- Format JSON avec métadonnées complètes

**Structure d'une entrée** :
```json
{
  "filename": "neon_city_night.png",
  "category": "CYBERPUNK FUTURISTIC",
  "date": "29/09/2025",
  "timestamp": "2025-10-24T12:34:56",
  "adjustment_mode": "Fill"
}
```

#### Interface historique
- **Grille scrollable** avec miniatures
- **Clic pour réappliquer** un wallpaper précédent
- Limite de 100 entrées (les plus anciennes sont supprimées)

#### Cache des miniatures
- Stockage : `AppData/Local/WallpaperAI/thumbnails/`
- Format : JPEG 204x115px
- Limite : 100 fichiers max (nettoyage automatique des plus anciens)
- Partagé entre catégories et historique

---

### 5. System Tray

#### Fonctionnalités
- **Icône persistante** dans la barre des tâches
- **Menu contextuel** :
  - "Afficher" : Ouvrir la fenêtre principale
  - "Changer le fond d'écran" : Changement immédiat
  - "Quitter" : Fermer l'application

#### Comportement
- Minimiser → Cache dans le tray (fenêtre masquée)
- Fermer (X) → Minimise dans le tray (ne quitte pas)
- Démarrage Windows → Lance directement dans le tray (pas de fenêtre)

---

### 6. Démarrage automatique avec Windows

#### Implémentation
- **Registre Windows** : `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run`
- **Clé** : `WallpaperAI`
- **Valeur** : Chemin complet vers l'exécutable + `--startup`

**Fonctions (StartupManager)** :
```cpp
bool addToWindowsStartup();       // Ajouter au registre
bool removeFromWindowsStartup();  // Retirer du registre
bool isInWindowsStartup();        // Vérifier si présent
```

#### Argument --startup
- Détecté via `QCoreApplication::arguments()`
- Si présent → Démarrage direct dans le system tray
- Si "Au démarrage" sélectionné → Changement de fond d'écran avec délai 2s

---

### 7. Synchronisation avec l'API

#### Système de timestamp
- **API** : Retourne un timestamp basé sur `filemtime(wallpapers.csv)`
- **Application** : Sauvegarde le timestamp dans le cache local

**Workflow** :
1. Au démarrage, récupérer `/categories` avec timestamp
2. Comparer avec le timestamp en cache
3. Si différent → Recharger catégories + nettoyer cache thumbnails
4. Si identique → Utiliser le cache local (économie réseau)

**Logs** :
```
[CACHE] Mise à jour détectée (API: 1735171200 vs Local: 1735084800)
[CACHE] Catégories à jour, utilisation du cache
```

---

## 🔨 Compilation

### Prérequis

- **Qt 6.9.2** : Framework Qt avec MinGW
- **MinGW GCC 13.1.0** : Compilateur C++
- **CMake 3.16+** : Build system

### Installation Qt

1. Télécharger Qt Online Installer : https://www.qt.io/download
2. Installer Qt 6.9.2 pour MinGW
3. Chemin typique : `C:\Installation\Qt\6.9.2\mingw_64\`

### Compilation simple (une langue)

```bash
cd app
build.bat         # Français (défaut)
build.bat EN      # Anglais
build.bat ES      # Espagnol
build.bat PT      # Portugais
build.bat IT      # Italien
build.bat DE      # Allemand
build.bat RU      # Russe
```

### Compilation multilingue (toutes les langues)

```bash
cd app
build.bat ALL
```

**Résultat** : 7 exécutables dans `app/release/`
- `WallpaperAI_FR.exe` (~13 MB)
- `WallpaperAI_EN.exe` (~13 MB)
- `WallpaperAI_ES.exe` (~13 MB)
- `WallpaperAI_PT.exe` (~13 MB)
- `WallpaperAI_IT.exe` (~13 MB)
- `WallpaperAI_DE.exe` (~13 MB)
- `WallpaperAI_RU.exe` (~13 MB)

### Processus de compilation détaillé

**Étape 1 : Arrêt de l'application**
```batch
powershell "Stop-Process -Name 'WallpaperAI' -Force -ErrorAction SilentlyContinue"
```

**Étape 2 : Configuration environnement**
```batch
set PATH=C:\Installation\Qt\Tools\CMake_64\bin;...
set Qt6_DIR=C:\Installation\Qt\6.9.2\mingw_64\lib\cmake\Qt6
```

**Étape 3 : Configuration CMake**
```batch
cmake -G "MinGW Makefiles" -DCMAKE_CXX_FLAGS="-DLANG_FR" ..
```

**Étape 4 : Compilation**
```batch
cmake --build . --config Release
```

**Étape 5 : Déploiement Qt**
```batch
windeployqt.exe WallpaperAI.exe
```
→ Copie automatique des DLLs Qt nécessaires

**Étape 6 : Création du dossier release**
- Copie de l'exécutable
- Copie des DLLs Qt
- Copie des plugins Qt (platforms, imageformats, iconengines, styles, tls)
- Copie des assets PNG

**Résultat final** : `app/release/WallpaperAI.exe` (~76 MB au total)

---

## 📁 Structure du projet

### Fichiers principaux

```
app/
├── main.cpp                         # Point d'entrée + classe MainWindow
├── language.h                       # Switch de langue (préprocesseur)
├── lang_fr.h                        # Français (145 macros)
├── lang_en.h                        # Anglais (145 macros)
├── lang_es.h                        # Espagnol (145 macros)
├── lang_pt.h                        # Portugais (145 macros)
├── lang_it.h                        # Italien (145 macros)
├── lang_de.h                        # Allemand (145 macros)
├── lang_ru.h                        # Russe (145 macros)
├── CMakeLists.txt                   # Configuration CMake
├── build.bat                        # Script de compilation Windows
├── resources.qrc                    # Ressources Qt (icônes embarquées)
└── app.rc                           # Ressources Windows (icône .ico)
```

### Modules (src/)

```
src/
├── core/
│   ├── wallpaper_builder.h/.cpp     # Génération d'images composites multi-écrans
│
├── system/
│   ├── wallpaper_manager.h/.cpp     # Interface Windows API (SystemParametersInfo)
│
├── config/
│   ├── startup_manager.h/.cpp       # Gestion démarrage Windows (registre)
│
├── utils/
│   ├── utils.h/.cpp                 # Fonctions utilitaires générales
│   ├── path_helper.h/.cpp           # Chemins standardisés (AppData)
│   └── date_helper.h/.cpp           # Formatage dates pour API
│
└── widgets/
    ├── toggle_switch.h/.cpp         # Widget toggle personnalisé
    ├── screen_selector.h/.cpp       # Sélecteur d'écrans multi-moniteurs
    ├── countdown_widget.h/.cpp      # Widget timer circulaire
    ├── hover_filters.h/.cpp         # Filtres d'événements pour effets hover
    └── apply_category_dialog.h/.cpp # Dialog d'application de catégorie
```

### Ressources (assets/)

```
assets/
├── icon.ico                         # Icône application (Windows)
├── icon.png                         # Icône fallback
├── info.png                         # Icône informations
├── star_active.png                  # Étoile active (notation)
├── star_inactive.png                # Étoile inactive
├── disable_category.png             # Icône désactiver catégorie
├── red_cross.png                    # Croix rouge (suppression)
├── wallpaper_fill_icon.png          # Icône mode Fill
├── wallpaper_fit_icon.png           # Icône mode Fit
├── wallpaper_span_icon.png          # Icône mode Span
├── wallpaper_stretch_icon.png       # Icône mode Stretch
└── wallpaper_tile_icon.png          # Icône mode Tile
```

---

## 🧩 Composants

### 1. WallpaperBuilder (Core)

**Rôle** : Génération d'images composites pour multi-écrans

**Fonctions principales** :

```cpp
QRect calculateVirtualDesktopBounds(const QList<ScreenMapping> &mappings);
// Calcule les limites du bureau virtuel en résolution native

QList<ScreenMapping> generateScreenMappings(const QList<QScreen*> &screens);
// Génère les mappings écran par écran avec résolutions natives

QImage createCompositeImageFromMappings(
    const QList<ScreenMapping> &mappings,
    const QMap<int, QString> &wallpaperPaths
);
// Crée l'image composite finale à partir des mappings

QPair<int, int> wrapCoordinatesForWindows(int x, int y, int width, int height);
// Système de wrapping pour Windows (gestion quadrants A/B/C/D)
```

**Architecture du système de wrapping** :

Windows ne supporte pas les coordonnées négatives pour les wallpapers. Le système de wrapping transforme les coordonnées négatives en coordonnées positives en les "enroulant" autour du bureau virtuel.

```
Quadrant A (x≥0, y≥0) → Pas de wrapping
Quadrant B (x<0, y≥0) → Wrapping horizontal
Quadrant C (x≥0, y<0) → Wrapping vertical
Quadrant D (x<0, y<0) → Wrapping horizontal + vertical
```

---

### 2. WallpaperManager (System)

**Rôle** : Interaction avec l'API Windows pour appliquer les fonds d'écran

**Fonctions principales** :

```cpp
bool setWallpaper(const QString &imagePath, const QString &adjustmentMode);
// Applique un wallpaper avec le mode d'ajustement spécifié
// Modes : Fill, Fit, Span, Stretch, Tile

bool applyWallpaperSettings();
// Force Windows à rafraîchir le fond d'écran
// Appelle SystemParametersInfoW(SPI_SETDESKWALLPAPER)
```

**Valeurs de registre** :

| Mode | WallpaperStyle | TileWallpaper |
|------|----------------|---------------|
| Fill | 10 | 0 |
| Fit | 6 | 0 |
| Span | 22 | 0 |
| Stretch | 2 | 0 |
| Tile | 0 | 1 |

**Clés de registre** :
```
HKEY_CURRENT_USER\Control Panel\Desktop
- WallpaperStyle
- TileWallpaper
```

---

### 3. StartupManager (Config)

**Rôle** : Gestion du démarrage automatique avec Windows

**Fonctions** :

```cpp
bool addToWindowsStartup();
// Ajoute l'application au démarrage Windows
// HKCU\Software\Microsoft\Windows\CurrentVersion\Run

bool removeFromWindowsStartup();
// Retire l'application du démarrage

bool isInWindowsStartup();
// Vérifie si l'application est dans le démarrage
```

**Valeur ajoutée** :
```
"WallpaperAI" = "C:\Path\To\WallpaperAI.exe --startup"
```

---

### 4. ToggleSwitch (Widget)

**Rôle** : Widget toggle personnalisé avec design moderne

**Propriétés** :
- Largeur : 50px
- Hauteur : 24px
- Animation : Transition fluide 200ms
- Couleurs :
  - Inactif : `#555` (gris)
  - Actif : `#2196F3` (bleu)

**Implémentation** :
```cpp
void paintEvent(QPaintEvent *event) override;
// Dessin personnalisé du toggle avec QBrush circulaire

void mousePressEvent(QMouseEvent *event) override;
// Gestion du clic → emit toggled(bool)
```

---

### 5. ScreenSelector (Widget)

**Rôle** : Sélection visuelle des écrans pour multi-screen

**Affichage** :
- Grille de checkboxes représentant les écrans
- Numérotation : "Écran 1", "Écran 2", etc.
- Visibilité conditionnelle (seulement si multi-screen activé + plusieurs écrans)

**Fonctions** :

```cpp
QList<int> selectedScreens() const;
// Retourne la liste des indices d'écrans sélectionnés

int screenCount() const;
// Retourne le nombre total d'écrans détectés
```

---

### 6. CountdownWidget (Widget)

**Rôle** : Affichage du compte à rebours jusqu'au prochain changement

**Design** :
- Cercle progressif (QPainter avec arc)
- Texte centré avec temps restant
- Taille : 280x200px
- Couleur : `#2196F3` (bleu)

**Modes** :
1. **Timer actif** : Affiche le temps restant + cercle progressif
2. **Mode "Au démarrage"** : Encadré informatif bleu
3. **Mode "Changement manuel uniquement"** : Encadré informatif

**Signal** :
```cpp
void countdownExpired();
// Émis quand le timer atteint 0
```

---

### 7. ApplyCategoryDialog (Widget)

**Rôle** : Popup pour appliquer un wallpaper d'une catégorie

**Contenu** :
- Carrousel d'images (QLabel avec QPixmap)
- Boutons navigation (Précédent / Suivant)
- Sélecteur d'écran (si multi-screen activé)
- Bouton "Appliquer"

**Workflow** :
1. Utilisateur clique sur une catégorie
2. Popup s'ouvre avec la première image
3. Navigation avec ◀ ▶
4. Sélection d'écran (optionnel)
5. Clic "Appliquer" → Changement immédiat

---

## 🖥️ Système multi-écrans

### Problématique

Windows ne gère pas correctement les wallpapers multi-écrans avec :
- Écrans en positions négatives (à gauche ou au-dessus de l'écran principal)
- Écrans avec scaling (125%, 150%, etc.)
- Résolutions natives différentes

### Solution implémentée

#### 1. Structure ScreenMapping

```cpp
struct ScreenMapping {
    QRect logicalGeometry;      // Coordonnées logiques Windows
    QSize nativeResolution;     // Résolution native écran
    int screenIndex;            // Index de l'écran
};
```

**Distinction importante** :
- `logicalGeometry` : Position de l'écran dans le bureau virtuel Windows (peut être négatif)
- `nativeResolution` : Taille réelle de l'écran en pixels (toujours positive)

#### 2. Génération des mappings

```cpp
QList<ScreenMapping> generateScreenMappings(const QList<QScreen*> &screens) {
    QList<ScreenMapping> mappings;
    for (int i = 0; i < screens.size(); i++) {
        QScreen *screen = screens[i];
        ScreenMapping mapping;
        mapping.logicalGeometry = screen->geometry();  // Qt geometry (avec scaling)
        mapping.nativeResolution = screen->size() * screen->devicePixelRatio();
        mapping.screenIndex = i;
        mappings.append(mapping);
    }
    return mappings;
}
```

#### 3. Calcul du bureau virtuel

```cpp
QRect calculateVirtualDesktopBounds(const QList<ScreenMapping> &mappings) {
    int minX = 0, minY = 0, maxX = 0, maxY = 0;

    for (const auto &mapping : mappings) {
        QRect logicalGeom = mapping.logicalGeometry;
        QSize nativeSize = mapping.nativeResolution;

        minX = qMin(minX, logicalGeom.x());
        minY = qMin(minY, logicalGeom.y());
        maxX = qMax(maxX, logicalGeom.x() + nativeSize.width());
        maxY = qMax(maxY, logicalGeom.y() + nativeSize.height());
    }

    return QRect(minX, minY, maxX - minX, maxY - minY);
}
```

#### 4. Création de l'image composite

```cpp
QImage createCompositeImageFromMappings(
    const QList<ScreenMapping> &mappings,
    const QMap<int, QString> &wallpaperPaths
) {
    QRect virtualBounds = calculateVirtualDesktopBounds(mappings);
    QImage compositeImage(virtualBounds.width(), virtualBounds.height(),
                          QImage::Format_RGB32);
    compositeImage.fill(Qt::black);

    QPainter painter(&compositeImage);

    for (const auto &mapping : mappings) {
        QString wallpaperPath = wallpaperPaths.value(mapping.screenIndex);
        QImage wallpaperImage(wallpaperPath);

        QImage scaledWallpaper = wallpaperImage.scaled(
            mapping.nativeResolution,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        );

        // Position dans l'image composite (origine en haut-gauche)
        int x = mapping.logicalGeometry.x() - virtualBounds.x();
        int y = mapping.logicalGeometry.y() - virtualBounds.y();

        painter.drawImage(x, y, scaledWallpaper);
    }

    return compositeImage;
}
```

#### 5. Système de wrapping pour Windows

Windows ne supporte pas les coordonnées négatives. Le système de wrapping convertit les coordonnées négatives en positives en "enroulant" l'image.

```cpp
QPair<int, int> wrapCoordinatesForWindows(int x, int y, int width, int height) {
    int wrappedX = x;
    int wrappedY = y;

    if (x < 0) {
        wrappedX = width + x;  // Quadrant B ou D
    }

    if (y < 0) {
        wrappedY = height + y;  // Quadrant C ou D
    }

    return QPair<int, int>(wrappedX, wrappedY);
}
```

**Exemple** :
- Écran 1 : (0, 0) → Pas de wrapping
- Écran 2 : (-1920, 0) → Wrapping horizontal → (3840 - 1920, 0) = (1920, 0)

### Résultat

✅ Support complet des configurations multi-écrans :
- Écrans en positions négatives (gauche, haut)
- Résolutions natives différentes
- Scaling Windows (125%, 150%, etc.)
- Aucun tiling, images haute qualité

---

## 🌐 Système multilingue

### Architecture

Le système multilingue est basé sur des **macros de préprocesseur C++** définies à la compilation.

**Avantages** :
- ✅ Aucun fichier externe à distribuer
- ✅ Aucune surcharge à l'exécution
- ✅ Pas de dépendance au système de traduction Qt
- ✅ Simplicité maximale
- ✅ Performance optimale

### Fichiers de langue

**Format** : Headers C++ avec `#define`

```cpp
// lang_fr.h
#ifndef LANG_FR_H
#define LANG_FR_H

#define APP_NAME "WallpaperAI"
#define APP_TITLE "WallpaperAI - Fonds d'écrans générés par IA"
#define BTN_CHANGE_NOW "🖼️ Changer Maintenant"
// ... 145 macros au total

#endif
```

**7 langues supportées** :
- `lang_fr.h` : Français (défaut)
- `lang_en.h` : Anglais
- `lang_es.h` : Espagnol
- `lang_pt.h` : Portugais
- `lang_it.h` : Italien
- `lang_de.h` : Allemand
- `lang_ru.h` : Russe

### Système de sélection (language.h)

```cpp
#if defined(LANG_EN)
    #include "lang_en.h"
#elif defined(LANG_ES)
    #include "lang_es.h"
#elif defined(LANG_PT)
    #include "lang_pt.h"
#elif defined(LANG_IT)
    #include "lang_it.h"
#elif defined(LANG_DE)
    #include "lang_de.h"
#elif defined(LANG_RU)
    #include "lang_ru.h"
#elif defined(LANG_FR)
    #include "lang_fr.h"
#else
    #include "lang_fr.h"  // Défaut
#endif
```

### Processus de compilation

**Commande** : `build.bat EN`

**1. Script build.bat** :
```batch
set LANG=EN
cmake -DCMAKE_CXX_FLAGS="-DLANG_%LANG%" ..
```

**2. CMake génère** :
```
-DLANG_EN
```

**3. Préprocesseur C++** :
```cpp
#define LANG_EN  // Défini à la compilation
```

**4. language.h inclut** :
```cpp
#include "lang_en.h"
```

**5. Tous les textes sont remplacés** :
```cpp
setWindowTitle(APP_TITLE);
// Devient : setWindowTitle("WallpaperAI - AI-generated Wallpapers");
```

### Utilisation dans le code

```cpp
// ❌ Mauvais : Chaînes en dur
setWindowTitle("WallpaperAI");
label->setText("Cliquez pour changer");

// ✅ Bon : Macros de langue
setWindowTitle(APP_NAME);
label->setText(LBL_CLICK_TO_CHANGE);
```

### Macros disponibles (145 au total)

**Informations application** :
- `APP_NAME`, `APP_TITLE`

**Onglets** :
- `TAB_WALLPAPER`, `TAB_CATEGORIES`, `TAB_SETTINGS`

**Boutons** :
- `BTN_CHANGE_NOW`, `BTN_APPLY`, `BTN_PREV`, `BTN_NEXT`, `BTN_CLOSE`

**Messages** :
- `MSG_WALLPAPER_APPLIED`, `MSG_SAVE_ERROR`, `MSG_LOAD_ERROR`, etc.

**Paramètres** :
- `LBL_FREQUENCY`, `FREQ_MANUAL`, `FREQ_1H`, `LBL_STARTUP_WINDOWS`, etc.

**Modes d'ajustement** :
- `ADJ_FILL`, `ADJ_FIT`, `ADJ_SPAN`, `ADJ_STRETCH`, `ADJ_TILE`
- `EXPL_FILL`, `EXPL_FIT`, etc. (Explications)

**System Tray** :
- `TRAY_SHOW`, `TRAY_CHANGE_WALLPAPER`, `TRAY_QUIT`

---

## ⚙️ Configuration

### Stockage des paramètres

**Utilisation de QSettings** :
```cpp
QSettings settings("WallpaperAI", "WallpaperAI");
```

**Emplacement** :
```
HKEY_CURRENT_USER\Software\WallpaperAI\WallpaperAI\
```

**Paramètres sauvegardés** :
- `frequency` : Fréquence de changement (string)
- `customValue` : Valeur personnalisée (int)
- `customUnit` : Unité personnalisée (string: "minutes", "hours", "days")
- `adjustmentMode` : Mode d'ajustement (string: "Fill", "Fit", etc.)
- `startWithWindows` : Démarrage Windows (bool)
- `changeOnStartup` : Changement au démarrage (bool)
- `multiScreen` : Multi-écran activé (bool)
- `categories/{id}/active` : Catégorie active (bool)
- `categories/{id}/rating` : Notation (int 1-5)

### Stockage des données

**AppData Local** :
```
C:\Users\{Username}\AppData\Local\WallpaperAI\
```

**Fichiers** :
- `history.json` : Historique des wallpapers appliqués
- `categories_cache.json` : Cache des catégories API
- `thumbnails/` : Cache des miniatures (JPEG 204x115px)

### Cache API

**categories_cache.json** :
```json
{
  "timestamp": 1735171200,
  "categories": [
    {
      "id": "cf",
      "name": "CYBERPUNK FUTURISTIC",
      "thumbnail_url": "https://api.domain.com/mini/wallpaper.png"
    }
  ]
}
```

**Synchronisation** :
- Compare le timestamp API vs cache local
- Si différent → Recharge + nettoie thumbnails
- Si identique → Utilise le cache (économie réseau)

---

## 🔧 Développement

### Environnement de développement

**IDE recommandé** : Visual Studio Code
- Extension : CMake Tools
- Extension : C/C++ (Microsoft)
- Extension : Qt for Python (pour IntelliSense Qt)

**Configuration CMake** :
```json
{
  "cmake.configureArgs": [
    "-DCMAKE_CXX_FLAGS=-DLANG_FR"
  ]
}
```

### Debug

**Mode Debug** :
```batch
cd app/build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

**Flags Debug** :
- `-g` : Symboles de debug
- `-O0` : Pas d'optimisation

### Ajout d'une nouvelle langue

**Étapes** :

1. **Copier un fichier de langue** :
```bash
cp lang_fr.h lang_ja.h  # Japonais
```

2. **Traduire toutes les macros** :
```cpp
// lang_ja.h
#define APP_TITLE "WallpaperAI - AIで生成された壁紙"
// ... 145 macros
```

3. **Ajouter dans language.h** :
```cpp
#elif defined(LANG_JA)
    #include "lang_ja.h"
```

4. **Ajouter dans build.bat** :
```batch
for %%L in (FR EN ES PT IT DE RU JA) do (
```

5. **Compiler** :
```batch
build.bat JA
```

### Ajout d'une nouvelle fonctionnalité

**Workflow** :

1. Modifier le code source
2. **Compiler immédiatement** : `build.bat`
3. Corriger les erreurs de compilation
4. Tester la fonctionnalité
5. Commit (sans push)

**Règles de style** :
- Espacement : 10px entre éléments
- Largeurs fixes : 280px pour containers
- Couleurs : `#2196F3` (bleu), `#d14836` (rouge-orange), `#8b4513` (marron)
- Commentaires : Autorisés dans le code principal

---

## 📊 Statistiques

**Code source** :
- Fichiers C++ : ~15 fichiers (.cpp + .h)
- Lignes de code : ~5000 lignes (estimation)
- Fichiers de langue : 7 fichiers × 145 macros = 1015 définitions

**Taille de l'application** :
- Exécutable : ~13 MB
- Avec DLLs Qt : ~76 MB au total
- Installeur multilingue : ~20 MB (compression LZMA)

**Performance** :
- Démarrage : < 1 seconde
- Changement de wallpaper : < 2 secondes
- Génération composite multi-écrans : < 500ms

---

## 🔗 Liens

- **API Backend** : Voir `api/docs/README.md`
- **Installeur NSIS** : Voir `installer/README.md`
- **Consignes développement** : Voir `CLAUDE.md` (racine)

---

## 📄 Licence

Application développée avec [Claude Code](https://claude.com/claude-code)

---

**Version** : 1.0.2
**Dernière mise à jour** : Octobre 2025
