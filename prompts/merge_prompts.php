<?php
/**
 * Script pour fusionner tous les fichiers de prompts en un seul fichier
 * Usage: php merge_prompts.php <promptsDir> <outputFile>
 *
 * Exemple: php merge_prompts.php ./27_10_2025 all_prompts_27_10.txt
 */

// Vérification des arguments
if ($argc < 3) {
    echo "Usage: php merge_prompts.php <promptsDir> <outputFile>\n";
    echo "\n";
    echo "Arguments:\n";
    echo "  promptsDir  - Répertoire contenant les fichiers .txt à fusionner\n";
    echo "  outputFile  - Nom du fichier de sortie (sera créé dans promptsDir)\n";
    echo "\n";
    echo "Exemple: php merge_prompts.php ./27_10_2025 all_prompts_27_10.txt\n";
    exit(1);
}

$promptsDir = rtrim($argv[1], '/\\');
$outputFileName = $argv[2];

// Vérification que le répertoire existe
if (!is_dir($promptsDir)) {
    echo "ERREUR: Le répertoire '$promptsDir' n'existe pas.\n";
    exit(1);
}

// Construction du chemin complet du fichier de sortie
$outputFile = $promptsDir . DIRECTORY_SEPARATOR . $outputFileName;

echo "Fusion des fichiers de prompts\n";
echo "================================\n";
echo "Répertoire source: $promptsDir\n";
echo "Fichier de sortie: $outputFile\n\n";

/**
 * Valide la syntaxe d'un fichier de prompts selon le format ComfyUI
 *
 * @param string $content Contenu du fichier
 * @param string $filename Nom du fichier (pour les messages d'erreur)
 * @return array ['valid' => bool, 'errors' => array, 'promptCount' => int]
 */
function validatePromptFile($content, $filename) {
    $errors = [];
    $lines = explode("\n", $content);
    $promptCount = 0;
    $currentPrompt = 0;
    $expectedField = 'name';

    foreach ($lines as $lineNum => $line) {
        $line = trim($line);

        // Ignorer les lignes vides
        if ($line === '') {
            continue;
        }

        // Vérifier si c'est un séparateur
        if (preg_match('/^-{4,}$/', $line)) {
            if ($expectedField !== 'separator') {
                $errors[] = "Ligne " . ($lineNum + 1) . ": Séparateur '----' inattendu (champ attendu: $expectedField)";
            }
            $expectedField = 'name';
            $currentPrompt++;
            continue;
        }

        // Vérifier le champ name:
        if (preg_match('/^name:(.+)$/i', $line, $matches)) {
            if ($expectedField !== 'name') {
                $errors[] = "Ligne " . ($lineNum + 1) . ": Champ 'name:' inattendu (champ attendu: $expectedField)";
            }
            $name = trim($matches[1]);
            if (empty($name)) {
                $errors[] = "Ligne " . ($lineNum + 1) . ": Le champ 'name:' ne peut pas être vide";
            }
            if (!preg_match('/^[^\/]+\/[^\/]+$/', $name)) {
                $errors[] = "Ligne " . ($lineNum + 1) . ": Le champ 'name:' doit être au format 'CATEGORIE/nom_fichier' (valeur: '$name')";
            }
            $expectedField = 'positive';
            $promptCount++;
            continue;
        }

        // Vérifier le champ positive:
        if (preg_match('/^positive:(.*)$/i', $line, $matches)) {
            if ($expectedField !== 'positive') {
                $errors[] = "Ligne " . ($lineNum + 1) . ": Champ 'positive:' inattendu (champ attendu: $expectedField)";
            }
            $expectedField = 'negative';
            continue;
        }

        // Vérifier le champ negative:
        if (preg_match('/^negative:(.*)$/i', $line, $matches)) {
            if ($expectedField !== 'negative') {
                $errors[] = "Ligne " . ($lineNum + 1) . ": Champ 'negative:' inattendu (champ attendu: $expectedField)";
            }
            $expectedField = 'separator';
            continue;
        }

        // Si on arrive ici, c'est une ligne non reconnue
        // (peut-être une continuation de negative: sur plusieurs lignes, ce qui est valide)
        if ($expectedField !== 'separator') {
            // C'est OK, c'est probablement la suite du champ précédent
            continue;
        }
    }

    // Vérifier que le fichier se termine correctement
    if ($expectedField !== 'name' && $expectedField !== 'separator') {
        $errors[] = "Fin de fichier: Le fichier se termine de manière incomplète (dernier champ attendu: $expectedField)";
    }

    return [
        'valid' => empty($errors),
        'errors' => $errors,
        'promptCount' => $promptCount
    ];
}

// Scanner tous les fichiers .txt dans le répertoire
$txtFiles = glob($promptsDir . '/*.txt');

if (empty($txtFiles)) {
    echo "ERREUR: Aucun fichier .txt trouvé dans le répertoire '$promptsDir'\n";
    exit(1);
}

// Trier les fichiers par ordre alphabétique
sort($txtFiles);

echo "Fichiers trouvés: " . count($txtFiles) . "\n\n";

$totalPrompts = 0;
$mergedContent = '';
$validFiles = 0;
$invalidFiles = 0;

foreach ($txtFiles as $filePath) {
    $filename = basename($filePath);

    // Ignorer le fichier de sortie s'il existe déjà
    // Normaliser les chemins pour la comparaison (realpath résout les chemins absolus)
    $normalizedFilePath = realpath($filePath);
    $normalizedOutputFile = realpath($outputFile);
    if ($normalizedFilePath === $normalizedOutputFile || $filename === $outputFileName) {
        echo "SKIP: $filename (fichier de sortie)\n";
        continue;
    }

    echo "Traitement de $filename... ";

    $content = file_get_contents($filePath);

    // Valider la syntaxe
    $validation = validatePromptFile($content, $filename);

    if (!$validation['valid']) {
        echo "❌ ERREUR DE SYNTAXE\n";
        foreach ($validation['errors'] as $error) {
            echo "  - $error\n";
        }
        $invalidFiles++;
        continue;
    }

    $promptCount = $validation['promptCount'];
    $totalPrompts += $promptCount;
    $validFiles++;

    echo "✓ $promptCount prompts\n";

    // S'assurer que le contenu se termine correctement
    // en supprimant tous les sauts de ligne finaux
    $content = rtrim($content, "\r\n");

    // Ajouter le contenu au fichier fusionné
    if (!empty($mergedContent)) {
        // Ajouter un saut de ligne entre les fichiers pour séparer le dernier ---- du précédent du name: du suivant
        $mergedContent .= "\n";
    }
    $mergedContent .= $content;
}

echo "\n================================\n";
echo "Fichiers valides: $validFiles\n";
echo "Fichiers invalides: $invalidFiles\n";
echo "Total: $totalPrompts prompts fusionnés\n";

if ($validFiles === 0) {
    echo "\nERREUR: Aucun fichier valide à fusionner.\n";
    exit(1);
}

echo "Écriture du fichier de sortie: $outputFileName\n";

// Écrire le fichier fusionné
file_put_contents($outputFile, $mergedContent);

echo "✓ Fusion terminée avec succès!\n";
echo "Fichier créé: $outputFile\n";
echo "Taille: " . number_format(filesize($outputFile)) . " octets\n";
