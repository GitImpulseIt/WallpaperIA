<?php
/**
 * Script pour générer un fichier CSV de patch pour les noms de fichiers en collision
 * Usage: php generate_patch.php <prompts_file> <csv_file> <output_patch> <target_date>
 */

if ($argc < 5) {
    echo "Usage: php generate_patch.php <prompts_file> <csv_file> <output_patch> <target_date>\n";
    echo "\n";
    echo "Exemple: php generate_patch.php 27_10_2025/all_prompts_27_10_2025.txt ../api/wallpapers.csv insert_patch.csv 27/10/2025\n";
    exit(1);
}

$promptsFile = $argv[1];
$csvFile = $argv[2];
$outputPatch = $argv[3];
$targetDate = $argv[4];

// Vérifier que les fichiers existent
if (!file_exists($promptsFile)) {
    echo "ERREUR: Le fichier de prompts '$promptsFile' n'existe pas.\n";
    exit(1);
}

if (!file_exists($csvFile)) {
    echo "ERREUR: Le fichier CSV '$csvFile' n'existe pas.\n";
    exit(1);
}

echo "Analyse des fichiers...\n";
echo "========================\n";
echo "Fichier prompts: $promptsFile\n";
echo "Fichier CSV: $csvFile\n";
echo "Fichier patch: $outputPatch\n\n";

// Lire tous les noms de fichiers existants dans le CSV
$existingFiles = [];
$existingFilesWithDate = []; // Pour stocker les fichiers avec leur date
$csvHandle = fopen($csvFile, 'r');
fgetcsv($csvHandle, 0, ',', '"', ''); // Skip header
while (($row = fgetcsv($csvHandle, 0, ',', '"', '')) !== false) {
    $filename = trim($row[1], '"');
    $date = trim($row[2], '"');
    $existingFiles[$filename] = true;
    $existingFilesWithDate[$filename] = $date;
}
fclose($csvHandle);

echo "Fichiers existants dans le CSV: " . count($existingFiles) . "\n\n";

// Parser le fichier de prompts
$promptsContent = file_get_contents($promptsFile);
$prompts = [];
$lines = explode("\n", $promptsContent);
$currentPrompt = [];

foreach ($lines as $line) {
    $line = trim($line);

    if (preg_match('/^name:(.+)$/i', $line, $matches)) {
        $currentPrompt['name'] = trim($matches[1]);
    } elseif (preg_match('/^positive:(.+)$/i', $line, $matches)) {
        $currentPrompt['positive'] = trim($matches[1]);
    } elseif (preg_match('/^----$/', $line)) {
        if (!empty($currentPrompt['name']) && !empty($currentPrompt['positive'])) {
            // Extraire la catégorie et le nom du fichier
            $parts = explode('/', $currentPrompt['name']);
            $category = $parts[0];
            $filename = $parts[1] . '.png';

            $prompts[] = [
                'category' => $category,
                'filename' => $filename,
                'positive' => $currentPrompt['positive']
            ];
        }
        $currentPrompt = [];
    }
}

echo "Prompts trouvés: " . count($prompts) . "\n\n";

// Détecter les collisions (uniquement les fichiers pas encore dans le CSV avec la date cible)
$collisions = [];
$alreadyInserted = [];
foreach ($prompts as $prompt) {
    if (isset($existingFiles[$prompt['filename']])) {
        // Vérifier si le fichier existe avec une date différente de la date cible
        if (isset($existingFilesWithDate[$prompt['filename']]) &&
            $existingFilesWithDate[$prompt['filename']] !== $targetDate) {
            // C'est une vraie collision (fichier existe avec une autre date)
            $collisions[] = $prompt;
        } else {
            // Le fichier existe déjà avec la date cible, il a déjà été inséré
            $alreadyInserted[] = $prompt;
        }
    }
}

echo "Collisions réelles (pas encore insérées): " . count($collisions) . "\n";
echo "Déjà insérées avec la date $targetDate: " . count($alreadyInserted) . "\n\n";

if (empty($collisions)) {
    echo "Aucune collision détectée. Aucun fichier patch nécessaire.\n";
    exit(0);
}

// Générer des noms alternatifs pour chaque collision
$patchData = [];

foreach ($collisions as $collision) {
    $originalFilename = $collision['filename'];
    $category = $collision['category'];
    $positive = $collision['positive'];

    // Générer un nouveau nom basé sur le prompt
    $newName = generateAlternativeName($positive, $existingFiles);

    $patchData[] = [
        'category' => $category,
        'original_filename' => $originalFilename,
        'new_filename' => $newName,
        'prompt_preview' => substr($positive, 0, 80) . (strlen($positive) > 80 ? '...' : '')
    ];

    // Ajouter le nouveau nom aux fichiers existants pour éviter les doublons dans le patch
    $existingFiles[$newName] = true;
}

// Écrire le fichier patch
$patchHandle = fopen($outputPatch, 'w');
fputcsv($patchHandle, ['category', 'original_filename', 'new_filename', 'prompt_preview'], ',', '"', '');

foreach ($patchData as $row) {
    fputcsv($patchHandle, [
        $row['category'],
        $row['original_filename'],
        $row['new_filename'],
        $row['prompt_preview']
    ], ',', '"', '');
}

fclose($patchHandle);

echo "✓ Fichier patch généré: $outputPatch\n";
echo "\nAperçu des suggestions:\n";
echo str_repeat("=", 100) . "\n";

foreach ($patchData as $row) {
    printf("%-30s -> %-35s [%s]\n",
        substr($row['original_filename'], 0, 28),
        substr($row['new_filename'], 0, 33),
        $row['category']
    );
}

echo str_repeat("=", 100) . "\n";
echo "\nTotal: " . count($patchData) . " renommages proposés\n";

/**
 * Génère un nom de fichier alternatif basé sur le prompt
 */
function generateAlternativeName($prompt, &$existingFiles) {
    // Nettoyer et extraire les mots-clés du prompt
    $prompt = strtolower($prompt);
    $prompt = preg_replace('/[^a-z0-9\s]/', ' ', $prompt);

    // Mots à ignorer (stop words)
    $stopWords = ['the', 'a', 'an', 'and', 'or', 'but', 'in', 'on', 'at', 'to', 'for',
                  'of', 'with', 'by', 'from', 'as', 'is', 'was', 'are', 'were', 'been',
                  'be', 'have', 'has', 'had', 'do', 'does', 'did', 'will', 'would',
                  'could', 'should', 'may', 'might', 'must', 'can', 'that', 'this',
                  'these', 'those', 'it', 'its'];

    // Extraire les mots
    $words = explode(' ', $prompt);
    $words = array_filter($words, function($word) use ($stopWords) {
        return strlen($word) > 2 && !in_array($word, $stopWords);
    });

    // Prendre les 3-4 premiers mots significatifs
    $keywords = array_slice(array_values($words), 0, 4);

    // Générer le nom de base
    $baseName = implode('_', $keywords);
    $filename = $baseName . '.png';

    // Si le nom existe déjà, ajouter un suffixe
    $counter = 2;
    while (isset($existingFiles[$filename])) {
        $filename = $baseName . '_' . $counter . '.png';
        $counter++;
    }

    // Limiter la longueur du nom de fichier
    if (strlen($filename) > 50) {
        $baseName = implode('_', array_slice($keywords, 0, 3));
        $filename = $baseName . '.png';

        $counter = 2;
        while (isset($existingFiles[$filename])) {
            $filename = $baseName . '_' . $counter . '.png';
            $counter++;
        }
    }

    return $filename;
}
