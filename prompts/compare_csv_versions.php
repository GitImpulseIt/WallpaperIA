<?php
/**
 * Script pour comparer deux versions du CSV wallpapers
 * Usage: php compare_csv_versions.php <csv_old> <csv_new> <output_csv>
 */

if ($argc < 4) {
    echo "Usage: php compare_csv_versions.php <csv_old> <csv_new> <output_csv>\n";
    echo "\n";
    echo "Exemple: php compare_csv_versions.php ../api/wallpapers.csv.backup.20251028211209 ../api/wallpapers.csv csv_changes.csv\n";
    exit(1);
}

$csvOld = $argv[1];
$csvNew = $argv[2];
$outputCsv = $argv[3];

// Vérifier que les fichiers existent
if (!file_exists($csvOld)) {
    echo "ERREUR: Le fichier CSV ancien '$csvOld' n'existe pas.\n";
    exit(1);
}

if (!file_exists($csvNew)) {
    echo "ERREUR: Le fichier CSV nouveau '$csvNew' n'existe pas.\n";
    exit(1);
}

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Comparaison des versions CSV                            ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "Ancien CSV: $csvOld\n";
echo "Nouveau CSV: $csvNew\n\n";

// Lire l'ancien CSV
$entriesOld = [];
$csvHandle = fopen($csvOld, 'r');
fgetcsv($csvHandle, 0, ',', '"', ''); // Skip header

while (($row = fgetcsv($csvHandle, 0, ',', '"', '')) !== false) {
    $category = trim($row[0], '"');
    $filename = trim($row[1], '"');
    $date = trim($row[2], '"');
    $id = isset($row[3]) ? trim($row[3], '"') : '';

    $entriesOld[$filename] = [
        'category' => $category,
        'date' => $date,
        'id' => $id
    ];
}
fclose($csvHandle);

echo "Entrées dans l'ancien CSV: " . count($entriesOld) . "\n";

// Lire le nouveau CSV
$entriesNew = [];
$csvHandle = fopen($csvNew, 'r');
fgetcsv($csvHandle, 0, ',', '"', ''); // Skip header

while (($row = fgetcsv($csvHandle, 0, ',', '"', '')) !== false) {
    $category = trim($row[0], '"');
    $filename = trim($row[1], '"');
    $date = trim($row[2], '"');
    $id = isset($row[3]) ? trim($row[3], '"') : '';

    $entriesNew[$filename] = [
        'category' => $category,
        'date' => $date,
        'id' => $id
    ];
}
fclose($csvHandle);

echo "Entrées dans le nouveau CSV: " . count($entriesNew) . "\n\n";

// Identifier les différences
$added = []; // Nouvelles entrées
$removed = []; // Entrées supprimées
$modified = []; // Entrées modifiées

// Fichiers ajoutés dans le nouveau CSV
foreach ($entriesNew as $filename => $info) {
    if (!isset($entriesOld[$filename])) {
        $added[] = [
            'filename' => $filename,
            'category' => $info['category'],
            'date' => $info['date'],
            'id' => $info['id']
        ];
    } elseif ($entriesOld[$filename]['category'] !== $info['category'] ||
              $entriesOld[$filename]['date'] !== $info['date']) {
        $modified[] = [
            'filename' => $filename,
            'old_category' => $entriesOld[$filename]['category'],
            'new_category' => $info['category'],
            'old_date' => $entriesOld[$filename]['date'],
            'new_date' => $info['date']
        ];
    }
}

// Fichiers supprimés du nouveau CSV
foreach ($entriesOld as $filename => $info) {
    if (!isset($entriesNew[$filename])) {
        $removed[] = [
            'filename' => $filename,
            'category' => $info['category'],
            'date' => $info['date'],
            'id' => $info['id']
        ];
    }
}

// Afficher les résultats
echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Résultats de la comparaison                             ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

if (!empty($added)) {
    echo "✓ Entrées AJOUTÉES: " . count($added) . "\n";
    echo str_repeat("-", 100) . "\n";
    foreach ($added as $entry) {
        printf("%-50s  %-30s  [ID: %s] [%s]\n",
            $entry['filename'],
            substr($entry['category'], 0, 28),
            $entry['id'],
            $entry['date']
        );
    }
    echo "\n";
} else {
    echo "• Aucune entrée ajoutée\n\n";
}

if (!empty($removed)) {
    echo "⚠ Entrées SUPPRIMÉES: " . count($removed) . "\n";
    echo str_repeat("-", 100) . "\n";
    foreach ($removed as $entry) {
        printf("%-50s  %-30s  [%s]\n",
            $entry['filename'],
            substr($entry['category'], 0, 28),
            $entry['date']
        );
    }
    echo "\n";
} else {
    echo "• Aucune entrée supprimée\n\n";
}

if (!empty($modified)) {
    echo "⚠ Entrées MODIFIÉES: " . count($modified) . "\n";
    echo str_repeat("-", 100) . "\n";
    foreach ($modified as $entry) {
        printf("%-50s\n", $entry['filename']);
        printf("  Catégorie: %s → %s\n", $entry['old_category'], $entry['new_category']);
        printf("  Date: %s → %s\n\n", $entry['old_date'], $entry['new_date']);
    }
} else {
    echo "• Aucune entrée modifiée\n\n";
}

// Écrire le fichier CSV de changements
$outputHandle = fopen($outputCsv, 'w');
fputcsv($outputHandle, ['change_type', 'filename', 'category', 'date', 'id', 'details'], ',', '"', '');

// Ajouter les entrées ajoutées
foreach ($added as $entry) {
    fputcsv($outputHandle, [
        'added',
        $entry['filename'],
        $entry['category'],
        $entry['date'],
        $entry['id'],
        'Nouvelle entrée ajoutée au CSV'
    ], ',', '"', '');
}

// Ajouter les entrées supprimées
foreach ($removed as $entry) {
    fputcsv($outputHandle, [
        'removed',
        $entry['filename'],
        $entry['category'],
        $entry['date'],
        $entry['id'],
        'Entrée supprimée du CSV'
    ], ',', '"', '');
}

// Ajouter les entrées modifiées
foreach ($modified as $entry) {
    fputcsv($outputHandle, [
        'modified',
        $entry['filename'],
        $entry['new_category'],
        $entry['new_date'],
        '',
        "Catégorie: {$entry['old_category']} → {$entry['new_category']}, Date: {$entry['old_date']} → {$entry['new_date']}"
    ], ',', '"', '');
}

fclose($outputHandle);

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Rapport généré                                          ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "✓ Fichier de rapport créé: $outputCsv\n";
echo "  - Entrées ajoutées: " . count($added) . "\n";
echo "  - Entrées supprimées: " . count($removed) . "\n";
echo "  - Entrées modifiées: " . count($modified) . "\n";
echo "  - Total des changements: " . (count($added) + count($removed) + count($modified)) . "\n\n";

if (count($added) > 0 || count($removed) > 0 || count($modified) > 0) {
    echo "Différence nette: " . (count($added) - count($removed)) . " entrées\n";
}
