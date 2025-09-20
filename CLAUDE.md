# Consignes Claude Code - WallpaperIA

## 🔧 Compilation obligatoire
- **TOUJOURS compiler après chaque modification** de code
- **Commande de compilation** : `cd app && ./build.bat`
- Vérifier que la compilation réussit avant de continuer
- En cas d'erreur de compilation, corriger immédiatement

## 📝 Gestion des commits
- **Commit automatique** quand l'utilisateur valide les changements
- **JAMAIS de push** - commits locaux uniquement
- **Format du message** : Description claire + `🤖 Generated with [Claude Code](https://claude.ai/code)`
- **Ne pas ajouter** de ligne "Co-Authored-By" (redondant)

## 🎨 Style et architecture du projet
- **Framework** : Qt 6 en C++ avec interfaces modernes
- **Esthétique** : Design sombre avec couleurs #2196F3 (bleu), #d14836 (rouge-orange), #8b4513 (marron)
- **Composants personnalisés** : ToggleSwitch, CountdownWidget, ScreenSelector avec esthétique spécifique
- **Alignements** : Justification parfaite, espacements cohérents (10px standard)

## 🖥️ Fonctionnalités spécifiques
- **Sélecteur d'écran** : S'affiche uniquement si "Image différente sur chaque écran" activé ET plusieurs écrans détectés
- **Interface multi-moniteur** : Détection automatique des écrans connectés
- **Sauvegarde des paramètres** : Utilisation de QSettings pour persistance
- **System tray** : Application reste en arrière-plan

## 📐 Conventions de code
- **Commentaires** : Autorisés dans le code principal, évités uniquement dans screenmap.cpp (outil de debug)
- **Noms en français** pour l'interface utilisateur
- **Espacement** : 10px entre éléments, alignements justifiés
- **Largeurs fixes** : Containers de 280px pour alignement, boutons adaptés au contenu

## 🔄 Workflow de développement
1. Modifier le code selon la demande
2. **Compiler immédiatement** avec `cd app && ./build.bat`
3. Corriger les erreurs de compilation si nécessaire
4. Tester les fonctionnalités
5. Si validé par l'utilisateur → **commit sans push**

## 🎯 Priorités de qualité
- **Compilation réussie** = priorité absolue
- **Esthétique cohérente** avec le design existant
- **Alignements parfaits** et espacements uniformes
- **Fonctionnalité multi-écran** robuste
- **Performance** et réactivité de l'interface

## 📁 Structure du projet
- `app/main.cpp` : Fichier principal avec toutes les classes
- `app/build.bat` : Script de compilation Qt/MinGW
- `app/assets/` : Images et ressources (PNG)
- `app/release/` : Build final avec dépendances

## 🚫 Éviter
- Modifications sans compilation préalable
- Commits avec push automatique
- Désalignements dans l'interface
- Messages de commit sans format spécifié

---

## ✅ SYSTÈME MULTI-ÉCRANS RÉSOLU

### 🎯 Solution finale implémentée
- **Problème résolu** : Correction définitive du tiling sur l'écran haute résolution (commit a5c62da)
- **Technique** : Distinction entre coordonnées logiques Windows et résolutions natives écrans
- **Méthode** : Utilisation des résolutions natives au lieu des tailles logiques Qt

### 🔧 Système actuel fonctionnel
- **ScreenMapping** : Structure sophistiquée pour mapper chaque écran
- **calculateVirtualDesktopBounds()** : Utilise maintenant les résolutions natives
- **generateScreenMappings()** : Coordonnées conservées, tailles en résolution native
- **createCompositeImageFromMappings()** : Génération d'image composite haute qualité
- **wrapCoordinatesForWindows()** : Système de wrapping complet (quadrants A,B,C,D)
- **Suppression du scaling** : Plus de scaling incorrect des coordonnées

### 🧪 Outils de debug disponibles
- **screenmap.cpp** : Outil de test pour vérifier le mapping des écrans
- **compile_screenmap.bat** : Script de compilation Visual Studio pour les tests
- **Debug détaillé** : Logs complets des calculs avec résolutions réelles
- **Image composite** : Sauvée dans `/temp/WallpaperIA/composite_wallpaper.bmp`

### 📝 Notes techniques importantes
- **Coordonnées logiques Windows** : Conservées telles quelles pour le positionnement
- **Résolutions natives écrans** : Utilisées uniquement pour les tailles des rectangles
- **Plus de tiling** : Problème résolu sur les écrans haute résolution avec scaling
- **Tests** : Utiliser `./screenmap.exe` pour diagnostiquer le mapping si besoin