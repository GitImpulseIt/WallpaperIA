<?php

$themes = [
    'theme01_cyberpunk.txt' => 'CYBERPUNK/FUTURISTIC',
    'theme02_fantasy.txt' => 'FANTASY WORLDS',
    'theme03_natural_landscapes.txt' => 'NATURAL LANDSCAPES', 
    'theme04_urban_architecture.txt' => 'URBAN ARCHITECTURE',
    'theme05_minimalist_abstract.txt' => 'MINIMALIST ABSTRACT',
    'theme06_ocean_marine.txt' => 'OCEAN & MARINE',
    'theme07_space_cosmos.txt' => 'SPACE & COSMOS',
    'theme08_autumn_seasons.txt' => 'AUTUMN SEASONS',
    'theme09_mountains_peaks.txt' => 'MOUNTAINS & PEAKS',
    'theme10_tropical_paradise.txt' => 'TROPICAL PARADISE',
    'theme11_mysterious_gothic.txt' => 'MYSTERIOUS GOTHIC',
    'theme12_geometric_patterns.txt' => 'GEOMETRIC PATTERNS',
    'theme13_vintage_retro.txt' => 'VINTAGE RETRO',
    'theme14_winter_wonderland.txt' => 'WINTER WONDERLAND',
    'theme15_desert_landscapes.txt' => 'DESERT LANDSCAPES'
];

$promptsDir = __DIR__ . '/prompts/';
$outputFile = $promptsDir . 'MASTER_150_PROMPTS.txt';

$combinedContent = "# MASTER FILE - 150 WALLPAPER PROMPTS\n";
$combinedContent .= "# Generated from 15 themes with 10 prompts each\n";
$combinedContent .= "# Format: CLIP_L (positive) / T5 (negative) for Flux.1 Dev\n\n";

$totalPrompts = 0;

foreach ($themes as $filename => $themeName) {
    $filePath = $promptsDir . $filename;
    
    if (!file_exists($filePath)) {
        echo "ERREUR: Fichier non trouvé: $filePath\n";
        continue;
    }
    
    $content = file_get_contents($filePath);
    if ($content === false) {
        echo "ERREUR: Impossible de lire le fichier: $filePath\n";
        continue;
    }
    
    // Compter les prompts (chaque "positive:" = 1 prompt)
    $promptCount = substr_count($content, 'positive:');
    $totalPrompts += $promptCount;
    
    // Ajouter l'en-tête de thème
    $combinedContent .= "# ======================================\n";
    $combinedContent .= "# THEME: $themeName ($promptCount prompts)\n"; 
    $combinedContent .= "# ======================================\n\n";
    
    // Ajouter le contenu du fichier
    $combinedContent .= $content . "\n\n";
    
    echo "✓ Ajouté: $themeName - $promptCount prompts\n";
}

// Ajouter statistiques finales
$combinedContent .= "# ======================================\n";
$combinedContent .= "# STATISTIQUES FINALES\n";
$combinedContent .= "# Total: $totalPrompts prompts\n";
$combinedContent .= "# Thèmes: " . count($themes) . "\n";
$combinedContent .= "# Date: " . date('Y-m-d H:i:s') . "\n";
$combinedContent .= "# ======================================\n";

// Écrire le fichier final
$result = file_put_contents($outputFile, $combinedContent);

if ($result !== false) {
    echo "\n✅ SUCCÈS: Fichier créé avec $totalPrompts prompts\n";
    echo "📁 Fichier: $outputFile\n";
    echo "📊 Taille: " . number_format(filesize($outputFile)) . " octets\n";
} else {
    echo "\n❌ ERREUR: Impossible de créer le fichier final\n";
}

?>