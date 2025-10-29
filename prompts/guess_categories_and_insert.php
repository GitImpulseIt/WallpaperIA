<?php
/**
 * Script pour deviner les catégories des fichiers manquants et créer un patch d'insertion
 * Usage: php guess_categories_and_insert.php <sync_diff_csv> <cleaned_csv> <output_patch> <date>
 */

if ($argc < 5) {
    echo "Usage: php guess_categories_and_insert.php <sync_diff_csv> <cleaned_csv> <output_patch> <date>\n";
    echo "\n";
    echo "Exemple: php guess_categories_and_insert.php sync_diff_backup.csv wallpapers_cleaned.csv insert_missing.csv 28/10/2025\n";
    exit(1);
}

$syncDiffCsv = $argv[1];
$cleanedCsv = $argv[2];
$outputPatch = $argv[3];
$targetDate = $argv[4];

// Vérifier que les fichiers existent
if (!file_exists($syncDiffCsv)) {
    echo "ERREUR: Le fichier '$syncDiffCsv' n'existe pas.\n";
    exit(1);
}

if (!file_exists($cleanedCsv)) {
    echo "ERREUR: Le fichier '$cleanedCsv' n'existe pas.\n";
    exit(1);
}

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Détection de catégories et génération de patch         ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

// Définir les mots-clés pour chaque catégorie
$categoryKeywords = [
    'MOUNTAINS & PEAKS' => ['mountain', 'alpine', 'peak', 'summit', 'himalayan'],
    'NATURAL LANDSCAPES' => ['forest', 'meadow', 'stream', 'valley', 'landscape', 'nature', 'countryside', 'spring'],
    'OCEAN & MARINE' => ['ocean', 'sea', 'marine', 'turtle', 'underwater', 'beach', 'coral', 'whale', 'diving', 'lagoon'],
    'WINTER WONDERLAND' => ['winter', 'snow', 'skiing', 'ski', 'snowy', 'ice', 'frozen'],
    'TROPICAL PARADISE' => ['tropical', 'palm', 'jungle', 'waterfall', 'paradise', 'exotic'],
    'DESERT LANDSCAPES' => ['desert', 'canyon', 'dune', 'arid', 'bushes', 'cactus'],
    'AUTUMN SEASONS' => ['autumn', 'fall', 'harvest', 'orchard', 'leaf', 'foliage'],
    'SPACE & COSMOS' => ['space', 'cosmos', 'nebula', 'galaxy', 'star', 'planet', 'aurora', 'alien'],
    'CYBERPUNK FUTURISTIC' => ['futuristic', 'cyber', 'neon', 'hologram', 'circuit', 'data', 'hexagonal', 'network'],
    'FANTASY WORLDS' => ['fantasy', 'dragon', 'castle', 'floating', 'enchanted', 'magic', 'wizard'],
    'URBAN ARCHITECTURE' => ['urban', 'city', 'architecture', 'building', 'street', 'skyline', 'cathedral', 'colonial'],
    'VINTAGE RETRO' => ['vintage', 'retro', 'hippie', 'classic', 'old', 'nostalgic', 'van'],
    'MYSTERIOUS GOTHIC' => ['gothic', 'mysterious', 'dark', 'gargoyle', 'cathedral', 'ruins'],
    'MINIMALIST ABSTRACT' => ['minimalist', 'abstract', 'geometric', 'pattern', 'fractal', 'liquid', 'metal', 'surface', 'spiral'],
    'GEOMETRIC PATTERNS' => ['geometric', 'pattern', 'tessellation', 'mosaic', 'symmetry'],
    'STEAMPUNK' => ['steampunk', 'clockwork', 'mechanical', 'brass', 'gear'],
    'MANGA & ANIME' => ['anime', 'manga', 'japanese'],
    'RETRO CARTOONS 70s-2000s' => ['cartoon', 'toons', 'animated'],
    'MAGICAL FANTASY' => ['magical', 'spell', 'wizard', 'enchanted', 'mystical'],
    'ALIEN PLANETS' => ['alien', 'planet', 'extraterrestrial', 'otherworldly'],
    'INFERNO & FIRE' => ['fire', 'flame', 'inferno', 'burning', 'phoenix'],
    'CUTE PETS' => ['pet', 'dog', 'cat', 'puppy', 'kitten', 'corgi'],
    'RELAXATION' => ['zen', 'calm', 'peaceful', 'meditation', 'tranquil'],
];

// Fonction pour deviner la catégorie d'un fichier
function guessCategory($filename, $categoryKeywords) {
    $filenameLower = strtolower($filename);
    $scores = [];

    foreach ($categoryKeywords as $category => $keywords) {
        $score = 0;
        foreach ($keywords as $keyword) {
            if (strpos($filenameLower, $keyword) !== false) {
                $score++;
            }
        }
        if ($score > 0) {
            $scores[$category] = $score;
        }
    }

    if (empty($scores)) {
        return null;
    }

    // Retourner la catégorie avec le meilleur score
    arsort($scores);
    return array_key_first($scores);
}

// Lire les fichiers manquants depuis sync_diff_backup.csv
$missingFiles = [];
$syncHandle = fopen($syncDiffCsv, 'r');
fgetcsv($syncHandle, 0, ',', '"', ''); // Skip header

while (($row = fgetcsv($syncHandle, 0, ',', '"', '')) !== false) {
    $type = trim($row[0], '"');
    if ($type === 'missing_in_csv') {
        $filename = trim($row[1], '"');
        $missingFiles[] = $filename;
    }
}
fclose($syncHandle);

echo "Fichiers manquants dans le CSV: " . count($missingFiles) . "\n\n";

// Obtenir le dernier ID utilisé dans le CSV nettoyé
$lastId = 0;
$cleanedHandle = fopen($cleanedCsv, 'r');
fgetcsv($cleanedHandle, 0, ',', '"', ''); // Skip header

while (($row = fgetcsv($cleanedHandle, 0, ',', '"', '')) !== false) {
    if (isset($row[3])) {
        $id = intval(trim($row[3], '"'));
        if ($id > $lastId) {
            $lastId = $id;
        }
    }
}
fclose($cleanedHandle);

echo "Dernier ID dans le CSV nettoyé: $lastId\n";
echo "Premier nouvel ID: " . ($lastId + 1) . "\n\n";

// Deviner les catégories et créer le patch
$patchEntries = [];
$currentId = $lastId + 1;
$categorized = 0;
$uncategorized = 0;

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Analyse des fichiers                                    ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

foreach ($missingFiles as $filename) {
    $guessedCategory = guessCategory($filename, $categoryKeywords);

    if ($guessedCategory) {
        $patchEntries[] = [
            'category' => $guessedCategory,
            'filename' => $filename,
            'date' => $targetDate,
            'id' => $currentId
        ];

        printf("✓ %-50s → %-30s [ID: %d]\n", $filename, $guessedCategory, $currentId);
        $categorized++;
        $currentId++;
    } else {
        printf("⚠ %-50s → IMPOSSIBLE À CATÉGORISER\n", $filename);
        $uncategorized++;
    }
}

echo "\n";
echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Résultats                                               ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "✓ Fichiers catégorisés: $categorized\n";
echo "⚠ Fichiers non catégorisés: $uncategorized\n\n";

// Écrire le patch CSV
if (!empty($patchEntries)) {
    $patchHandle = fopen($outputPatch, 'w');

    // Écrire l'en-tête
    fputcsv($patchHandle, ['category', 'filename', 'date', 'id'], ',', '"', '');

    // Écrire les entrées
    foreach ($patchEntries as $entry) {
        fputcsv($patchHandle, [
            $entry['category'],
            $entry['filename'],
            $entry['date'],
            $entry['id']
        ], ',', '"', '');
    }

    fclose($patchHandle);

    echo "✓ Fichier patch créé: $outputPatch\n";
    echo "  - Entrées: " . count($patchEntries) . "\n";
    echo "  - IDs: " . ($lastId + 1) . " à " . ($currentId - 1) . "\n";
    echo "  - Date: $targetDate\n\n";

    // Afficher la répartition par catégorie
    echo "Répartition par catégorie:\n";
    echo str_repeat("-", 80) . "\n";

    $categoryCount = [];
    foreach ($patchEntries as $entry) {
        $cat = $entry['category'];
        if (!isset($categoryCount[$cat])) {
            $categoryCount[$cat] = 0;
        }
        $categoryCount[$cat]++;
    }

    arsort($categoryCount);
    foreach ($categoryCount as $category => $count) {
        printf("%-40s : %2d fichier(s)\n", $category, $count);
    }

    echo "\n✓ Patch généré avec succès!\n";
    echo "  Vous pouvez maintenant l'appliquer avec apply_patch.php\n\n";
} else {
    echo "⚠ Aucun fichier à ajouter - patch non créé.\n\n";
}
