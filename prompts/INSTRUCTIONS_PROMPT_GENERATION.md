# Instructions pour la génération de prompts thématiques - Flux.1 Dev

## Format de fichier requis

Le format utilisé par ComfyUI inspire pack suit cette structure :
- `positive:` contient le prompt CLIP_L (court et direct)
- `negative:` contient le prompt T5 (détaillé et descriptif)
- `----` ou `-----------------` sépare chaque prompt

## Commande à exécuter

Quand l'utilisateur demande : "suis les instructions de ce fichier et produit X thèmes avec Y images par thème"

1. **Calculer le nombre total de prompts** : X thèmes × Y images = nombre total
2. **Choisir X thèmes variés** pour les wallpapers (paysages, urbains, abstraits, nature, etc.)
3. **Créer un nouveau fichier** dans le dossier `prompts/` avec un nom descriptif
4. **Générer les prompts** en respectant le format exact

## Structure du fichier de sortie

```
positive:[CLIP_L prompt court - 1-2 phrases descriptives]
negative:[T5 prompt détaillé - paragraphe riche et descriptif avec détails techniques, atmosphère, couleurs, composition]
----
positive:[CLIP_L prompt court pour image 2]
negative:[T5 prompt détaillé pour image 2]
----
[continuer pour tous les prompts...]
```

## Bonnes pratiques pour les prompts

### CLIP_L (positive) :
- Court et direct (20-40 mots)
- Mots-clés essentiels
- Style photographique/artistique
- Éléments principaux de la scène

### T5 (negative) :
- Très détaillé (100-200 mots)
- Description atmosphérique riche
- Détails techniques (lumière, composition, couleurs)
- Éléments émotionnels et sensoriels
- Qualité cinématographique

## Exemple de thèmes possibles
- Paysages naturels (montagnes, forêts, océans)
- Urbain (skylines, rues, architecture)
- Abstrait (formes, couleurs, patterns)
- Science-fiction (futuriste, space, cyber)
- Fantasy (magique, créatures, mondes fantastiques)
- Saisons (automne, hiver, printemps, été)
- Minimaliste (épuré, simple, zen)

## Notes importantes
- Varier les styles au sein d'un même thème
- Assurer une diversité visuelle
- Maintenir une cohérence thématique
- Utiliser des termes techniques photographiques
- Éviter les répétitions entre prompts