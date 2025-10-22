# Système Multilingue - WallpaperAI

## 📖 Vue d'ensemble

L'application WallpaperAI dispose maintenant d'un système multilingue complet basé sur des fichiers header C++. Ce système permet de compiler l'application dans différentes langues simplement en utilisant un paramètre de compilation.

## 🗂️ Architecture

### Fichiers de langue

Le système utilise trois fichiers principaux :

1. **`language.h`** : Fichier principal qui inclut le bon fichier de langue en fonction du `#define` de compilation
2. **`lang_fr.h`** : Contient tous les textes en français (langue par défaut)
3. **`lang_en.h`** : Contient tous les textes en anglais

### Structure des fichiers de langue

Chaque fichier de langue (`lang_XX.h`) contient des `#define` pour tous les textes de l'interface :

```cpp
#define APP_NAME "WallpaperAI"
#define APP_TITLE "WallpaperAI - Fonds d'écrans générés par IA"
#define BTN_CHANGE_NOW "🖼️ Changer Maintenant"
// ... etc
```

## 🔨 Compilation

### Compilation avec la langue par défaut (français)

```bash
cd app
./build.bat
```

ou explicitement :

```bash
./build.bat FR
```

### Compilation en anglais

```bash
./build.bat EN
```

### Compilation pour d'autres langues

Pour ajouter une nouvelle langue (par exemple allemand) :

1. Créer un fichier `lang_de.h` en copiant `lang_fr.h`
2. Traduire tous les `#define` en allemand
3. Ajouter le cas dans `language.h` :

```cpp
#if defined(LANG_EN)
    #include "lang_en.h"
#elif defined(LANG_DE)
    #include "lang_de.h"
#elif defined(LANG_FR)
    #include "lang_fr.h"
#else
    // Langue par défaut : français
    #include "lang_fr.h"
#endif
```

4. Compiler avec : `./build.bat DE`

## 📝 Utilisation dans le code

Dans `main.cpp` et les autres fichiers source, utilisez simplement les macros au lieu de chaînes en dur :

```cpp
// ❌ Ancien code
setWindowTitle("WallpaperAI");
QLabel *label = new QLabel("Cliquez pour changer");

// ✅ Nouveau code
setWindowTitle(APP_NAME);
QLabel *label = new QLabel(LBL_CLICK_TO_CHANGE);
```

## 🎯 Macros disponibles

### Informations de l'application
- `APP_NAME` : Nom de l'application
- `APP_TITLE` : Titre complet de l'application

### Onglets
- `TAB_WALLPAPER` : Onglet fond d'écran
- `TAB_CATEGORIES` : Onglet catégories
- `TAB_SETTINGS` : Onglet paramètres

### Boutons principaux
- `BTN_CHANGE_NOW` : Bouton changer maintenant
- `BTN_APPLY` : Bouton appliquer
- `BTN_PREV` / `BTN_NEXT` : Boutons de navigation

### Messages d'état
- `MSG_WALLPAPER_APPLIED` : Fond d'écran appliqué avec succès
- `MSG_SAVE_ERROR` : Erreur de sauvegarde
- `MSG_LOAD_ERROR` : Erreur de chargement
- `MSG_DOWNLOAD_ERROR` : Erreur de téléchargement
- `MSG_NO_IMAGE_AVAILABLE` : Aucune image disponible
- `MSG_NO_IMAGE_7DAYS` : Aucune image sur 7 jours
- `MSG_API_ERROR` : Erreur API

### Paramètres
- `LBL_FREQUENCY` : Label de fréquence
- `FREQ_MANUAL`, `FREQ_1H`, `FREQ_3H`, etc. : Options de fréquence
- `UNIT_MINUTES`, `UNIT_HOURS`, `UNIT_DAYS` : Unités de temps
- `LBL_STARTUP_WINDOWS` : Démarrer avec Windows
- `LBL_CHANGE_ON_STARTUP` : Changer au démarrage
- `LBL_MULTI_SCREEN` : Multi-écran

### Modes d'ajustement
- `ADJ_FILL`, `ADJ_FIT`, `ADJ_SPAN`, `ADJ_STRETCH`, `ADJ_TILE` : Modes
- `EXPL_FILL`, `EXPL_FIT`, etc. : Explications des modes

### System Tray
- `TRAY_SHOW` : Afficher
- `TRAY_CHANGE_WALLPAPER` : Changer le fond d'écran
- `TRAY_QUIT` : Quitter

### Autres
- `LBL_HISTORY` : Historique
- `LBL_NEXT_CHANGE` : Prochain changement
- `LBL_CLICK_TO_CHANGE` : Message par défaut

## 🌐 Langues actuellement supportées

- 🇫🇷 **Français (FR)** - Langue par défaut
- 🇬🇧 **Anglais (EN)** - Complètement traduit

## ⚙️ Technique

Le système fonctionne à la compilation :

1. Le script `build.bat` reçoit un paramètre de langue (ex: `EN`)
2. Ce paramètre est transformé en define CMake : `-DLANG_EN`
3. À la compilation, le préprocesseur C++ inclut le bon fichier de langue
4. Tous les textes sont remplacés par leurs versions traduites
5. L'exécutable généré est entièrement dans la langue choisie

**Avantages :**
- ✅ Pas de fichier de langue externe à distribuer
- ✅ Pas de surcharge à l'exécution
- ✅ Pas de dépendance au système de traduction Qt
- ✅ Simplicité maximale
- ✅ Performance optimale

## 📌 Notes importantes

- La langue est définie **à la compilation**, pas à l'exécution
- Pour changer de langue, il faut recompiler l'application
- Chaque exécutable est monolingue (taille optimisée)
- Les fichiers `lang_XX.h` doivent rester synchronisés (mêmes macros)
- **Tous** les widgets utilisant du texte doivent inclure `language.h`

## 🔄 Processus pour ajouter une nouvelle chaîne

1. Ajouter le `#define` dans **tous** les fichiers de langue
2. Utiliser le define dans le code source
3. Recompiler pour chaque langue

Exemple :
```cpp
// Dans lang_fr.h
#define NEW_BUTTON_TEXT "Nouveau bouton"

// Dans lang_en.h
#define NEW_BUTTON_TEXT "New button"

// Dans le code
QPushButton *btn = new QPushButton(NEW_BUTTON_TEXT);
```
