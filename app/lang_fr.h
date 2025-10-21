#ifndef LANG_FR_H
#define LANG_FR_H

// Informations de l'application
#define APP_NAME "WallpaperAI"
#define APP_TITLE "WallpaperAI - Fonds d'écrans générés par IA"

// Onglets
#define TAB_WALLPAPER "Fond d'écran"
#define TAB_CATEGORIES "Catégories"
#define TAB_SETTINGS "Paramètres"

// Onglet Application
#define BTN_CHANGE_NOW "🖼️ Changer Maintenant"
#define LBL_CLICK_TO_CHANGE "Cliquez pour changer le fond d'écran"
#define LBL_NEXT_CHANGE "Prochain Changement"
#define LBL_HISTORY "Historique"
#define BTN_APPLY "Appliquer"
#define BTN_PREV "◀"
#define BTN_NEXT "▶"
#define MSG_WALLPAPER_APPLIED "Fond d'écran appliqué avec succès !"
#define MSG_SAVE_ERROR "Erreur lors de la sauvegarde de l'image."
#define MSG_LOAD_ERROR "Erreur lors du chargement de l'image."
#define MSG_DOWNLOAD_ERROR "Erreur de téléchargement : "
#define MSG_NO_IMAGE_AVAILABLE "Aucune image disponible pour cette catégorie."
#define MSG_NO_IMAGE_7DAYS "Aucune image trouvée pour cette catégorie (7 derniers jours)."
#define MSG_API_ERROR "Erreur API: "
#define MSG_DOWNLOADING_FROM_API "Téléchargement de l'image depuis l'API..."
#define MSG_FETCHING_RANDOM_IMAGE "Récupération d'une image aléatoire..."
#define MSG_FETCHING_RANDOM_IMAGES "Récupération de %1 images aléatoires..."
#define MSG_CONNECTION_ERROR "Erreur de connexion wallpapers pour écran %1: %2"

// Onglet Catégories
#define MSG_NO_THUMBNAIL "Aucune miniature disponible pour cette catégorie."
#define LBL_THUMBNAIL "Miniature\n"
#define BTN_APPLY_THUMBNAIL "Appliquer"

// Onglet Paramètres - Groupes
#define GRP_FREQUENCY "Fréquence de changement"
#define GRP_ADJUSTMENT "Mode d'ajustement de l'image"
#define GRP_SYSTEM_OPTIONS "Options système"

// Onglet Paramètres - Fréquence
#define LBL_FREQUENCY "Fréquence de changement"
#define FREQ_MANUAL "Changement manuel uniquement"
#define FREQ_1H "Toutes les heures"
#define FREQ_3H "Toutes les 3 heures"
#define FREQ_6H "Toutes les 6 heures"
#define FREQ_12H "Toutes les 12 heures"
#define FREQ_24H "Toutes les 24 heures"
#define FREQ_7D "Toutes les semaines"
#define FREQ_STARTUP "Au démarrage de l'ordinateur"
#define FREQ_CUSTOM "Autre"

// Onglet Paramètres - Unités personnalisées
#define UNIT_MINUTES "minutes"
#define UNIT_HOURS "heures"
#define UNIT_DAYS "jours"

// Onglet Paramètres - Options de démarrage
#define LBL_STARTUP_WINDOWS "Démarrer avec Windows"
#define LBL_CHANGE_ON_STARTUP "Changer le fond d'écran\nau démarrage de l'ordinateur"

// Onglet Paramètres - Multi-écrans
#define LBL_MULTI_SCREEN "Image différente sur chaque écran"

// Onglet Paramètres - Ajustement
#define LBL_ADJUSTMENT "Ajustement de l'image"
#define ADJ_FILL "Remplir"
#define ADJ_FIT "Ajuster"
#define ADJ_SPAN "Étendre"
#define ADJ_STRETCH "Étirer"
#define ADJ_TILE "Mosaïque"

// Explications des modes d'ajustement
#define EXPL_FILL "L'image conserve son ratio mais dépasse de l'écran pour le remplir"
#define EXPL_FIT "L'image conserve son ratio mais des bandes noires sont ajoutées pour tenir dans l'écran"
#define EXPL_SPAN "L'image s'étend sur plusieurs écrans"
#define EXPL_STRETCH "L'image est étirée de façon à remplir tout l'écran mais le ratio n'est pas respecté"
#define EXPL_TILE "L'image se répète de façon à remplir tout l'écran et le ratio est conservé"
#define EXPL_DEFAULT "Mode d'ajustement sélectionné"

// Sélecteur d'écran
#define MSG_SCREENS_SELECTED " écran(s) sélectionné(s)"
#define MSG_ALL_SCREENS "Tous les écrans"
#define MSG_CANNOT_DESELECT "Impossible de désélectionner tous les écrans. Au moins un écran doit être sélectionné."
#define MSG_SCREEN_SELECTED "Écran sélectionné : "
#define MSG_SCREENS_SELECTED_PLURAL "Écrans sélectionnés : "
#define MSG_NO_SCREEN_SELECTED "Aucun écran sélectionné\nSélectionnez au moins un écran"
#define MSG_SCREEN_NO_HISTORY_TITLE "Écran sans historique"
#define MSG_SCREEN_NO_HISTORY "L'écran %1 ne peut pas être désélectionné car il n'a pas encore d'historique de fonds d'écran.\n\nAppliquez d'abord au moins un fond d'écran sur cet écran pour pouvoir le désélectionner."
#define MSG_WALLPAPER_APPLIED_SCREENS "Le fond d'écran sera appliqué sur %1 écrans sélectionnés."

// Widget de compte à rebours
#define LBL_NEVER_MODE "Changement manuel uniquement"
#define LBL_STARTUP_MODE "Au démarrage de l'ordinateur"
#define INFO_STARTUP "Changement de fond d'écran au prochain redémarrage de l'ordinateur"
#define INFO_MANUAL "Changement de fond d'écran manuel seulement"
#define TIME_DAYS "j"
#define TIME_HOURS "h"
#define TIME_MINUTES "m"
#define TIME_SECONDS "s"

// Alerte d'erreur API
#define ERR_API_TITLE "Erreur lors de la récupération des catégories"
#define ERR_RETRY_IN "Nouvelle tentative dans "
#define ERR_CHECK_CONNECTION "Vérifiez votre connexion internet et vos paramètres réseau."

// System Tray
#define TRAY_SHOW "Afficher"
#define TRAY_CHANGE_WALLPAPER "Changer le fond d'écran"
#define TRAY_QUIT "Quitter"

// Modes de déclenchement
#define MODE_STARTUP "Démarrage"
#define MODE_AUTOMATIC "Automatique"
#define MODE_MANUAL "Manuel"

// Dialogue d'application rapide de catégorie
#define DIALOG_APPLY_TITLE "Appliquer un fond d'écran"
#define DIALOG_CATEGORY "Catégorie : "
#define DIALOG_TARGET_SCREEN "Écran cible :"
#define DIALOG_SCREEN "Écran %1"
#define DIALOG_APPLY_THIS_IMAGE "Appliquer cette image"
#define DIALOG_APPLY_RANDOM_IMAGE "Appliquer une image de cette catégorie"
#define DIALOG_CLOSE "Fermer"

// Messages d'historique
#define MSG_HISTORY_SELECTION_REQUIRED_TITLE "Sélection requise"
#define MSG_HISTORY_SELECTION_REQUIRED "Veuillez sélectionner une image de l'historique."
#define MSG_MULTISCREEN_WARNING_TITLE "Application multi-écrans"
#define MSG_MULTISCREEN_CONTINUE "Voulez-vous continuer ?"
#define MSG_MULTISCREEN_DONT_SHOW "Ne plus afficher cet avertissement"

// Bandeau d'erreur API
#define ERR_BANNER_TITLE "Erreur lors de la récupération des catégories"
#define ERR_BANNER_RETRY "Nouvelle tentative dans "
#define ERR_BANNER_CHECK_CONNECTION "Vérifiez votre connexion internet et vos paramètres réseau."
#define ERR_BANNER_BTN_DETAILS "Détails"

#endif // LANG_FR_H
