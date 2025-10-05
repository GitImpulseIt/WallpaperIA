# Consignes Claude Code - WallpaperAI

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
- `app/src/` : Modules extraits (config/, api/, utils/)
- `app/build.bat` : Script de compilation Qt/MinGW
- `app/assets/` : Images et ressources (PNG)
- `app/release/` : Build final avec dépendances
- `api/` : Backend PHP REST pour wallpapers

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

### 📝 Notes techniques importantes
- **Coordonnées logiques Windows** : Conservées telles quelles pour le positionnement
- **Résolutions natives écrans** : Utilisées uniquement pour les tailles des rectangles
- **Plus de tiling** : Problème résolu sur les écrans haute résolution avec scaling

---

## 🆕 FONCTIONNALITÉS RÉCENTES IMPLÉMENTÉES

### ⏱️ Système de déclenchement automatique (Commits de7c881, 04a70c8)
- **Signal countdownExpired()** : Émis automatiquement quand le compte à rebours expire
- **Connexion automatique** : Déclenche `onChangeNowClicked()` pour changer le fond d'écran
- **Widget élargi** : CountdownWidget agrandi (280x200) avec cercle recentré dynamiquement
- **Mode "Au démarrage"** : Encadré informatif bleu (#2196F3) avec icône info.png
- **Mode "Changement manuel uniquement"** : Encadré informatif identique au mode démarrage

### 🔄 Gestion des fréquences avec logique de cohérence
- **Option "Changement manuel uniquement"** : Première option pour désactiver les changements automatiques
- **Logique de dépendance** : "Au démarrage" disponible seulement si "Démarrer avec Windows" activé
- **Basculement automatique** : Passage vers "Changement manuel uniquement" en cas d'incohérence
- **Interface cohérente** : Options grisées visuellement, indices corrigés (index 8 pour "Autre")
- **Mode m_isNeverMode** : Nouveau mode pour gérer l'affichage sans timer

### 🖥️ Démarrage automatique avec Windows (✅ Fonctionnel)
- **Gestion registre Windows** : Fonctions `addToWindowsStartup()`, `removeFromWindowsStartup()`, `isInWindowsStartup()`
- **Argument `--startup`** : Détection du démarrage automatique via ligne de commande
- **System tray intelligent** : Démarrage direct dans le tray quand lancé au boot
- **Toggle fonctionnel** : Activation/désactivation du démarrage Windows via interface
- **Vérification automatique** : S'assure que l'option est correctement appliquée au lancement
- **Changement au démarrage** : Si "Au démarrage" est sélectionné, déclenche automatiquement le changement de fond d'écran avec délai de 2s

### 🎨 Améliorations visuelles et UX
- **Encadrés informatifs** : Style uniforme bleu #2196F3 avec icône info.png et texte non-gras
- **CountdownWidget responsive** : Adaptation automatique pour cadres élargis
- **Cohérence des couleurs** : Respect strict du thème (#2196F3, #d14836, #8b4513)
- **Fallbacks visuels** : Emojis de secours (🔄) si images non chargées

### 🎯 Système intelligent de sélection de wallpapers
- **Pondération par étoiles** : Sélection probabiliste des catégories selon leur notation
- **Fallback en cascade** : Date actuelle → 7 jours précédents → exclusion catégorie → historique local
- **Évitement doublons** : Vérification historique avant sélection, filtrage doublons dans carrousel
- **Exclusion temporaire** : Catégories épuisées exclues pour la session en cours

### 🗂️ Architecture modulaire
- **StartupManager** : Gestion du démarrage Windows (registre)
- **PathHelper** : Gestion centralisée des chemins (AppConfigLocation)
- **DateHelper** : Utilitaires de date pour l'API
- **Cache unifié** : `/thumbnails` partagé (catégories + historique), limite 100 fichiers

## 🔧 API REST (Backend PHP)
- **Endpoint `/categories`** : Retourne catégories avec miniature par défaut (wallpaper le plus récent)
- **Endpoint `/wallpapers`** : Paramètres obligatoires `category` + `date` (DD/MM/YYYY)
- **Endpoint `/mini/{filename}`** : Miniatures optimisées (204x115px)
- **Optimisation** : Réduction drastique des appels API grâce aux thumbnails dans `/categories`
- **Fallback intelligent** : Remontée automatique jusqu'à 7 jours en arrière si date vide