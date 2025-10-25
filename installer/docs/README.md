# WallpaperAI - Installateur NSIS Multilingue

Documentation complète du système d'installation Windows pour WallpaperAI.

## 📋 Table des matières

- [Vue d'ensemble](#vue-densemble)
- [Prérequis](#prérequis)
- [Fichiers du projet](#fichiers-du-projet)
- [Architecture de l'installateur](#architecture-de-linstallateur)
- [Génération de l'installateur](#génération-de-linstallateur)
- [Fonctionnalités détaillées](#fonctionnalités-détaillées)
- [Configuration](#configuration)
- [Personnalisation](#personnalisation)
- [Dépannage](#dépannage)
- [Distribution](#distribution)

---

## 🎯 Vue d'ensemble

L'installateur WallpaperAI est un **installateur NSIS multilingue intelligent** qui permet à l'utilisateur de :
- Choisir la langue de l'application parmi 7 langues disponibles
- Installer uniquement le binaire de la langue sélectionnée (optimisation de l'espace)
- Bénéficier d'une interface moderne MUI2 adaptée à la langue du système Windows
- Désinstaller proprement l'application avec préservation des données utilisateur

**Caractéristiques techniques** :
- **Framework** : NSIS 3.11+ (Nullsoft Scriptable Install System)
- **Interface** : MUI2 (Modern User Interface 2)
- **Compression** : LZMA Solid (taux de ~13.5%)
- **Encodage** : UTF-8 avec BOM (support Unicode complet)
- **Taille** : ~20 MB (au lieu de ~140 MB si 7 exécutables inclus)

---

## 📦 Prérequis

### Installation de NSIS

**Version requise** : NSIS 3.11 ou supérieur

**Téléchargement** : https://nsis.sourceforge.io/Download

**Installation** :
1. Exécuter l'installateur NSIS
2. Installer dans `C:\Program Files (x86)\NSIS` ou `C:\Installation\NSIS`
3. (Optionnel) Ajouter au PATH système pour accès global

**Vérification** :
```batch
makensis /VERSION
```

### Compilation préalable de l'application

Les 7 exécutables multilingues doivent être compilés avant de générer l'installateur :

```bash
cd app
build.bat ALL
```

**Résultat attendu** : 7 fichiers dans `app/release/`
- `WallpaperAI_FR.exe` (~13 MB)
- `WallpaperAI_EN.exe` (~13 MB)
- `WallpaperAI_ES.exe` (~13 MB)
- `WallpaperAI_PT.exe` (~13 MB)
- `WallpaperAI_IT.exe` (~13 MB)
- `WallpaperAI_DE.exe` (~13 MB)
- `WallpaperAI_RU.exe` (~13 MB)

---

## 📁 Fichiers du projet

### Structure du répertoire installer/

```
installer/
├── WallpaperAI.nsi                      # Script NSIS principal
├── build_installer.bat                   # Script de build automatique
├── convert_to_utf8_bom.ps1              # Utilitaire d'encodage UTF-8 BOM
├── .gitignore                           # Exclusion des .exe générés
└── docs/
    └── README.md                        # Cette documentation
```

### WallpaperAI.nsi (Script NSIS principal)

**Taille** : ~14 KB
**Encodage** : UTF-8 avec BOM (requis pour Unicode)
**Version actuelle** : 1.0.3

**Sections principales** :
1. **Configuration** : Defines, version, compression
2. **Variables** : `SelectedLanguage`, `LanguageCode`
3. **Interface MUI2** : Pages et langues de l'installateur
4. **Strings** : Traductions pour la page de sélection de langue
5. **Fonctions** : `.onInit`, `LanguageSelectionPage`, etc.
6. **Section Install** : Installation des fichiers
7. **Section Uninstall** : Désinstallation propre

### build_installer.bat (Script de build)

**Rôle** : Automatise la génération de l'installateur

**Workflow** :
1. Vérifie la présence de NSIS
2. Affiche la version de NSIS
3. Vérifie la présence des 7 exécutables (FR, EN, ES, PT, IT, DE, RU)
4. Compile le script NSIS avec verbosité niveau 3
5. Affiche les informations du fichier généré

### convert_to_utf8_bom.ps1 (Utilitaire PowerShell)

**Rôle** : Convertit un fichier NSIS en UTF-8 avec BOM

**Pourquoi nécessaire ?**
NSIS nécessite le BOM (Byte Order Mark U+FEFF) au début du fichier pour détecter correctement l'encodage UTF-8. Sans BOM, NSIS interprète le fichier en ACP (ANSI Code Page), causant des problèmes d'affichage des caractères accentués.

**Utilisation** :
```powershell
.\convert_to_utf8_bom.ps1 WallpaperAI.nsi
```

**Vérification** :
```batch
# NSIS doit afficher "Processing script file: (UTF8)" et non "(ACP)"
makensis /V2 WallpaperAI.nsi
```

---

## 🏗️ Architecture de l'installateur

### Flux d'installation

```
┌─────────────────────────────────────────────────────────┐
│  Lancement de WallpaperAI-Setup-Multilang-1.0.3.exe    │
└────────────────────┬────────────────────────────────────┘
                     │
         ┌───────────▼───────────┐
         │  Page de bienvenue    │
         │  (MUI_PAGE_WELCOME)   │
         └───────────┬───────────┘
                     │
         ┌───────────▼──────────────────────────────────┐
         │  Page de sélection de langue personnalisée   │
         │  - Dropdown avec 7 langues                   │
         │  - Pré-sélection selon langue système        │
         │  → Variable $LanguageCode = FR/EN/ES/etc     │
         └───────────┬──────────────────────────────────┘
                     │
         ┌───────────▼───────────┐
         │  Choix du répertoire  │
         │  (MUI_PAGE_DIRECTORY) │
         │  Défaut: C:\Program   │
         │  Files\WallpaperAI    │
         └───────────┬───────────┘
                     │
         ┌───────────▼────────────────────────────────────┐
         │  Installation (MUI_PAGE_INSTFILES)             │
         │  1. Arrêt de l'application (taskkill)          │
         │  2. Copie WallpaperAI_XX.exe → WallpaperAI.exe│
         │  3. Copie DLLs Qt (10 fichiers)                │
         │  4. Copie plugins Qt (platforms, etc.)         │
         │  5. Copie assets PNG (11 fichiers)             │
         │  6. Création désinstallateur                   │
         │  7. Enregistrement registre Windows            │
         │  8. Création raccourcis (Bureau + Démarrer)    │
         └───────────┬────────────────────────────────────┘
                     │
         ┌───────────▼───────────┐
         │  Page de fin          │
         │  (MUI_PAGE_FINISH)    │
         │  ☑ Lancer WallpaperAI │
         └───────────────────────┘
```

### Sélection intelligente de la langue

**Mécanisme** :

1. **Détection langue système** (fonction `.onInit`) :
```nsis
${If} $LANGUAGE == ${LANG_FRENCH}
    StrCpy $LanguageCode "FR"
${ElseIf} $LANGUAGE == ${LANG_ENGLISH}
    StrCpy $LanguageCode "EN"
${Else}
    StrCpy $LanguageCode "EN"  # Défaut : Anglais
${EndIf}
```

2. **Page de sélection personnalisée** (`LanguageSelectionPage`) :
- Dropdown avec 7 options : "Français (French)", "English", "Español (Spanish)", etc.
- Pré-sélection basée sur `$LanguageCode`
- Modification possible par l'utilisateur

3. **Récupération du choix** (`LanguageSelectionLeave`) :
- Parsing du texte sélectionné
- Mise à jour de `$LanguageCode`

4. **Installation conditionnelle** (Section Install) :
```nsis
${If} $LanguageCode == "FR"
    File /oname=${APP_EXE} "..\app\release\WallpaperAI_FR.exe"
${ElseIf} $LanguageCode == "EN"
    File /oname=${APP_EXE} "..\app\release\WallpaperAI_EN.exe"
# ... pour chaque langue
${EndIf}
```

### Compression LZMA Solid

**Configuration** :
```nsis
SetCompressor /SOLID lzma
SetCompressorDictSize 64
```

**Résultats** :
- Taille originale : ~154 MB (7 × 13 MB + DLLs + assets)
- Taille compressée : ~20 MB
- **Taux de compression : 13.5%**

**Avantages LZMA Solid** :
- Compression en un seul bloc (meilleur ratio)
- Décompression rapide lors de l'installation
- Idéal pour fichiers similaires (7 exécutables quasi-identiques)

---

## 🔨 Génération de l'installateur

### Méthode 1 : Script automatique (recommandé)

**Commandes** :
```bash
# Étape 1 : Compiler tous les exécutables
cd app
build.bat ALL

# Étape 2 : Générer l'installateur
cd ..\installer
build_installer.bat
```

**Sortie attendue** :
```
========================================
 Build WallpaperAI Multilang Installer
========================================

NSIS trouve: C:\Installation\NSIS\makensis.exe
Version de NSIS: v3.11

Verification des executables sources...
[OK] WallpaperAI_FR.exe
[OK] WallpaperAI_EN.exe
[OK] WallpaperAI_ES.exe
[OK] WallpaperAI_PT.exe
[OK] WallpaperAI_IT.exe
[OK] WallpaperAI_DE.exe
[OK] WallpaperAI_RU.exe

Tous les executables sont presents

Compilation de l'installateur multilingue...

Processing script file: "WallpaperAI.nsi" (UTF8)
Output: "WallpaperAI-Setup-Multilang-1.0.3.exe"
Total size: 20915384 / 153880060 bytes (13.5%)

========================================
 BUILD REUSSI !
========================================

Installateur cree: WallpaperAI-Setup-Multilang-1.0.3.exe
Taille: 20915384 octets
```

### Méthode 2 : Compilation manuelle

**Avec NSIS en ligne de commande** :
```bash
cd installer
"C:\Installation\NSIS\makensis.exe" /V3 WallpaperAI.nsi
```

**Options de verbosité** :
- `/V0` : Silencieux
- `/V1` : Erreurs seulement
- `/V2` : Erreurs + warnings
- `/V3` : Tout (recommandé pour debug)
- `/V4` : Maximum de détails

### Workflow complet de release

**Release production** :

1. **Mettre à jour la version** dans `WallpaperAI.nsi` :
```nsis
!define APP_VERSION "1.0.4"
```

2. **Compiler l'application** :
```bash
cd app
build.bat ALL
```

3. **Générer l'installateur** :
```bash
cd ..\installer
build_installer.bat
```

4. **Tester l'installateur** :
- Installation complète sur machine propre
- Vérifier la sélection de langue
- Tester le lancement de l'application
- Vérifier les raccourcis (Bureau + Menu Démarrer)
- Tester la désinstallation

5. **(Optionnel) Signer numériquement** :
```bash
signtool sign /f certificat.pfx /p password /t http://timestamp.digicert.com WallpaperAI-Setup-Multilang-1.0.4.exe
```

6. **Distribuer** :
- Upload sur GitHub Releases
- Upload sur site web
- Calcul du hash SHA256 pour vérification :
```bash
certutil -hashfile WallpaperAI-Setup-Multilang-1.0.4.exe SHA256
```

---

## ⚡ Fonctionnalités détaillées

### 1. Détection et désinstallation automatique de l'ancienne version

**Fonction `.onInit`** :
```nsis
ReadRegStr $R0 HKLM "${UNINSTALL_KEY}" "UninstallString"
StrCmp $R0 "" done

MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
"${APP_NAME} est déjà installé. Cliquez sur OK pour désinstaller..."

uninst:
    ExecWait '$R0 _?=$INSTDIR'
```

**Comportement** :
1. Vérifie si une version est déjà installée (lecture du registre)
2. Si oui → Affiche un message demandant confirmation
3. Si OK → Exécute le désinstallateur en mode silencieux
4. Poursuit l'installation avec la nouvelle version

### 2. Arrêt automatique de l'application

**Avant l'installation** :
```nsis
nsExec::ExecToLog 'taskkill /F /IM ${APP_EXE} /T'
Sleep 1000
```

**Flags taskkill** :
- `/F` : Force la fermeture
- `/IM` : Image Name (nom de l'exécutable)
- `/T` : Termine tous les processus enfants

### 3. Installation intelligente par langue

**Principe** : Copier uniquement l'exécutable de la langue choisie

**Code** :
```nsis
${If} $LanguageCode == "FR"
    File /oname=${APP_EXE} "..\app\release\WallpaperAI_FR.exe"
${ElseIf} $LanguageCode == "EN"
    File /oname=${APP_EXE} "..\app\release\WallpaperAI_EN.exe"
# ... 5 autres langues
${Else}
    File /oname=${APP_EXE} "..\app\release\WallpaperAI_EN.exe"
${EndIf}
```

**Flag `/oname`** : Renomme le fichier lors de la copie
**Résultat** : `WallpaperAI_FR.exe` devient `WallpaperAI.exe` dans le dossier d'installation

### 4. Enregistrement Windows complet

**Clés de registre créées** :

**1. Informations de l'application** :
```
HKLM\Software\WallpaperAI\
  - InstallDir = "C:\Program Files\WallpaperAI"
  - Language = "FR"
```

**2. Panneau de configuration (Programmes et fonctionnalités)** :
```
HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\WallpaperAI\
  - DisplayName = "WallpaperAI"
  - DisplayVersion = "1.0.3"
  - Publisher = "WallpaperAI Team"
  - DisplayIcon = "C:\Program Files\WallpaperAI\WallpaperAI.exe"
  - UninstallString = "C:\Program Files\WallpaperAI\Uninstall.exe"
  - QuietUninstallString = "C:\...\Uninstall.exe /S"
  - URLInfoAbout = "https://github.com/kazuya/wallpaperai"
  - InstallLocation = "C:\Program Files\WallpaperAI"
  - NoModify = 1 (DWORD)
  - NoRepair = 1 (DWORD)
  - EstimatedSize = 78643 (DWORD, en KB)
```

**Calcul de la taille** :
```nsis
${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
IntFmt $0 "0x%08X" $0
WriteRegDWORD HKLM "${UNINSTALL_KEY}" "EstimatedSize" "$0"
```

### 5. Création des raccourcis

**Menu Démarrer** :
```
C:\Users\{User}\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\WallpaperAI\
  - WallpaperAI.lnk → WallpaperAI.exe
  - Désinstaller WallpaperAI.lnk → Uninstall.exe
```

**Bureau** :
```
C:\Users\{User}\Desktop\
  - WallpaperAI.lnk → WallpaperAI.exe
```

### 6. Désinstallation complète et propre

**Section Uninstall** :

1. **Arrêt de l'application** :
```nsis
nsExec::ExecToLog 'taskkill /F /IM ${APP_EXE} /T'
```

2. **Suppression du démarrage automatique** :
```nsis
DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "${APP_NAME}"
```

3. **Suppression des fichiers** :
- Exécutable principal
- DLLs Qt (10 fichiers)
- Assets PNG (11 fichiers)
- Désinstallateur

4. **Suppression des plugins Qt** :
- platforms/
- imageformats/
- iconengines/
- styles/
- tls/

5. **Suppression des raccourcis** :
- Menu Démarrer
- Bureau

6. **Suppression du registre** :
- `HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\WallpaperAI`
- `HKLM\Software\WallpaperAI`

**Données préservées** :
```
C:\Users\{User}\AppData\Local\WallpaperAI\
  - history.json (historique)
  - categories_cache.json (cache)
  - thumbnails/ (miniatures)
```

**Message de fin** :
```
"Note : Les paramètres utilisateur ont été conservés."
```

---

## ⚙️ Configuration

### Modification de la version

**Fichier** : `WallpaperAI.nsi`
**Ligne** : 21

```nsis
!define APP_VERSION "1.0.3"
```

**Impact** :
- Nom du fichier généré : `WallpaperAI-Setup-Multilang-1.0.3.exe`
- Version affichée dans Windows (Programmes et fonctionnalités)
- Version affichée dans l'installateur

### Modification du nom de l'éditeur

**Fichier** : `WallpaperAI.nsi`
**Ligne** : 22

```nsis
!define APP_PUBLISHER "WallpaperAI Team"
```

### Modification de l'URL du site web

**Fichier** : `WallpaperAI.nsi`
**Ligne** : 23

```nsis
!define APP_WEBSITE "https://github.com/kazuya/wallpaperai"
```

### Modification du répertoire d'installation par défaut

**Fichier** : `WallpaperAI.nsi`
**Ligne** : 33

```nsis
InstallDir "$PROGRAMFILES64\${APP_NAME}"
```

**Alternatives** :
- `$PROGRAMFILES` : Program Files (32-bit)
- `$PROGRAMFILES64` : Program Files (64-bit)
- `$LOCALAPPDATA` : AppData\Local
- `$APPDATA` : AppData\Roaming

### Modification de la compression

**Fichier** : `WallpaperAI.nsi`
**Lignes** : 42-43

```nsis
SetCompressor /SOLID lzma
SetCompressorDictSize 64
```

**Alternatives** :
- `zlib` : Rapide, ratio moyen
- `bzip2` : Bon ratio, vitesse moyenne
- `lzma` : Meilleur ratio, plus lent

**Taille du dictionnaire** : 1 à 128 MB (défaut: 8)

---

## 🎨 Personnalisation

### Ajout d'une nouvelle langue dans l'installateur

**Étape 1 : Ajouter la langue MUI** (ligne ~94) :
```nsis
!insertmacro MUI_LANGUAGE "Japanese"
```

**Étape 2 : Ajouter les strings personnalisés** (ligne ~99) :
```nsis
LangString PAGE_TITLE_LANG ${LANG_JAPANESE} "言語選択"
LangString PAGE_SUBTITLE_LANG ${LANG_JAPANESE} "アプリケーションの言語を選択"
LangString LANG_SELECT_TEXT ${LANG_JAPANESE} "アプリケーション言語:"
```

**Étape 3 : Ajouter dans le dropdown** (ligne ~183) :
```nsis
${NSD_CB_AddString} $SelectedLanguage "日本語 (Japanese)"
```

**Étape 4 : Ajouter dans la fonction d'initialisation** (ligne ~133) :
```nsis
${ElseIf} $LANGUAGE == ${LANG_JAPANESE}
    StrCpy $LanguageCode "JA"
```

**Étape 5 : Ajouter dans la sélection** (ligne ~198) :
```nsis
${ElseIf} $LanguageCode == "JA"
    ${NSD_CB_SelectString} $SelectedLanguage "日本語 (Japanese)"
```

**Étape 6 : Ajouter dans la récupération** (ligne ~217) :
```nsis
${ElseIf} $0 == "日本語 (Japanese)"
    StrCpy $LanguageCode "JA"
```

**Étape 7 : Ajouter dans l'installation** (ligne ~267) :
```nsis
${ElseIf} $LanguageCode == "JA"
    File /oname=${APP_EXE} "..\app\release\WallpaperAI_JA.exe"
```

### Ajout d'une page de licence

**Décommenter dans WallpaperAI.nsi** (après ligne 66) :
```nsis
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"
```

**Créer le fichier** `LICENSE.txt` à la racine du projet.

### Personnalisation de la page de bienvenue

**Modifier dans WallpaperAI.nsi** (après ligne 65) :
```nsis
!define MUI_WELCOMEPAGE_TITLE "Bienvenue dans l'installation de ${APP_NAME}"
!define MUI_WELCOMEPAGE_TEXT "Texte personnalisé ici..."
```

### Modification de l'icône

**Fichier** : `WallpaperAI.nsi`
**Ligne** : 25

```nsis
!define APP_ICON "..\app\assets\icon.ico"
```

**Format requis** : `.ico` (pas `.png`)
**Résolutions recommandées** : 16x16, 32x32, 48x48, 256x256

---

## 🔧 Dépannage

### Erreur : "NSIS n'est pas reconnu"

**Cause** : NSIS n'est pas dans le PATH système

**Solutions** :
1. Utiliser le chemin complet :
```bash
"C:\Installation\NSIS\makensis.exe" WallpaperAI.nsi
```

2. Ajouter au PATH :
- Panneau de configuration → Système → Variables d'environnement
- PATH → Modifier → Ajouter `C:\Installation\NSIS`
- Redémarrer le terminal

### Erreur : "File not found: WallpaperAI_FR.exe"

**Cause** : Les exécutables multilingues n'ont pas été compilés

**Solution** :
```bash
cd app
build.bat ALL
```

### Erreur : "Processing script file (ACP)" au lieu de "(UTF8)"

**Cause** : Le fichier .nsi n'a pas le BOM UTF-8

**Solution** :
```powershell
cd installer
.\convert_to_utf8_bom.ps1 WallpaperAI.nsi
```

**Vérification** :
```bash
makensis /V2 WallpaperAI.nsi | findstr "Processing"
# Doit afficher : Processing script file: "WallpaperAI.nsi" (UTF8)
```

### Erreur : "Invalid command: Unicode"

**Cause** : Version de NSIS trop ancienne (< 3.0)

**Solution** : Installer NSIS 3.11+ depuis https://nsis.sourceforge.io/Download

### L'installateur ne démarre pas

**Causes possibles** :
1. **Antivirus bloque l'exécutable non signé**
   - Désactiver temporairement l'antivirus
   - Ou signer numériquement l'installateur

2. **Droits administrateur insuffisants**
   - Clic droit → Exécuter en tant qu'administrateur

3. **Fichier corrompu**
   - Vérifier le hash SHA256
   - Recompiler l'installateur

### Caractères accentués mal affichés

**Cause** : Encodage UTF-8 sans BOM

**Solution** : Voir "Processing script file (ACP)" ci-dessus

---

## 📤 Distribution

### Génération du hash pour vérification

**SHA256** (recommandé) :
```bash
certutil -hashfile WallpaperAI-Setup-Multilang-1.0.3.exe SHA256
```

**SHA1** :
```bash
certutil -hashfile WallpaperAI-Setup-Multilang-1.0.3.exe SHA1
```

### Signature numérique (production)

**Prérequis** : Certificat de signature de code (Code Signing Certificate)

**Commande** :
```bash
signtool sign ^
  /f certificat.pfx ^
  /p motdepasse ^
  /t http://timestamp.digicert.com ^
  /fd SHA256 ^
  /d "WallpaperAI" ^
  /du "https://github.com/kazuya/wallpaperai" ^
  WallpaperAI-Setup-Multilang-1.0.3.exe
```

**Options** :
- `/f` : Fichier certificat (.pfx ou .p12)
- `/p` : Mot de passe du certificat
- `/t` : Serveur de timestamp (horodatage)
- `/fd` : Algorithme de hash (SHA256 recommandé)
- `/d` : Description de l'application
- `/du` : URL de l'application

**Vérification** :
- Clic droit sur .exe → Propriétés → Signatures numériques

**Avantages** :
- Pas de warning Windows SmartScreen
- Confiance utilisateur
- Intégrité vérifiable

### Upload sur GitHub Releases

**Workflow** :
1. Créer un tag de version :
```bash
git tag v1.0.3
git push origin v1.0.3
```

2. Créer une release sur GitHub
3. Uploader `WallpaperAI-Setup-Multilang-1.0.3.exe`
4. Ajouter le hash SHA256 dans la description

**Description recommandée** :
```markdown
## WallpaperAI v1.0.3

### Installateur
- `WallpaperAI-Setup-Multilang-1.0.3.exe` (20 MB)
- SHA256: `abc123...`

### Langues supportées
🇫🇷 Français | 🇬🇧 English | 🇪🇸 Español | 🇵🇹 Português | 🇮🇹 Italiano | 🇩🇪 Deutsch | 🇷🇺 Русский

### Installation
1. Télécharger l'installateur
2. Exécuter en tant qu'administrateur
3. Choisir la langue
4. Suivre les instructions
```

---

## 📊 Statistiques

**Fichiers inclus dans l'installateur** :

| Type | Nombre | Taille totale |
|------|--------|---------------|
| Exécutables | 1 (parmi 7) | ~13 MB |
| DLLs Qt | 10 | ~50 MB |
| Plugins Qt | ~20 | ~10 MB |
| Assets PNG | 11 | ~500 KB |
| **Total non compressé** | **~42 fichiers** | **~74 MB** |
| **Total compressé (LZMA)** | - | **~20 MB** |

**Compression LZMA** :
- Taux de compression : **13.5%**
- Gain d'espace : **~54 MB économisés**

**Performance** :
- Installation : < 30 secondes
- Désinstallation : < 10 secondes
- Décompression LZMA : < 5 secondes

---

## 🔗 Liens utiles

- **NSIS Documentation** : https://nsis.sourceforge.io/Docs/
- **MUI2 Reference** : https://nsis.sourceforge.io/Docs/Modern%20UI%202/Readme.html
- **NSIS Forums** : https://forums.winamp.com/forumdisplay.php?f=65
- **Application WallpaperAI** : Voir `app/docs/README.md`
- **API Backend** : Voir `api/docs/README.md`

---

## 📄 Licence

L'installateur est créé avec **NSIS** (Nullsoft Scriptable Install System), distribué sous licence **zlib/libpng**.

**Texte de licence** : https://nsis.sourceforge.io/License

---

**Version** : 1.0.3
**Dernière mise à jour** : Octobre 2025
**Créé avec** : [Claude Code](https://claude.com/claude-code)
