<?php
/**
 * Script pour vérifier la synchronisation entre le répertoire FTP et le CSV
 * Usage: php check_files_sync.php <wallpapers_dir> <csv_file> <output_csv>
 */

if ($argc < 4) {
    echo "Usage: php check_files_sync.php <wallpapers_dir> <csv_file> <output_csv>\n";
    echo "\n";
    echo "Exemple: php check_files_sync.php C:\\Installation\\ComfyUI\\wallpapers ../api/wallpapers.csv sync_diff.csv\n";
    exit(1);
}

$wallpapersDir = $argv[1];
$csvFile = $argv[2];
$outputCsv = $argv[3];

// Vérifier que les fichiers/répertoires existent
if (!is_dir($wallpapersDir)) {
    echo "ERREUR: Le répertoire '$wallpapersDir' n'existe pas.\n";
    exit(1);
}

if (!file_exists($csvFile)) {
    echo "ERREUR: Le fichier CSV '$csvFile' n'existe pas.\n";
    exit(1);
}

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Analyse de synchronisation                              ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "Répertoire: $wallpapersDir\n";
echo "Fichier CSV: $csvFile\n\n";

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

// Lire tous les fichiers du CSV
$filesInCsv = [];
$csvCategories = [];
$csvHandle = fopen($csvFile, 'r');
fgetcsv($csvHandle, 0, ',', '"', ''); // Skip header

while (($row = fgetcsv($csvHandle, 0, ',', '"', '')) !== false) {
    $category = trim($row[0], '"');
    $filename = trim($row[1], '"');
    $date = trim($row[2], '"');

    $filesInCsv[$filename] = [
        'category' => $category,
        'date' => $date
    ];
    $csvCategories[$filename] = $category;
}
fclose($csvHandle);

echo "Entrées dans le CSV: " . count($filesInCsv) . "\n\n";

// Identifier les différences
$missingInDir = []; // Dans CSV mais pas dans répertoire
$missingInCsv = []; // Dans répertoire mais pas dans CSV

// Fichiers dans CSV mais pas dans répertoire
foreach ($filesInCsv as $filename => $info) {
    if (!isset($filesInDir[$filename])) {
        $missingInDir[] = [
            'filename' => $filename,
            'category' => $info['category'],
            'date' => $info['date']
        ];
    }
}

// Fichiers dans répertoire mais pas dans CSV
foreach ($filesInDir as $filename => $dummy) {
    if (!isset($filesInCsv[$filename])) {
        $missingInCsv[] = [
            'filename' => $filename
        ];
    }
}

// Afficher les résultats
echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Résultats de l'analyse                                  ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

if (!empty($missingInDir)) {
    echo "⚠ Fichiers dans CSV mais ABSENTS du répertoire: " . count($missingInDir) . "\n";
    echo str_repeat("-", 80) . "\n";
    foreach ($missingInDir as $file) {
        printf("%-50s  [%s]\n", $file['filename'], $file['category']);
    }
    echo "\n";
} else {
    echo "✓ Tous les fichiers du CSV sont présents dans le répertoire\n\n";
}

if (!empty($missingInCsv)) {
    echo "⚠ Fichiers dans répertoire mais ABSENTS du CSV: " . count($missingInCsv) . "\n";
    echo str_repeat("-", 80) . "\n";
    foreach ($missingInCsv as $file) {
        echo $file['filename'] . "\n";
    }
    echo "\n";
} else {
    echo "✓ Tous les fichiers du répertoire sont enregistrés dans le CSV\n\n";
}

// Écrire le fichier CSV de différences
$outputHandle = fopen($outputCsv, 'w');
fputcsv($outputHandle, ['type', 'filename', 'category', 'date', 'note'], ',', '"', '');

// Ajouter les fichiers manquants dans le répertoire
foreach ($missingInDir as $file) {
    fputcsv($outputHandle, [
        'missing_in_directory',
        $file['filename'],
        $file['category'],
        $file['date'],
        'Présent dans CSV mais absent du répertoire FTP'
    ], ',', '"', '');
}

// Ajouter les fichiers manquants dans le CSV
foreach ($missingInCsv as $file) {
    fputcsv($outputHandle, [
        'missing_in_csv',
        $file['filename'],
        '',
        '',
        'Présent dans le répertoire FTP mais absent du CSV'
    ], ',', '"', '');
}

fclose($outputHandle);

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Rapport généré                                          ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "✓ Fichier de rapport créé: $outputCsv\n";
echo "  - Fichiers manquants dans répertoire: " . count($missingInDir) . "\n";
echo "  - Fichiers manquants dans CSV: " . count($missingInCsv) . "\n";
echo "  - Total des différences: " . (count($missingInDir) + count($missingInCsv)) . "\n\n";

if (count($missingInDir) > 0 || count($missingInCsv) > 0) {
    echo "⚠ ATTENTION: Des différences ont été détectées!\n";
    echo "\nRecommandations:\n";

    if (count($missingInDir) > 0) {
        echo "  1. Fichiers manquants dans le répertoire:\n";
        echo "     - Générer et uploader ces images avec ComfyUI\n";
        echo "     - Ou supprimer ces entrées du CSV si obsolètes\n\n";
    }

    if (count($missingInCsv) > 0) {
        echo "  2. Fichiers manquants dans le CSV:\n";
        echo "     - Ajouter ces fichiers dans le CSV avec sync_to_csv.php\n";
        echo "     - Ou supprimer ces fichiers du répertoire s'ils sont en trop\n\n";
    }

    exit(1);
} else {
    echo "✓ Le répertoire et le CSV sont parfaitement synchronisés!\n";
    exit(0);
}
