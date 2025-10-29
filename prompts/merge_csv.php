<?php
/**
 * Script pour fusionner deux fichiers CSV wallpapers
 * Usage: php merge_csv.php <csv_base> <csv_to_add> <output_csv>
 */

if ($argc < 4) {
    echo "Usage: php merge_csv.php <csv_base> <csv_to_add> <output_csv>\n";
    echo "\n";
    echo "Exemple: php merge_csv.php ../api/wallpapers.csv insert_missing.csv ../api/wallpapers_merged.csv\n";
    exit(1);
}

$csvBase = $argv[1];
$csvToAdd = $argv[2];
$outputCsv = $argv[3];

// Vérifier que les fichiers existent
if (!file_exists($csvBase)) {
    echo "ERREUR: Le fichier CSV de base '$csvBase' n'existe pas.\n";
    exit(1);
}

if (!file_exists($csvToAdd)) {
    echo "ERREUR: Le fichier CSV à ajouter '$csvToAdd' n'existe pas.\n";
    exit(1);
}

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Fusion de fichiers CSV                                  ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "CSV de base: $csvBase\n";
echo "CSV à ajouter: $csvToAdd\n";
echo "Fichier de sortie: $outputCsv\n\n";

// Lire le CSV de base
$allEntries = [];
$existingFiles = [];
$header = null;

$baseHandle = fopen($csvBase, 'r');
$header = fgetcsv($baseHandle, 0, ',', '"', '');

while (($row = fgetcsv($baseHandle, 0, ',', '"', '')) !== false) {
    $filename = trim($row[1], '"');
    $existingFiles[$filename] = true;

    $allEntries[] = [
        'category' => trim($row[0], '"'),
        'filename' => $filename,
        'date' => trim($row[2], '"'),
        'id' => isset($row[3]) ? trim($row[3], '"') : ''
    ];
}
fclose($baseHandle);

echo "Entrées dans le CSV de base: " . count($allEntries) . "\n";

// Lire le CSV à ajouter
$addedCount = 0;
$skippedCount = 0;

$addHandle = fopen($csvToAdd, 'r');
fgetcsv($addHandle, 0, ',', '"', ''); // Skip header

while (($row = fgetcsv($addHandle, 0, ',', '"', '')) !== false) {
    $filename = trim($row[1], '"');

    // Vérifier si le fichier existe déjà
    if (isset($existingFiles[$filename])) {
        echo "⚠ Ignoré (doublon): $filename\n";
        $skippedCount++;
        continue;
    }

    $allEntries[] = [
        'category' => trim($row[0], '"'),
        'filename' => $filename,
        'date' => trim($row[2], '"'),
        'id' => isset($row[3]) ? trim($row[3], '"') : ''
    ];

    $existingFiles[$filename] = true;
    $addedCount++;
}
fclose($addHandle);

echo "\n";
echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Résultats de la fusion                                  ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "✓ Entrées ajoutées: $addedCount\n";
echo "⚠ Doublons ignorés: $skippedCount\n";
echo "✓ Total final: " . count($allEntries) . "\n\n";

// Écrire le CSV fusionné
$outputHandle = fopen($outputCsv, 'w');

// Écrire l'en-tête
fputcsv($outputHandle, $header, ',', '"', '');

// Écrire toutes les entrées
foreach ($allEntries as $entry) {
    fputcsv($outputHandle, [
        $entry['category'],
        $entry['filename'],
        $entry['date'],
        $entry['id']
    ], ',', '"', '');
}

fclose($outputHandle);

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Fichier fusionné créé                                   ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "✓ Fichier créé: $outputCsv\n";
echo "  - Entrées totales: " . count($allEntries) . "\n";
echo "  - Dont nouvelles: $addedCount\n\n";

if ($addedCount > 0) {
    echo "✓ Fusion réussie! $addedCount entrées ont été ajoutées.\n\n";
} else {
    echo "⚠ Aucune entrée ajoutée - tous les fichiers existaient déjà.\n\n";
}
