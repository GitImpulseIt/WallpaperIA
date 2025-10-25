#ifndef CONFIG_H
#define CONFIG_H

// ================================
// Configuration de l'application
// ================================

// URL de base de l'API REST
// Modifier cette valeur pour pointer vers une autre instance de l'API
#define API_BASE_URL "https://kazflow.com"

// Endpoints disponibles :
// - GET /categories           : Liste des catégories
// - GET /wallpapers           : Wallpapers par catégorie/date
// - GET /get/{filename}       : Téléchargement de fichier
// - GET /mini/{filename}      : Miniature (204x115px)

#endif // CONFIG_H
