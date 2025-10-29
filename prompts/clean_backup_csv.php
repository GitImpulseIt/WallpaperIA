<?php
/**
 * Script pour nettoyer un CSV backup en retirant les entrées dont les fichiers n'existent pas
 * Usage: php clean_backup_csv.php <backup_csv> <wallpapers_dir> <output_csv>
 */

if ($argc < 4) {
    echo "Usage: php clean_backup_csv.php <backup_csv> <wallpapers_dir> <output_csv>\n";
    echo "\n";
    echo "Exemple: php clean_backup_csv.php ../api/wallpapers.csv.backup.20251028211209 C:\\Installation\\ComfyUI\\wallpapers wallpapers_cleaned.csv\n";
    exit(1);
}

$backupCsv = $argv[1];
$wallpapersDir = $argv[2];
$outputCsv = $argv[3];

// Vérifier que les fichiers/répertoires existent
if (!file_exists($backupCsv)) {
    echo "ERREUR: Le fichier CSV backup '$backupCsv' n'existe pas.\n";
    exit(1);
}

if (!is_dir($wallpapersDir)) {
    echo "ERREUR: Le répertoire '$wallpapersDir' n'existe pas.\n";
    exit(1);
}

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Nettoyage du CSV backup                                 ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "Fichier backup: $backupCsv\n";
echo "Répertoire: $wallpapersDir\n";
echo "Fichier de sortie: $outputCsv\n\n";

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

// Lire le CSV backup
$backupEntries = [];
$header = null;
$csvHandle = fopen($backupCsv, 'r');
$header = fgetcsv($csvHandle, 0, ',', '"', ''); // Save header

$totalEntries = 0;
while (($row = fgetcsv($csvHandle, 0, ',', '"', '')) !== false) {
    $totalEntries++;
    $category = trim($row[0], '"');
    $filename = trim($row[1], '"');
    $date = trim($row[2], '"');
    $id = isset($row[3]) ? trim($row[3], '"') : '';

    $backupEntries[] = [
        'category' => $category,
        'filename' => $filename,
        'date' => $date,
        'id' => $id,
        'exists' => isset($filesInDir[$filename])
    ];
}
fclose($csvHandle);

echo "Entrées dans le backup CSV: " . $totalEntries . "\n\n";

// Filtrer les entrées
$keptEntries = [];
$removedEntries = [];

foreach ($backupEntries as $entry) {
    if ($entry['exists']) {
        $keptEntries[] = $entry;
    } else {
        $removedEntries[] = $entry;
    }
}

// Afficher les résultats
echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Résultats du nettoyage                                  ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "✓ Entrées CONSERVÉES: " . count($keptEntries) . "\n";
echo "⚠ Entrées SUPPRIMÉES: " . count($removedEntries) . "\n\n";

if (!empty($removedEntries)) {
    echo "Fichiers supprimés du CSV (fichiers absents du répertoire):\n";
    echo str_repeat("-", 100) . "\n";
    foreach ($removedEntries as $entry) {
        printf("%-50s  %-30s  [%s]\n",
            $entry['filename'],
            substr($entry['category'], 0, 28),
            $entry['date']
        );
    }
    echo "\n";
}

// Écrire le CSV nettoyé
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
echo "║  Fichier nettoyé généré                                  ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "✓ Fichier créé: $outputCsv\n";
echo "  - Entrées conservées: " . count($keptEntries) . "\n";
echo "  - Entrées supprimées: " . count($removedEntries) . "\n";
echo "  - Taux de conservation: " . round((count($keptEntries) / $totalEntries) * 100, 1) . "%\n\n";

if (count($removedEntries) > 0) {
    echo "✓ Le CSV a été nettoyé avec succès!\n";
    echo "  Les " . count($removedEntries) . " entrées dont les fichiers sont absents ont été retirées.\n\n";
} else {
    echo "✓ Aucune entrée à supprimer - le CSV était déjà propre!\n\n";
}
