# Thumbnail Quality Tester

Cet outil permet de tester différentes techniques de redimensionnement d'images pour trouver la meilleure qualité possible pour les miniatures.

## Usage

```bash
php thumbnail_tester.php <image_input> <scale_ratio>
```

### Paramètres

- `image_input` : Chemin vers l'image source (PNG, JPG, GIF, WebP)
- `scale_ratio` : Pourcentage de la taille originale (ex: 25 pour 25% de la taille)

### Exemples

```bash
# Réduire une image à 25% de sa taille originale
php thumbnail_tester.php ../api/test-image.png 25

# Réduire une image à 50% de sa taille originale
php thumbnail_tester.php mon_image.jpg 50
```

## Techniques testées

1. **Basic Resampled** - Méthode actuelle de l'API (imagecopyresampled basique)
2. **Enhanced + Edge Enhance** - Amélioration avec filtre de netteté
3. **Two-Step Progressive** - Redimensionnement en deux étapes pour les réductions importantes
4. **PHP imagescale (Bicubic)** - Utilisation de la fonction imagescale avec interpolation bicubique
5. **Pre-Sharpened Source** - Pré-netteté de l'image source avant redimensionnement
6. **Maximum JPEG Quality** - JPEG à 100% de qualité
7. **PNG Lossless** - Format PNG sans perte
8. **WebP Format** - Format WebP moderne

## Sortie

L'outil génère :
- Un dossier `output_[nom_image]/` contenant toutes les variantes
- Un fichier `comparison.html` pour comparer visuellement les résultats
- Affichage des tailles de fichiers pour chaque méthode

## Recommandations

1. Comparez visuellement la qualité dans `comparison.html`
2. Vérifiez les tailles de fichiers générées
3. Choisissez le meilleur compromis qualité/taille pour votre usage
4. Intégrez la meilleure technique dans l'API principale

## Configuration requise

- PHP 5.4+ (certaines techniques nécessitent PHP 5.5+)
- Extensions PHP : GD, (optionnel: WebP support)