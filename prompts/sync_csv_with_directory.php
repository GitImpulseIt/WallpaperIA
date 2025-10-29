<?php
/**
 * Script pour créer un CSV contenant UNIQUEMENT les fichiers présents dans le répertoire
 * Usage: php sync_csv_with_directory.php <wallpapers_dir> <csv_source> <output_csv>
 */

if ($argc < 4) {
    echo "Usage: php sync_csv_with_directory.php <wallpapers_dir> <csv_source> <output_csv>\n";
    echo "\n";
    echo "Exemple: php sync_csv_with_directory.php C:\\Installation\\ComfyUI\\wallpapers ../api/wallpapers_merged.csv ../api/wallpapers_synced.csv\n";
    exit(1);
}

$wallpapersDir = $argv[1];
$csvSource = $argv[2];
$outputCsv = $argv[3];

// Vérifier que les fichiers/répertoires existent
if (!is_dir($wallpapersDir)) {
    echo "ERREUR: Le répertoire '$wallpapersDir' n'existe pas.\n";
    exit(1);
}

if (!file_exists($csvSource)) {
    echo "ERREUR: Le fichier CSV source '$csvSource' n'existe pas.\n";
    exit(1);
}

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Synchronisation CSV avec répertoire                     ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "Répertoire: $wallpapersDir\n";
echo "CSV source: $csvSource\n";
echo "CSV de sortie: $outputCsv\n\n";

// Lire tous les fichiers du répertoire
$filesInDir = [];
$dirHandle = opendir($wallpapersDir);
while (($file = readdir($dirHandle)) !== false) {
    $filePath = $wallpapersDir . DIRECTORY_SEPARATOR . $file;
    if (is_file($filePath)) {
        $extension = strtolower(pathinfo($file, PATHINFO_EXTENSION));
        if (in_array($extension, ['png', 'jpg', 'jpeg', 'webp'])) {
            $filesInDir[$file] = true;
        }
    }
}
closedir($dirHandle);

echo "Fichiers dans le répertoire: " . count($filesInDir) . "\n";

// Lire le CSV source
$csvEntries = [];
$header = null;

$csvHandle = fopen($csvSource, 'r');
$header = fgetcsv($csvHandle, 0, ',', '"', '');

$totalInCsv = 0;
while (($row = fgetcsv($csvHandle, 0, ',', '"', '')) !== false) {
    $totalInCsv++;
    $filename = trim($row[1], '"');

    $csvEntries[$filename] = [
        'category' => trim($row[0], '"'),
        'filename' => $filename,
        'date' => trim($row[2], '"'),
        'id' => isset($row[3]) ? trim($row[3], '"') : ''
    ];
}
fclose($csvHandle);

echo "Entrées dans le CSV source: $totalInCsv\n\n";

// Filtrer pour ne garder que les fichiers qui existent
$keptEntries = [];
$removedCount = 0;

foreach ($csvEntries as $filename => $entry) {
    if (isset($filesInDir[$filename])) {
        $keptEntries[] = $entry;
    } else {
        $removedCount++;
    }
}

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Résultats de la synchronisation                         ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "✓ Entrées CONSERVÉES (fichiers existants): " . count($keptEntries) . "\n";
echo "⚠ Entrées SUPPRIMÉES (fichiers absents): $removedCount\n\n";

// Vérifier s'il y a des fichiers dans le répertoire mais pas dans le CSV
$missingInCsv = [];
foreach ($filesInDir as $filename => $dummy) {
    if (!isset($csvEntries[$filename])) {
        $missingInCsv[] = $filename;
    }
}

if (!empty($missingInCsv)) {
    echo "⚠ ATTENTION: " . count($missingInCsv) . " fichiers dans le répertoire sont absents du CSV:\n";
    echo str_repeat("-", 80) . "\n";
    foreach ($missingInCsv as $filename) {
        echo "  - $filename\n";
    }
    echo "\n";
}

// Écrire le CSV synchronisé
$outputHandle = fopen($outputCsv, 'w');

// Écrire l'en-tête
fputcsv($outputHandle, $header, ',', '"', '');

// Écrire les entrées conservées
foreach ($keptEntries as $entry) {
    fputcsv($outputHandle, [
        $entry['category'],
        $entry['filename'],
        $entry['date'],
        $entry['id']
    ], ',', '"', '');
}

fclose($outputHandle);

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Fichier synchronisé créé                                ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "✓ Fichier créé: $outputCsv\n";
echo "  - Entrées: " . count($keptEntries) . "\n";
echo "  - Correspondance: 100% (tous les fichiers du CSV existent)\n\n";

if (count($keptEntries) === count($filesInDir)) {
    echo "✓✓ PARFAITEMENT SYNCHRONISÉ!\n";
    echo "   Le CSV contient exactement les " . count($filesInDir) . " fichiers présents dans le répertoire.\n\n";
} else {
    echo "⚠ Attention: Il reste " . count($missingInCsv) . " fichiers dans le répertoire qui ne sont pas dans le CSV.\n";
    echo "  Utilisez guess_categories_and_insert.php pour les ajouter.\n\n";
}
