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
- **Pas de commentaires** sauf si explicitement demandé
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
- Ajout de commentaires non demandés
- Désalignements dans l'interface
- Messages de commit sans format spécifié

---

## 🔥 PROBLÈME MULTI-ÉCRANS EN COURS

### 📋 Contexte du problème
- **Disposition des écrans** : Écran 3 (principal Windows) et écrans 2 en haut, écran 1 en bas (plus grand)
- **Problème actuel** : L'écran 1 affiche le fond d'écran en tuiles (4 répétitions) au lieu d'une image complète
- **Écrans 2 et 3** : Fonctionnent correctement avec images complètes
- **Configuration Windows** : Écran 3 = principal (pas l'écran 1)

### 🔧 Solutions tentées
1. **Mode Span** : WallpaperStyle="22", TileWallpaper="0" - amélioration partielle
2. **Mode Tile** : WallpaperStyle="0", TileWallpaper="1" - comme DualMonitorTools
3. **Système de wrapping** : Implémentation complète du wrapping DualMonitorTools pour gérer les coordonnées négatives
4. **ScreenMapping sophistiqué** : Structure de mapping avec calculs précis des positions
5. **Debug détaillé** : Logs complets des dimensions, mappings et wrapping

### 🎯 Système actuel implémenté
- **ScreenMapping** : Structure sophistiquée pour mapper chaque écran
- **calculateVirtualDesktopBounds()** : Calcul précis du bureau virtuel avec debug
- **generateScreenMappings()** : Création des mappings par écran avec logs
- **createCompositeImageFromMappings()** : Génération d'image composite haute qualité
- **wrapCoordinatesForWindows()** : Système de wrapping complet (quadrants A,B,C,D)
- **Debug complet** : Logs détaillés de tous les calculs et transformations

### 🔍 Investigations nécessaires
1. **Vérifier la détection Qt vs Windows** de l'écran principal
2. **Analyser les logs de debug** lors des tests multi-écrans
3. **Comprendre pourquoi l'écran 1** fait du tiling malgré le wrapping
4. **Tester différents modes** : Span, Tile, Fill selon les résultats
5. **Identifier la vraie cause** : Coordonnées, tailles, ou positionnement

### 📝 Notes importantes
- **Écran principal** : Écran 3 dans Windows, mais Qt peut détecter différemment
- **Disposition complexe** : Écrans avec coordonnées négatives possibles
- **DualMonitorTools** : Référence fonctionnelle pour la technique de wrapping
- **Tests** : Lancer `./WallpaperIA.exe` en console pour voir les logs debug
- **Image composite** : Sauvée dans `/temp/WallpaperIA/composite_wallpaper.bmp`

### 🚀 Prochaines étapes
1. Analyser les logs debug détaillés
2. Vérifier la correspondance écran Qt index vs écran Windows
3. Ajuster le wrapping selon la vraie disposition détectée
4. Tester les modes alternatifs si nécessaire
5. Corriger définitivement le problème de tiling sur l'écran 1