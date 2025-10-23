# WallpaperAI - Générateur d'installateur NSIS

Ce répertoire contient les fichiers nécessaires pour générer un installateur Windows professionnel pour WallpaperAI.

## Prérequis

**NSIS (Nullsoft Scriptable Install System)** doit être installé sur votre système.

### Installation de NSIS

1. Téléchargez NSIS depuis : https://nsis.sourceforge.io/Download
2. Installez NSIS (version 3.x recommandée)
3. Ajoutez le répertoire NSIS au PATH système (optionnel mais recommandé)
   - Exemple : `C:\Program Files (x86)\NSIS`

## Génération de l'installateur

### Méthode 1 : Script automatique (recommandé)

```bash
cd installer
build_installer.bat
```

Le script va :
- Vérifier que NSIS est installé
- Vérifier que l'application est compilée dans `app/release/`
- Compiler l'installateur NSIS
- Générer `WallpaperAI-Setup-1.0.0.exe`

### Méthode 2 : Compilation manuelle

```bash
cd installer
makensis WallpaperAI.nsi
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

- **Interface moderne** : MUI2 (Modern User Interface)
- **Multilingue** : Français + Anglais
- **Détection de version** : Désinstalle automatiquement l'ancienne version si présente
- **Arrêt automatique** : Ferme l'application en cours d'exécution avant installation
- **Registre Windows** : Enregistrement dans Programmes et fonctionnalités
- **Désinstallateur** : Complet avec nettoyage des raccourcis et du registre
- **Préservation des données** : Les paramètres utilisateur dans AppData sont conservés lors de la désinstallation
- **Compression LZMA** : Taille optimisée de l'installateur

## Workflow complet de release

```bash
# 1. Compiler l'application (version FR ou EN)
cd app
build.bat

# 2. Générer l'installateur
cd ..\installer
build_installer.bat

# 3. Tester l'installateur
WallpaperAI-Setup-1.0.0.exe
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

- `WallpaperAI-Setup-1.0.0.exe` : Installateur autonome (~70 MB)
- Taille compressée grâce à LZMA

## Licence

L'installateur est créé avec NSIS, qui est sous licence zlib/libpng.
