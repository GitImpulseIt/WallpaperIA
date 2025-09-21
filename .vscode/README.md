# Configuration VSCode pour débogage WallpaperIA

## 🔧 Prérequis

- **VSCode** avec les extensions suivantes :
  - `C/C++` (Microsoft)
  - `C/C++ Extension Pack` (Microsoft)
  - `CMake Tools` (Microsoft)

- **GDB** installé avec MinGW (déjà inclus dans l'installation Qt)

## 🚀 Utilisation du débogage

### 1. Compilation Debug
```bash
cd app
./build_debug.bat
```
Cette commande compile l'application avec les symboles de débogage (`-g -O0`).

### 2. Démarrer le débogage dans VSCode

#### Option A : Débogage direct
1. Ouvrez le fichier source que vous voulez déboguer (ex: `src/core/wallpaper_builder.cpp`)
2. Placez des **points d'arrêt** en cliquant dans la marge gauche
3. Appuyez sur **F5** ou utilisez le menu `Run > Start Debugging`
4. Sélectionnez la configuration **"Debug WallpaperIA"**

#### Option B : Attachement à un processus
1. Lancez WallpaperIA normalement
2. Dans VSCode, utilisez **Ctrl+Shift+P** → `Debug: Attach to Process`
3. Sélectionnez le processus `WallpaperIA.exe`

### 3. Contrôles de débogage

- **F5** : Continue
- **F10** : Step Over (enjamber)
- **F11** : Step Into (entrer dans)
- **Shift+F11** : Step Out (sortir de)
- **F9** : Toggle Breakpoint

## 📁 Fichiers de configuration

- **`.vscode/launch.json`** : Configuration de débogage
- **`.vscode/tasks.json`** : Tâches de build (Debug/Release/Clean)
- **`.vscode/c_cpp_properties.json`** : Configuration IntelliSense C++
- **`.vscode/settings.json`** : Paramètres VSCode pour le projet

## 🎯 Points d'entrée recommandés

Pour déboguer la logique multi-écrans, placez des breakpoints dans :

- **`src/core/wallpaper_builder.cpp`** :
  - `createMultiScreenWallpaper()` - Point d'entrée principal
  - `calculateVirtualDesktopBounds()` - Calcul bureau virtuel
  - `generateScreenMappings()` - Génération mappings
  - `createCompositeImageFromMappings()` - Création image composite

- **`main.cpp`** :
  - `setMultipleWallpapers()` - Déclenchement dans ModernWindow
  - `onChangeNowClicked()` - Action utilisateur

## 🔍 Variables à surveiller

Dans la fenêtre **Variables** de VSCode, surveillez :
- `virtualDesktop` (QRect) - Dimensions bureau virtuel
- `mappings` (QList<ScreenMapping>) - Mappings écran/image
- `composite` (QPixmap) - Image composite générée
- `imagePaths` (QMap<int, QString>) - Chemins images par écran

## 📝 Conseils de débogage

1. **Console de débogage** : Les `qDebug()` s'affichent dans la console
2. **Breakpoints conditionnels** : Clic droit sur un breakpoint pour ajouter une condition
3. **Watch expressions** : Ajoutez des expressions personnalisées à surveiller
4. **Call Stack** : Vérifiez la pile d'appels pour comprendre le flux d'exécution

## ⚡ Raccourcis utiles

- **Ctrl+Shift+P** : Palette de commandes
- **Ctrl+Shift+`** : Terminal intégré
- **Ctrl+K Ctrl+S** : Raccourcis clavier
- **F12** : Aller à la définition
- **Alt+F12** : Peek definition

## 🔧 Dépannage

### Problème : "GDB exited unexpectedly"

1. **Vérifiez la compilation debug :**
   ```bash
   cd app && ./build_debug.bat
   ```

2. **Testez GDB manuellement :**
   ```bash
   cd app && ./test_debug.bat
   ```

3. **Solutions alternatives :**
   - Utilisez la configuration **"Debug WallpaperIA (No Task)"** (sans pré-tâche)
   - Essayez **"Debug WallpaperIA (Attach to Process)"** en lançant d'abord l'app
   - Utilisez `launch_simple.json` comme configuration de base

### Problème : Application se ferme immédiatement

1. **Ajoutez stopAtEntry: true** dans launch.json
2. **Placez un breakpoint au début du main()** avant de démarrer
3. **Vérifiez les dépendances Qt** dans le répertoire build/

### Problème : Breakpoints non reconnus

1. **Recompilez en mode debug** avec `-g -O0`
2. **Vérifiez que les fichiers .cpp/.h sont synchronisés**
3. **Redémarrez VSCode** après changement de configuration

### Test manuel GDB

Si VSCode pose problème, testez directement avec GDB :

```bash
cd app/build
gdb WallpaperIA.exe
(gdb) break main
(gdb) run
(gdb) step
(gdb) continue
(gdb) quit
```