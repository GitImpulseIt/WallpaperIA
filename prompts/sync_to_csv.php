<?php
/**
 * Script de synchronisation des prompts vers wallpapers.csv
 *
 * Usage: php sync_to_csv.php <prompts_file> <csv_file> <date>
 *
 * Exemple: php sync_to_csv.php all_prompts.txt ../api/wallpapers.csv 22/10/2025
 */

// Vérification des arguments
if ($argc < 4) {
    echo "Usage: php sync_to_csv.php <prompts_file> <csv_file> <date>\n";
    echo "Exemple: php sync_to_csv.php all_prompts.txt ../api/wallpapers.csv 22/10/2025\n";
    exit(1);
}

$promptsFile = $argv[1];
$csvFile = $argv[2];
$date = $argv[3];

// Validation de la date
if (!preg_match('/^\d{2}\/\d{2}\/\d{4}$/', $date)) {
    echo "ERREUR: Format de date invalide. Utilisez DD/MM/YYYY\n";
    exit(1);
}

// Vérification des fichiers
if (!file_exists($promptsFile)) {
    echo "ERREUR: Fichier de prompts non trouvé: $promptsFile\n";
    exit(1);
}

if (!file_exists($csvFile)) {
    echo "ERREUR: Fichier CSV non trouvé: $csvFile\n";
    exit(1);
}

echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Synchronisation Prompts → wallpapers.csv                ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "Fichier prompts: $promptsFile\n";
echo "Fichier CSV:     $csvFile\n";
echo "Date:            $date\n\n";

// Parsing du fichier de prompts
function parsePromptsFile($file) {
    $content = file_get_contents($file);
    $prompts = [];
    $entries = explode('----', $content);

    foreach ($entries as $entry) {
        $entry = trim($entry);
        if (empty($entry)) continue;

        // Extraction des champs
        if (preg_match('/name:([^\n]+)/', $entry, $nameMatch) &&
            preg_match('/positive:([^\n]+)/', $entry, $positiveMatch) &&
            preg_match('/negative:(.+)/s', $entry, $negativeMatch)) {

            $nameParts = explode('/', $nameMatch[1]);
            if (count($nameParts) === 2) {
                $category = trim($nameParts[0]);
                $filename = trim($nameParts[1]) . '.png';
                $positive = trim($positiveMatch[1]);
                $negative = trim($negativeMatch[1]);

                // Nettoyage du negative (enlever "positive:" si présent au début)
                $negative = preg_replace('/^positive:.*?\nnegative:/s', '', $negative);
                $negative = trim($negative);

                $prompts[] = [
                    'category' => $category,
                    'filename' => $filename,
                    'positive' => $positive,
                    'negative' => $negative
                ];
            }
        }
    }

    return $prompts;
}

// Lecture du CSV existant
function readCsvFile($file) {
    $data = [];
    $existingFilenames = [];

    if (($handle = fopen($file, 'r')) !== false) {
        $header = fgetcsv($handle, 0, ',', '"', '\\'); // Skip header

        while (($row = fgetcsv($handle, 0, ',', '"', '\\')) !== false) {
            if (count($row) >= 4) {
                $data[] = [
                    'category' => $row[0],
                    'filename' => $row[1],
                    'date' => $row[2],
                    'id' => $row[3]
                ];
                $existingFilenames[] = strtolower($row[1]);
            }
        }
        fclose($handle);
    }

    return ['data' => $data, 'filenames' => $existingFilenames];
}

// Génération d'un nom de fichier alternatif basé sur le prompt
function generateAlternativeFilename($positive, $existingFilenames) {
    // Extraction des mots clés du prompt
    $words = preg_split('/[\s,]+/', strtolower($positive));
    $keywords = array_filter($words, function($word) {
        return strlen($word) > 3 && !in_array($word, ['with', 'from', 'the', 'and', 'for']);
    });

    // Prendre les 2-3 premiers mots significatifs
    $keywords = array_slice(array_values($keywords), 0, 3);
    $baseFilename = implode('_', $keywords);

    // Nettoyer le nom de fichier
    $baseFilename = preg_replace('/[^a-z0-9_]/', '', $baseFilename);

    // Vérifier les doublons et ajouter un suffixe si nécessaire
    $filename = $baseFilename . '.png';
    $counter = 2;

    while (in_array(strtolower($filename), $existingFilenames)) {
        $filename = $baseFilename . '_' . $counter . '.png';
        $counter++;
    }

    return $filename;
}

// Calculer le prochain ID pour une catégorie
function getNextIdForCategory($category, $csvData) {
    $maxId = 0;
    foreach ($csvData as $row) {
        if ($row['category'] === $category) {
            $maxId = max($maxId, (int)$row['id']);
        }
    }
    return $maxId + 1;
}

// Parsing des prompts
echo "Parsing du fichier de prompts...\n";
$prompts = parsePromptsFile($promptsFile);
echo "✓ " . count($prompts) . " prompts trouvés\n\n";

// Lecture du CSV existant
echo "Lecture du CSV existant...\n";
$csvContent = readCsvFile($csvFile);
$csvData = $csvContent['data'];
$existingFilenames = $csvContent['filenames'];
echo "✓ " . count($csvData) . " entrées existantes\n\n";

// Synchronisation
echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Synchronisation en cours...                             ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

$added = 0;
$skipped = 0;
$warnings = [];
$newEntries = [];

foreach ($prompts as $prompt) {
    $category = $prompt['category'];
    $filename = $prompt['filename'];
    $filenameLower = strtolower($filename);

    // Vérification des doublons
    if (in_array($filenameLower, $existingFilenames)) {
        $skipped++;

        // Générer un nom alternatif
        $altFilename = generateAlternativeFilename($prompt['positive'], $existingFilenames);

        $warning = [
            'original' => $filename,
            'category' => $category,
            'suggestion' => $altFilename,
            'prompt' => substr($prompt['positive'], 0, 60) . '...'
        ];
        $warnings[] = $warning;

        echo "⚠ DOUBLON: $filename\n";
        echo "  Catégorie: $category\n";
        echo "  Suggestion: $altFilename\n";
        echo "  Prompt: {$warning['prompt']}\n\n";

        continue;
    }

    // Calculer le prochain ID pour cette catégorie
    $nextId = getNextIdForCategory($category, $csvData);

    // Ajouter l'entrée
    $newEntry = [
        'category' => $category,
        'filename' => $filename,
        'date' => $date,
        'id' => $nextId
    ];

    $csvData[] = $newEntry;
    $newEntries[] = $newEntry;
    $existingFilenames[] = $filenameLower;
    $added++;

    echo "✓ Ajouté: $category / $filename (ID: $nextId)\n";
}

echo "\n";
echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Résumé                                                  ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

echo "Entrées ajoutées:  $added\n";
echo "Doublons ignorés:  $skipped\n";
echo "Total dans CSV:    " . count($csvData) . "\n\n";

// Écriture du CSV mis à jour
if ($added > 0) {
    echo "Écriture du fichier CSV mis à jour...\n";

    if (($handle = fopen($csvFile, 'w')) !== false) {
        // Header
        fputcsv($handle, ['category', 'filename', 'date', 'id']);

        // Data
        foreach ($csvData as $row) {
            fputcsv($handle, [
                $row['category'],
                $row['filename'],
                $row['date'],
                $row['id']
            ]);
        }

        fclose($handle);
        echo "✓ Fichier CSV mis à jour avec succès!\n\n";
    } else {
        echo "✗ ERREUR: Impossible d'écrire dans le fichier CSV\n\n";
        exit(1);
    }
}

// Affichage des warnings si présents
if (count($warnings) > 0) {
    echo "╔══════════════════════════════════════════════════════════╗\n";
    echo "║  Avertissements - Doublons détectés                     ║\n";
    echo "╚══════════════════════════════════════════════════════════╝\n\n";

    foreach ($warnings as $warning) {
        echo "Fichier:    {$warning['original']}\n";
        echo "Catégorie:  {$warning['category']}\n";
        echo "Prompt:     {$warning['prompt']}\n";
        echo "Suggestion: {$warning['suggestion']}\n";
        echo str_repeat('-', 60) . "\n";
    }

    echo "\n💡 Conseil: Renommez ces fichiers dans les prompts ou utilisez les noms suggérés.\n\n";
}

// Résumé final
echo "╔══════════════════════════════════════════════════════════╗\n";
echo "║  Synchronisation terminée!                               ║\n";
echo "╚══════════════════════════════════════════════════════════╝\n\n";

if ($added > 0) {
    echo "✓ $added nouvelles entrées ajoutées à wallpapers.csv\n";
    echo "✓ Date utilisée: $date\n";
}

if ($skipped > 0) {
    echo "⚠ $skipped doublons ignorés (voir suggestions ci-dessus)\n";
}

exit(0);
