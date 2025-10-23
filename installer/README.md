# WallpaperAI - Générateur d'installateur NSIS

Ce répertoire contient les fichiers nécessaires pour générer des installateurs Windows professionnels pour WallpaperAI.

## Types d'installateurs disponibles

### 1. Installateur standard (`WallpaperAI.nsi`)
- Installe une version mono-langue (selon le build de l'app)
- Fichier généré : `WallpaperAI-Setup-1.0.1.exe`

### 2. Installateur multilingue (`WallpaperAI_Multilang.nsi`) ⭐ RECOMMANDÉ
- L'utilisateur choisit sa langue lors de l'installation
- Installe uniquement l'exécutable de la langue choisie
- 7 langues disponibles : FR, EN, ES, PT, IT, DE, RU
- Fichier généré : `WallpaperAI-Setup-Multilang-1.0.2.exe`

## Prérequis

**NSIS (Nullsoft Scriptable Install System)** doit être installé sur votre système.

### Installation de NSIS

1. Téléchargez NSIS depuis : https://nsis.sourceforge.io/Download
2. Installez NSIS (version 3.x recommandée)
3. Ajoutez le répertoire NSIS au PATH système (optionnel mais recommandé)
   - Exemple : `C:\Program Files (x86)\NSIS`

## Génération de l'installateur

### Installateur multilingue (RECOMMANDÉ)

**Étape 1 : Compiler tous les exécutables multilingues**
```bash
cd app
build.bat ALL
```

Cela génère 7 exécutables dans `app/release/` : WallpaperAI_FR.exe, WallpaperAI_EN.exe, etc.

**Étape 2 : Compiler l'installateur multilingue**
```bash
cd ..\installer
build_multilang_installer.bat
```

Le script va :
- Vérifier que NSIS est installé
- Vérifier que tous les 7 exécutables multilingues sont présents
- Compiler l'installateur NSIS
- Générer `WallpaperAI-Setup-Multilang-1.0.2.exe` (~20 MB)

### Installateur standard (mono-langue)

**Étape 1 : Compiler l'application dans une langue spécifique**
```bash
cd app
build.bat FR    # ou EN, ES, PT, IT, DE, RU
```

**Étape 2 : Compiler l'installateur**
```bash
cd ..\installer
build_installer.bat
```

Génère `WallpaperAI-Setup-1.0.1.exe`

### Compilation manuelle

```bash
cd installer
makensis WallpaperAI_Multilang.nsi   # Installateur multilingue
# ou
makensis WallpaperAI.nsi             # Installateur standard
```

## Structure de l'installateur

L'installateur NSIS généré :

### ✅ Fonctionnalités installées

- **Application complète** : Exécutable + toutes les DLL Qt nécessaires
- **Plugins Qt** : platforms, imageformats, iconengines, styles, tls
- **Ressources** : Images PNG (icônes, assets)
- **Raccourcis** :
  - Menu Démarrer : WallpaperAI + Désinstaller
  - Bureau : WallpaperAI

### ✅ Fonctionnalités de l'installateur

#### Installateur multilingue (`WallpaperAI_Multilang.nsi`)

- **Sélection de langue interactive** : Page dédiée pour choisir la langue de l'application
- **7 langues supportées** : Français, English, Español, Português, Italiano, Deutsch, Русский
- **Installation intelligente** : Installe uniquement l'exécutable de la langue choisie
- **Interface MUI2 multilingue** : L'installateur s'adapte à la langue du système Windows
- **Taille optimisée** : ~20 MB avec compression LZMA (au lieu de ~70 MB × 7)
- **Enregistrement de la langue** : Sauvegardée dans le registre pour référence

#### Fonctionnalités communes

- **Interface moderne** : MUI2 (Modern User Interface)
- **Détection de version** : Désinstalle automatiquement l'ancienne version si présente
- **Arrêt automatique** : Ferme l'application en cours d'exécution avant installation
- **Registre Windows** : Enregistrement dans Programmes et fonctionnalités
- **Désinstallateur** : Complet avec nettoyage des raccourcis et du registre
- **Préservation des données** : Les paramètres utilisateur dans AppData sont conservés lors de la désinstallation
- **Compression LZMA** : Taille optimisée avec compression Solid (~13.5% de la taille originale)

## Workflow complet de release

### Release multilingue (recommandé)

```bash
# 1. Compiler l'application pour toutes les langues
cd app
build.bat ALL

# 2. Générer l'installateur multilingue
cd ..\installer
build_multilang_installer.bat

# 3. Tester l'installateur
WallpaperAI-Setup-Multilang-1.0.2.exe
```

### Release mono-langue

```bash
# 1. Compiler l'application (version FR, EN, etc.)
cd app
build.bat FR

# 2. Générer l'installateur
cd ..\installer
build_installer.bat

# 3. Tester l'installateur
WallpaperAI-Setup-1.0.1.exe
```

## Configuration de la version

Pour changer la version de l'application, éditez `WallpaperAI.nsi` :

```nsis
!define APP_VERSION "1.0.0"
```

Cela génèrera automatiquement `WallpaperAI-Setup-1.0.0.exe`.

## Personnalisation

### Icône de l'installateur

L'icône est définie dans `WallpaperAI.nsi` :

```nsis
!define APP_ICON "..\app\assets\icon.png"
```

NSIS convertit automatiquement le PNG en ICO.

### Ajouter une licence

Décommentez cette ligne dans `WallpaperAI.nsi` :

```nsis
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"
```

Et créez un fichier `LICENSE.txt` à la racine du projet.

### Page de bienvenue personnalisée

Modifiez les textes dans `WallpaperAI.nsi` :

```nsis
!define MUI_WELCOMEPAGE_TITLE "Votre titre personnalisé"
!define MUI_WELCOMEPAGE_TEXT "Votre texte personnalisé"
```

## Dépannage

### "NSIS n'est pas reconnu comme commande interne"

- Vérifiez que NSIS est installé
- Ajoutez le répertoire NSIS au PATH système
- Redémarrez votre terminal

### "Impossible de trouver WallpaperAI.exe"

- Compilez d'abord l'application avec `cd app && build.bat`
- Vérifiez que `app/release/WallpaperAI.exe` existe

### L'installateur ne démarre pas

- Vérifiez que vous avez les droits administrateur
- Désactivez temporairement l'antivirus (il peut bloquer les exécutables non signés)

## Notes de sécurité

L'installateur généré n'est **pas signé numériquement**. Pour une distribution professionnelle :

1. Obtenez un certificat de signature de code
2. Signez l'installateur avec `signtool.exe` (Windows SDK)

```bash
signtool sign /f certificat.pfx /p motdepasse /t http://timestamp.digicert.com WallpaperAI-Setup-1.0.0.exe
```

## Fichiers générés

### Installateurs disponibles

- `WallpaperAI-Setup-Multilang-1.0.2.exe` : Installateur multilingue (~20 MB)
  - Contient les 7 versions linguistiques
  - L'utilisateur choisit sa langue à l'installation
  - Optimisé avec compression LZMA Solid

- `WallpaperAI-Setup-1.0.1.exe` : Installateur mono-langue (~3 MB)
  - Une seule langue intégrée
  - Plus léger mais moins flexible

## Licence

L'installateur est créé avec NSIS, qui est sous licence zlib/libpng.
