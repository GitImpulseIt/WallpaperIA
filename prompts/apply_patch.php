<?php
/**
 * Script pour appliquer le patch et insérer les entrées dans wallpapers.csv
 * Usage: php apply_patch.php <patch_file> <csv_file> <target_date>
 */

if ($argc < 4) {
    echo "Usage: php apply_patch.php <patch_file> <csv_file> <target_date>\n";
    echo "\n";
    echo "Exemple: php apply_patch.php insert_patch.csv ../api/wallpapers.csv 27/10/2025\n";
    exit(1);
}

$patchFile = $argv[1];
$csvFile = $argv[2];
$targetDate = $argv[3];

// Vérifier que les fichiers existent
if (!file_exists($patchFile)) {
    echo "ERREUR: Le fichier patch '$patchFile' n'existe pas.\n";
    exit(1);
}

if (!file_exists($csvFile)) {
    echo "ERREUR: Le fichier CSV '$csvFile' n'existe pas.\n";
    exit(1);
}

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Application du patch dans wallpapers.csv                ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "Fichier patch: $patchFile\n";
echo "Fichier CSV: $csvFile\n";
echo "Date cible: $targetDate\n\n";

// Lire le CSV existant
$existingEntries = [];
$existingFiles = [];
$maxIdPerCategory = [];

$csvHandle = fopen($csvFile, 'r');
$header = fgetcsv($csvHandle, 0, ',', '"', ''); // Skip header

while (($row = fgetcsv($csvHandle, 0, ',', '"', '')) !== false) {
    $category = trim($row[0], '"');
    $filename = trim($row[1], '"');
    $date = trim($row[2], '"');
    $id = isset($row[3]) ? (int)trim($row[3], '"') : 1;

    $existingEntries[] = [
        'category' => $category,
        'filename' => $filename,
        'date' => $date,
        'id' => $id
    ];

    $existingFiles[$filename] = true;

    // Trouver l'ID max par catégorie
    if (!isset($maxIdPerCategory[$category]) || $maxIdPerCategory[$category] < $id) {
        $maxIdPerCategory[$category] = $id;
    }
}
fclose($csvHandle);

echo "Entrées existantes dans le CSV: " . count($existingEntries) . "\n";
echo "Catégories détectées: " . count($maxIdPerCategory) . "\n\n";

// Lire le fichier patch
$patchEntries = [];
$patchHandle = fopen($patchFile, 'r');
$patchHeader = fgetcsv($patchHandle, 0, ',', '"', ''); // Skip header

while (($row = fgetcsv($patchHandle, 0, ',', '"', '')) !== false) {
    if (count($row) < 3) {
        continue;
    }

    $category = trim($row[0], '"');
    $originalFilename = trim($row[1], '"');
    $newFilename = trim($row[2], '"');

    $patchEntries[] = [
        'category' => $category,
        'original_filename' => $originalFilename,
        'new_filename' => $newFilename
    ];
}
fclose($patchHandle);

echo "Entrées dans le patch: " . count($patchEntries) . "\n\n";

// Vérifier les collisions avec les nouveaux noms
$collisions = [];
$entriesToInsert = [];

foreach ($patchEntries as $entry) {
    $newFilename = $entry['new_filename'];
    $category = $entry['category'];
    $originalFilename = $entry['original_filename'];

    // Vérifier si le nouveau nom existe déjà
    if (isset($existingFiles[$newFilename])) {
        $collisions[] = [
            'category' => $category,
            'original' => $originalFilename,
            'proposed' => $newFilename
        ];
    } else {
        // Calculer le prochain ID pour cette catégorie
        $nextId = isset($maxIdPerCategory[$category]) ? $maxIdPerCategory[$category] + 1 : 1;
        $maxIdPerCategory[$category] = $nextId;

        $entriesToInsert[] = [
            'category' => $category,
            'filename' => $newFilename,
            'date' => $targetDate,
            'id' => $nextId
        ];

        // Marquer le nouveau fichier comme existant pour éviter les doublons dans le même batch
        $existingFiles[$newFilename] = true;
    }
}

// Afficher les résultats
if (!empty($collisions)) {
    echo "╔══════════════════════════════════════════════════════════╗\n";
    echo "║  ERREUR: Collisions détectées avec les nouveaux noms!   ║\n";
    echo "╚══════════════════════════════════════════════════════════╝\n\n";

    foreach ($collisions as $collision) {
        echo "Catégorie:  {$collision['category']}\n";
        echo "Original:   {$collision['original']}\n";
        echo "Proposé:    {$collision['proposed']} (DÉJÀ EXISTANT!)\n";
        echo str_repeat("-", 60) . "\n";
    }

    echo "\n❌ Impossible d'appliquer le patch. Veuillez corriger les noms dans le fichier patch.\n";
    exit(1);
}

if (empty($entriesToInsert)) {
    echo "⚠ Aucune entrée à insérer.\n";
    exit(0);
}

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Aperçu des insertions                                   ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

foreach ($entriesToInsert as $entry) {
    printf("%-30s  %s  [ID: %d]\n",
        substr($entry['category'], 0, 28),
        substr($entry['filename'], 0, 50),
        $entry['id']
    );
}

echo "\n" . str_repeat("=", 60) . "\n";
echo "Total: " . count($entriesToInsert) . " entrées à insérer\n";
echo str_repeat("=", 60) . "\n\n";

// Demander confirmation
echo "Voulez-vous continuer l'insertion ? (oui/non): ";
$confirmation = trim(fgets(STDIN));

if (strtolower($confirmation) !== 'oui' && strtolower($confirmation) !== 'o' && strtolower($confirmation) !== 'y' && strtolower($confirmation) !== 'yes') {
    echo "\n❌ Insertion annulée.\n";
    exit(0);
}

echo "\n";

// Créer une sauvegarde du CSV
$backupFile = $csvFile . '.backup.' . date('YmdHis');
if (!copy($csvFile, $backupFile)) {
    echo "ERREUR: Impossible de créer la sauvegarde.\n";
    exit(1);
}

echo "✓ Sauvegarde créée: " . basename($backupFile) . "\n";

// Ajouter les nouvelles entrées au CSV existant
$csvHandle = fopen($csvFile, 'a');

$insertedCount = 0;
foreach ($entriesToInsert as $entry) {
    $row = [
        $entry['category'],
        $entry['filename'],
        $entry['date'],
        $entry['id']
    ];

    if (fputcsv($csvHandle, $row, ',', '"', '')) {
        $insertedCount++;
        echo "✓ Inséré: {$entry['category']} / {$entry['filename']} [ID: {$entry['id']}]\n";
    } else {
        echo "❌ ERREUR lors de l'insertion: {$entry['filename']}\n";
    }
}

fclose($csvHandle);

echo "\n╔══════════════════════════════════════════════════════════╗\n";
echo "║  Insertion terminée!                                     ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "✓ $insertedCount entrées insérées avec succès\n";
echo "✓ Sauvegarde disponible: $backupFile\n";

// Afficher un résumé par catégorie
$categoryStats = [];
foreach ($entriesToInsert as $entry) {
    $cat = $entry['category'];
    if (!isset($categoryStats[$cat])) {
        $categoryStats[$cat] = 0;
    }
    $categoryStats[$cat]++;
}

echo "\nRésumé par catégorie:\n";
echo str_repeat("-", 60) . "\n";
foreach ($categoryStats as $category => $count) {
    printf("%-40s  %2d entrée(s)\n", $category, $count);
}
echo str_repeat("-", 60) . "\n";
