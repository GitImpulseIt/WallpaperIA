<?php
/**
 * Script pour fusionner tous les fichiers de prompts en un seul fichier
 * Usage: php merge_prompts.php
 */

$promptsDir = __DIR__;
$outputFile = $promptsDir . '/all_prompts.txt';

// Liste des fichiers de prompts à fusionner
$promptFiles = [
    'MYSTERIOUS_GOTHIC.txt',
    'NATURAL_LANDSCAPES.txt',
    'MOUNTAINS_AND_PEAKS.txt',
    'URBAN_ARCHITECTURE.txt',
    'AUTUMN_SEASONS.txt',
    'TROPICAL_PARADISE.txt',
    'MINIMALIST_ABSTRACT.txt',
    'OCEAN_AND_MARINE.txt',
    'DESERT_LANDSCAPES.txt',
    'CYBERPUNK_FUTURISTIC.txt',
    'VINTAGE_RETRO.txt',
    'WINTER_WONDERLAND.txt',
    'SPACE_AND_COSMOS.txt',
    'FANTASY_WORLDS.txt',
    'GEOMETRIC_PATTERNS.txt',
    'MANGA_AND_ANIME.txt',
    'RETRO_CARTOONS_70s_2000s.txt',
    'MAGICAL_FANTASY.txt',
    'STEAMPUNK.txt',
    'ALIEN_PLANETS.txt',
    'INFERNO_AND_FIRE.txt',
    'CUTE_PETS.txt',
    'LANDSCAPE_WITH_GIRL.txt'
];

echo "Fusion des fichiers de prompts...\n";
echo "================================\n\n";

$totalPrompts = 0;
$mergedContent = '';

foreach ($promptFiles as $file) {
    $filePath = $promptsDir . '/' . $file;

    if (!file_exists($filePath)) {
        echo "ERREUR: Fichier non trouvé: $file\n";
        continue;
    }

    echo "Traitement de $file... ";

    $content = file_get_contents($filePath);

    // Compter le nombre de prompts (nombre de "name:" dans le fichier)
    $promptCount = substr_count($content, 'name:');
    $totalPrompts += $promptCount;

    echo "$promptCount prompts\n";

    // Ajouter le contenu au fichier fusionné
    $mergedContent .= $content;
}

echo "\n================================\n";
echo "Total: $totalPrompts prompts fusionnés\n";
echo "Écriture du fichier de sortie: all_prompts.txt\n";

// Écrire le fichier fusionné
file_put_contents($outputFile, $mergedContent);

echo "✓ Fusion terminée avec succès!\n";
echo "Fichier créé: $outputFile\n";
