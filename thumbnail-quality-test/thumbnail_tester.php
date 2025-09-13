<?php
/**
 * Thumbnail Quality Tester
 * 
 * Usage: php thumbnail_tester.php <input_image> <scale_ratio>
 * Example: php thumbnail_tester.php input.png 25
 * 
 * Scale ratio: percentage of original size (e.g., 50 = 50% of original size)
 */

if ($argc < 3) {
    echo "Usage: php thumbnail_tester.php <input_image> <scale_ratio>\n";
    echo "Example: php thumbnail_tester.php input.png 25\n";
    echo "Scale ratio: percentage of original size (e.g., 50 = 50%)\n";
    exit(1);
}

$input_image = $argv[1];
$scale_ratio = floatval($argv[2]) / 100;

if (!file_exists($input_image)) {
    echo "Error: Input image '{$input_image}' not found.\n";
    exit(1);
}

if ($scale_ratio <= 0 || $scale_ratio > 1) {
    echo "Error: Scale ratio must be between 1 and 100.\n";
    exit(1);
}

// Get image info
$image_info = getimagesize($input_image);
if ($image_info === false) {
    echo "Error: Invalid image file.\n";
    exit(1);
}

$original_width = $image_info[0];
$original_height = $image_info[1];
$mime_type = $image_info['mime'];

// Calculate new dimensions
$new_width = (int)($original_width * $scale_ratio);
$new_height = (int)($original_height * $scale_ratio);

echo "=== Thumbnail Quality Tester ===\n";
echo "Input: {$input_image}\n";
echo "Original size: {$original_width}x{$original_height}\n";
echo "Target size: {$new_width}x{$new_height} ({$scale_ratio}%)\n";
echo "MIME type: {$mime_type}\n\n";

// Create source image
$source = null;
switch ($mime_type) {
    case 'image/jpeg':
        $source = imagecreatefromjpeg($input_image);
        break;
    case 'image/png':
        $source = imagecreatefrompng($input_image);
        break;
    case 'image/gif':
        $source = imagecreatefromgif($input_image);
        break;
    case 'image/webp':
        if (function_exists('imagecreatefromwebp')) {
            $source = imagecreatefromwebp($input_image);
        }
        break;
}

if (!$source) {
    echo "Error: Unable to create image resource from input file.\n";
    exit(1);
}

$output_dir = 'output_' . pathinfo($input_image, PATHINFO_FILENAME);
if (!is_dir($output_dir)) {
    mkdir($output_dir, 0755, true);
}

echo "Output directory: {$output_dir}/\n\n";

// Test 1: Basic imagecopyresampled (current API method)
echo "1. Testing basic imagecopyresampled...\n";
$thumb1 = imagecreatetruecolor($new_width, $new_height);
$white = imagecolorallocate($thumb1, 255, 255, 255);
imagefill($thumb1, 0, 0, $white);
imagealphablending($thumb1, true);

imagecopyresampled($thumb1, $source, 0, 0, 0, 0, 
                   $new_width, $new_height, $original_width, $original_height);

// Apply current API filters
if (function_exists('imagefilter')) {
    imagefilter($thumb1, IMG_FILTER_CONTRAST, -10);
    imagefilter($thumb1, IMG_FILTER_SMOOTH, -1);
}

imagejpeg($thumb1, "{$output_dir}/method1_basic_resampled.jpg", 95);
imagedestroy($thumb1);
echo "   → method1_basic_resampled.jpg (current API method)\n";

// Test 2: Enhanced resampling with different interpolation
echo "2. Testing enhanced resampling...\n";
$thumb2 = imagecreatetruecolor($new_width, $new_height);
$white2 = imagecolorallocate($thumb2, 255, 255, 255);
imagefill($thumb2, 0, 0, $white2);
imagealphablending($thumb2, true);

// Use higher quality resampling
imagecopyresampled($thumb2, $source, 0, 0, 0, 0, 
                   $new_width, $new_height, $original_width, $original_height);

// Different filter combination  
if (function_exists('imagefilter')) {
    imagefilter($thumb2, IMG_FILTER_CONTRAST, -15); // More contrast
    imagefilter($thumb2, IMG_FILTER_SMOOTH, -2); // More sharpening
}

imagejpeg($thumb2, "{$output_dir}/method2_enhanced_resampled.jpg", 98);
imagedestroy($thumb2);
echo "   → method2_enhanced_resampled.jpg (higher quality + edge enhance)\n";

// Test 3: Two-step resizing (large -> medium -> final)
echo "3. Testing two-step resizing...\n";
if ($scale_ratio < 0.5) {
    // First resize to 50% if target is smaller than 50%
    $intermediate_width = (int)($original_width * 0.5);
    $intermediate_height = (int)($original_height * 0.5);
    
    $intermediate = imagecreatetruecolor($intermediate_width, $intermediate_height);
    $white3a = imagecolorallocate($intermediate, 255, 255, 255);
    imagefill($intermediate, 0, 0, $white3a);
    imagealphablending($intermediate, true);
    
    imagecopyresampled($intermediate, $source, 0, 0, 0, 0,
                       $intermediate_width, $intermediate_height, $original_width, $original_height);
    
    // Then resize to final size
    $thumb3 = imagecreatetruecolor($new_width, $new_height);
    $white3b = imagecolorallocate($thumb3, 255, 255, 255);
    imagefill($thumb3, 0, 0, $white3b);
    imagealphablending($thumb3, true);
    
    imagecopyresampled($thumb3, $intermediate, 0, 0, 0, 0,
                       $new_width, $new_height, $intermediate_width, $intermediate_height);
    
    imagedestroy($intermediate);
} else {
    // Direct resize if target is >= 50%
    $thumb3 = imagecreatetruecolor($new_width, $new_height);
    $white3 = imagecolorallocate($thumb3, 255, 255, 255);
    imagefill($thumb3, 0, 0, $white3);
    imagealphablending($thumb3, true);
    
    imagecopyresampled($thumb3, $source, 0, 0, 0, 0,
                       $new_width, $new_height, $original_width, $original_height);
}

imagejpeg($thumb3, "{$output_dir}/method3_two_step.jpg", 98);
imagedestroy($thumb3);
echo "   → method3_two_step.jpg (progressive resize)\n";

// Test 4: Using imagescale (if available - PHP 5.5+)
echo "4. Testing imagescale...\n";
if (function_exists('imagescale')) {
    $thumb4 = imagescale($source, $new_width, $new_height, IMG_BICUBIC);
    if ($thumb4) {
        imagejpeg($thumb4, "{$output_dir}/method4_imagescale_bicubic.jpg", 98);
        imagedestroy($thumb4);
        echo "   → method4_imagescale_bicubic.jpg (PHP imagescale with bicubic)\n";
    } else {
        echo "   → imagescale failed\n";
    }
} else {
    echo "   → imagescale not available (PHP < 5.5)\n";
}

// Test 5: Lanczos-like resampling (manual implementation)
echo "5. Testing Lanczos-like resampling...\n";
$thumb5 = imagecreatetruecolor($new_width, $new_height);
$white5 = imagecolorallocate($thumb5, 255, 255, 255);
imagefill($thumb5, 0, 0, $white5);
imagealphablending($thumb5, true);

// Use imagecopyresampled but with pre-sharpening on source
$source_sharpened = imagecreatetruecolor($original_width, $original_height);
imagecopy($source_sharpened, $source, 0, 0, 0, 0, $original_width, $original_height);

if (function_exists('imagefilter')) {
    // Pre-sharpen source before resizing
    imagefilter($source_sharpened, IMG_FILTER_CONTRAST, -20); // Strong contrast
    imagefilter($source_sharpened, IMG_FILTER_SMOOTH, -3); // Strong sharpening
}

imagecopyresampled($thumb5, $source_sharpened, 0, 0, 0, 0,
                   $new_width, $new_height, $original_width, $original_height);

imagedestroy($source_sharpened);
imagejpeg($thumb5, "{$output_dir}/method5_presharpen.jpg", 98);
imagedestroy($thumb5);
echo "   → method5_presharpen.jpg (pre-sharpened source)\n";

// Test 6: High quality JPEG with maximum quality
echo "6. Testing maximum quality JPEG...\n";
$thumb6 = imagecreatetruecolor($new_width, $new_height);
$white6 = imagecolorallocate($thumb6, 255, 255, 255);
imagefill($thumb6, 0, 0, $white6);
imagealphablending($thumb6, true);

imagecopyresampled($thumb6, $source, 0, 0, 0, 0,
                   $new_width, $new_height, $original_width, $original_height);

imagejpeg($thumb6, "{$output_dir}/method6_max_quality.jpg", 100);
imagedestroy($thumb6);
echo "   → method6_max_quality.jpg (100% JPEG quality)\n";

// Test 7: PNG output (lossless)
echo "7. Testing PNG output...\n";
$thumb7 = imagecreatetruecolor($new_width, $new_height);
imagealphablending($thumb7, false);
imagesavealpha($thumb7, true);
$transparent = imagecolorallocatealpha($thumb7, 255, 255, 255, 0);
imagefill($thumb7, 0, 0, $transparent);
imagealphablending($thumb7, true);

imagecopyresampled($thumb7, $source, 0, 0, 0, 0,
                   $new_width, $new_height, $original_width, $original_height);

imagepng($thumb7, "{$output_dir}/method7_png_lossless.png", 0);
imagedestroy($thumb7);
echo "   → method7_png_lossless.png (PNG lossless)\n";

// Test 8: WebP output (if available)
echo "8. Testing WebP output...\n";
if (function_exists('imagewebp')) {
    $thumb8 = imagecreatetruecolor($new_width, $new_height);
    $white8 = imagecolorallocate($thumb8, 255, 255, 255);
    imagefill($thumb8, 0, 0, $white8);
    imagealphablending($thumb8, true);
    
    imagecopyresampled($thumb8, $source, 0, 0, 0, 0,
                       $new_width, $new_height, $original_width, $original_height);
    
    imagewebp($thumb8, "{$output_dir}/method8_webp.webp", 95);
    imagedestroy($thumb8);
    echo "   → method8_webp.webp (WebP 95% quality)\n";
} else {
    echo "   → WebP not available\n";
}

imagedestroy($source);

echo "\n=== Testing Complete ===\n";
echo "All thumbnails generated in: {$output_dir}/\n";
echo "\nRecommendation: Compare the visual quality of all methods\n";
echo "and check file sizes to find the best balance.\n";

// Generate comparison HTML
$html = "<!DOCTYPE html>
<html>
<head>
    <title>Thumbnail Quality Comparison</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .comparison { display: flex; flex-wrap: wrap; gap: 20px; }
        .method { border: 1px solid #ccc; padding: 10px; text-align: center; }
        .method img { max-width: 300px; border: 1px solid #ddd; }
        .info { margin-top: 10px; font-size: 12px; color: #666; }
    </style>
</head>
<body>
    <h1>Thumbnail Quality Comparison</h1>
    <p><strong>Original:</strong> {$original_width}x{$original_height} → <strong>Target:</strong> {$new_width}x{$new_height} ({$scale_ratio}%)</p>
    
    <div class='comparison'>";

$methods = [
    'method1_basic_resampled.jpg' => 'Basic Resampled (Current API)',
    'method2_enhanced_resampled.jpg' => 'Enhanced + Edge Enhance',
    'method3_two_step.jpg' => 'Two-Step Progressive',
    'method4_imagescale_bicubic.jpg' => 'PHP imagescale (Bicubic)',
    'method5_presharpen.jpg' => 'Pre-Sharpened Source',
    'method6_max_quality.jpg' => 'Maximum JPEG Quality',
    'method7_png_lossless.png' => 'PNG Lossless',
    'method8_webp.webp' => 'WebP Format'
];

foreach ($methods as $filename => $description) {
    $filepath = "{$output_dir}/{$filename}";
    if (file_exists($filepath)) {
        $filesize = round(filesize($filepath) / 1024, 1) . ' KB';
        $html .= "
        <div class='method'>
            <h3>{$description}</h3>
            <img src='{$filename}' alt='{$description}'>
            <div class='info'>File: {$filename}<br>Size: {$filesize}</div>
        </div>";
    }
}

$html .= "
    </div>
</body>
</html>";

file_put_contents("{$output_dir}/comparison.html", $html);
echo "Visual comparison available at: {$output_dir}/comparison.html\n";