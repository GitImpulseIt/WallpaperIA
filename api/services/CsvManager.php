<?php
/**
 * Service de gestion du fichier wallpapers.csv
 * Permet l'ajout d'entrées dans le CSV
 */
class CsvManager {
    private $csvFile;

    public function __construct() {
        $this->csvFile = __DIR__ . '/../wallpapers.csv';
    }

    /**
     * Échappe les caractères dangereux pour CSV Injection
     * @param string $value
     * @return string
     */
    private function escapeCsvValue($value) {
        // Supprimer les formules potentiellement dangereuses
        // Les formules commencent par =, +, -, @, tab, ou retour chariot
        $dangerous_chars = ['=', '+', '-', '@', "\t", "\r"];

        // Si la valeur commence par un caractère dangereux, préfixer avec '
        if (in_array(substr($value, 0, 1), $dangerous_chars)) {
            $value = "'" . $value;
        }

        // Échapper les guillemets doubles en les doublant (norme CSV)
        $value = str_replace('"', '""', $value);

        // Si la valeur contient une virgule, un guillemet ou un retour à la ligne, l'entourer de guillemets
        if (preg_match('/[,"\n\r]/', $value)) {
            $value = '"' . $value . '"';
        }

        return $value;
    }

    /**
     * Ajoute une nouvelle entrée au fichier CSV
     * @param string $category Nom de la catégorie
     * @param string $filename Nom du fichier (format flexible, plus seulement SHA256)
     * @param string $date Date au format DD/MM/YYYY
     * @return array ['success' => bool, 'id' => int|null, 'error' => string|null]
     */
    public function addEntry($category, $filename, $date) {
        // Valider les paramètres
        $validation = $this->validateEntry($category, $filename, $date);
        if (!$validation['success']) {
            return $validation;
        }

        // Vérifier que le fichier CSV existe
        if (!file_exists($this->csvFile)) {
            return [
                'success' => false,
                'id' => null,
                'error' => 'CSV file not found'
            ];
        }

        // Calculer le prochain ID pour cette catégorie/date
        $nextId = $this->getNextId($category, $date);

        // Ouvrir le fichier en mode append
        $handle = fopen($this->csvFile, 'a');
        if ($handle === false) {
            return [
                'success' => false,
                'id' => null,
                'error' => 'Cannot open CSV file for writing'
            ];
        }

        // Utiliser fputcsv pour un échappement correct et sécurisé
        // Format: category,filename,date,id
        $result = fputcsv($handle, [$category, $filename, $date, $nextId], ',', '"', '\\');

        fclose($handle);

        if ($result === false) {
            return [
                'success' => false,
                'id' => null,
                'error' => 'Failed to write to CSV file'
            ];
        }

        return [
            'success' => true,
            'id' => $nextId,
            'error' => null,
            'entry' => [
                'category' => $category,
                'filename' => $filename,
                'date' => $date,
                'id' => $nextId
            ]
        ];
    }

    /**
     * Valide les données d'une entrée
     * @param string $category
     * @param string $filename
     * @param string $date
     * @return array ['success' => bool, 'error' => string|null]
     */
    private function validateEntry($category, $filename, $date) {
        // Valider la catégorie (non vide)
        if (empty($category)) {
            return [
                'success' => false,
                'error' => 'Category is required'
            ];
        }

        // Détecter les tentatives de CSV Injection dans la catégorie
        $dangerous_chars = ['=', '+', '-', '@', "\t", "\r", "\n"];
        if (in_array(substr($category, 0, 1), $dangerous_chars)) {
            return [
                'success' => false,
                'error' => 'Category cannot start with formula characters (=, +, -, @)'
            ];
        }

        // Valider le filename (non vide, doit avoir une extension)
        if (empty($filename)) {
            return [
                'success' => false,
                'error' => 'Filename is required'
            ];
        }

        // Détecter les tentatives de CSV Injection dans le filename
        if (in_array(substr($filename, 0, 1), $dangerous_chars)) {
            return [
                'success' => false,
                'error' => 'Filename cannot start with formula characters (=, +, -, @)'
            ];
        }

        // Vérifier que le filename a une extension
        if (pathinfo($filename, PATHINFO_EXTENSION) === '') {
            return [
                'success' => false,
                'error' => 'Filename must have an extension (e.g., .png, .jpg)'
            ];
        }

        // Valider le format de la date (DD/MM/YYYY)
        if (!preg_match('/^\d{2}\/\d{2}\/\d{4}$/', $date)) {
            return [
                'success' => false,
                'error' => 'Date must be in DD/MM/YYYY format'
            ];
        }

        // Valider que la date est valide
        $parts = explode('/', $date);
        if (!checkdate((int)$parts[1], (int)$parts[0], (int)$parts[2])) {
            return [
                'success' => false,
                'error' => 'Invalid date'
            ];
        }

        return [
            'success' => true,
            'error' => null
        ];
    }

    /**
     * Calcule le prochain ID pour une catégorie/date donnée
     * @param string $category
     * @param string $date
     * @return int
     */
    private function getNextId($category, $date) {
        $maxId = 0;

        // Ouvrir le fichier CSV en lecture
        $handle = fopen($this->csvFile, 'r');
        if ($handle === false) {
            return 1; // Premier ID si le fichier n'existe pas ou n'est pas lisible
        }

        // Ignorer la ligne d'en-tête
        fgetcsv($handle, 0, ',', '"', '\\');

        // Parcourir les lignes
        while (($row = fgetcsv($handle, 0, ',', '"', '\\')) !== false) {
            if (count($row) >= 4) {
                $rowCategory = $row[0];
                $rowDate = $row[2];
                $rowId = (int)$row[3];

                // Si même catégorie et même date, suivre le max ID
                if ($rowCategory === $category && $rowDate === $date) {
                    $maxId = max($maxId, $rowId);
                }
            }
        }

        fclose($handle);

        return $maxId + 1;
    }

    /**
     * Vérifie si une entrée existe déjà
     * @param string $category
     * @param string $filename
     * @param string $date
     * @return bool
     */
    public function entryExists($category, $filename, $date) {
        $handle = fopen($this->csvFile, 'r');
        if ($handle === false) {
            return false;
        }

        // Ignorer la ligne d'en-tête
        fgetcsv($handle, 0, ',', '"', '\\');

        // Parcourir les lignes
        while (($row = fgetcsv($handle, 0, ',', '"', '\\')) !== false) {
            if (count($row) >= 3) {
                if ($row[0] === $category && $row[1] === $filename && $row[2] === $date) {
                    fclose($handle);
                    return true;
                }
            }
        }

        fclose($handle);
        return false;
    }
}
?>
