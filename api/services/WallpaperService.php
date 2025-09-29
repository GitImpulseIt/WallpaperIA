<?php
require_once __DIR__ . '/../config/Config.php';

/**
 * Service pour la gestion des wallpapers et données CSV
 */
class WallpaperService {
    private $csv_file;
    private $category_ids;

    public function __construct() {
        $paths = Config::getPaths();
        $this->csv_file = $paths['csv_file'];
        $this->category_ids = Config::getCategoryIds();
    }

    /**
     * Lit et parse le fichier CSV
     */
    private function loadWallpapers() {
        $wallpapers = [];

        if (!file_exists($this->csv_file)) {
            throw new Exception('Wallpapers CSV file not found');
        }

        $handle = fopen($this->csv_file, 'r');
        if ($handle !== FALSE) {
            // Skip header row
            fgetcsv($handle, 1000, ",", '"', "\\");

            while (($data = fgetcsv($handle, 1000, ",", '"', "\\")) !== FALSE) {
                if (count($data) >= 4) { // category, filename, date, id
                    $wallpapers[] = [
                        'category' => trim($data[0]),
                        'filename' => trim($data[1]),
                        'date' => trim($data[2]),
                        'id' => trim($data[3])
                    ];
                } elseif (count($data) >= 2) { // Backward compatibility
                    $wallpapers[] = [
                        'category' => trim($data[0]),
                        'filename' => trim($data[1]),
                        'date' => null,
                        'id' => null
                    ];
                }
            }
            fclose($handle);
        }

        return $wallpapers;
    }

    /**
     * Obtient les catégories uniques avec leurs IDs
     */
    public function getCategories() {
        try {
            $wallpapers = $this->loadWallpapers();
            $categories = [];
            $found_categories = [];

            foreach ($wallpapers as $wallpaper) {
                $category = $wallpaper['category'];
                if (!in_array($category, $found_categories) && $category !== 'Unknown') {
                    $found_categories[] = $category;
                }
            }

            // Sort categories alphabetically
            sort($found_categories);

            // Build response with IDs
            foreach ($found_categories as $category) {
                $categories[] = [
                    'id' => $this->category_ids[$category] ?? strtolower(substr(str_replace(' ', '', $category), 0, 2)),
                    'name' => $category
                ];
            }

            return [
                'success' => true,
                'data' => $categories,
                'count' => count($categories)
            ];

        } catch (Exception $e) {
            return [
                'success' => false,
                'error' => $e->getMessage()
            ];
        }
    }

    /**
     * Obtient les wallpapers par catégorie et date (paramètres obligatoires)
     */
    public function getWallpapersByCategoryAndDate($category_id, $date) {
        try {
            $wallpapers = $this->loadWallpapers();
            $filtered = [];

            // Validate category ID
            $id_to_name = array_flip($this->category_ids);
            if (!isset($id_to_name[$category_id])) {
                return [
                    'success' => false,
                    'error' => 'Invalid category ID. Use /categories endpoint to get valid IDs.'
                ];
            }

            $target_category = $id_to_name[$category_id];

            // Validate date format (JJ/MM/AAAA)
            if (!preg_match('/^\d{2}\/\d{2}\/\d{4}$/', $date)) {
                return [
                    'success' => false,
                    'error' => 'Invalid date format. Use DD/MM/YYYY format (e.g., 29/09/2025).'
                ];
            }

            // Filter by category and date
            foreach ($wallpapers as $wallpaper) {
                if (strcasecmp($wallpaper['category'], $target_category) === 0 &&
                    $wallpaper['date'] === $date) {
                    $filtered[] = $wallpaper;
                }
            }

            return [
                'success' => true,
                'category' => $target_category,
                'category_id' => $category_id,
                'date' => $date,
                'data' => $filtered,
                'count' => count($filtered)
            ];

        } catch (Exception $e) {
            return [
                'success' => false,
                'error' => $e->getMessage()
            ];
        }
    }


    /**
     * Recherche un wallpaper par catégorie ID, date et ID
     */
    public function getWallpaperById($category_id, $date, $id) {
        try {
            $wallpapers = $this->loadWallpapers();

            // Convert category ID to name - ID obligatoire
            $id_to_name = array_flip($this->category_ids);
            if (!isset($id_to_name[$category_id])) {
                return [
                    'success' => false,
                    'error' => 'Invalid category ID. Use /categories endpoint to get valid IDs.'
                ];
            }
            $target_category = $id_to_name[$category_id];

            foreach ($wallpapers as $wallpaper) {
                if (strcasecmp($wallpaper['category'], $target_category) === 0 &&
                    $wallpaper['date'] === $date &&
                    $wallpaper['id'] === $id) {
                    return [
                        'success' => true,
                        'data' => $wallpaper
                    ];
                }
            }

            return [
                'success' => false,
                'error' => 'Wallpaper not found'
            ];

        } catch (Exception $e) {
            return [
                'success' => false,
                'error' => $e->getMessage()
            ];
        }
    }
}
?>