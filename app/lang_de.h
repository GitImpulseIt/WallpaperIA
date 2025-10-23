#ifndef LANG_DE_H
#define LANG_DE_H

// Anwendungsinformationen
#define APP_NAME "WallpaperAI"
#define APP_TITLE "WallpaperAI - KI-generierte Hintergrundbilder"

// Registerkarten
#define TAB_WALLPAPER "Hintergrundbild"
#define TAB_CATEGORIES "Kategorien"
#define TAB_SETTINGS "Einstellungen"

// Registerkarte Anwendung
#define BTN_CHANGE_NOW "🖼️ Jetzt Wechseln"
#define LBL_CLICK_TO_CHANGE "Klicken Sie, um das Hintergrundbild zu wechseln"
#define LBL_NEXT_CHANGE "Nächster Wechsel"
#define LBL_HISTORY "Verlauf"
#define BTN_APPLY "Anwenden"
#define BTN_PREV "◀"
#define BTN_NEXT "▶"
#define MSG_WALLPAPER_APPLIED "Hintergrundbild erfolgreich angewendet!"
#define MSG_SAVE_ERROR "Fehler beim Speichern des Bildes."
#define MSG_LOAD_ERROR "Fehler beim Laden des Bildes."
#define MSG_DOWNLOAD_ERROR "Download-Fehler: "
#define MSG_NO_IMAGE_AVAILABLE "Kein Bild für diese Kategorie verfügbar."
#define MSG_NO_IMAGE_7DAYS "Kein Bild für diese Kategorie gefunden (letzte 7 Tage)."
#define MSG_API_ERROR "API-Fehler: "
#define MSG_DOWNLOADING_FROM_API "Bild wird von der API heruntergeladen..."
#define MSG_FETCHING_RANDOM_IMAGE "Zufälliges Bild wird abgerufen..."
#define MSG_FETCHING_RANDOM_IMAGES "%1 zufällige Bilder werden abgerufen..."
#define MSG_CHANGING_IN_PROGRESS "🔄 Wechsel läuft..."
#define MSG_APPLYING_WALLPAPERS "Hintergrundbilder werden angewendet..."
#define MSG_APPLYING_WALLPAPER "Hintergrundbild wird angewendet..."
#define MSG_CONNECTION_ERROR "Verbindungsfehler Hintergrundbilder für Bildschirm %1: %2"

// Registerkarte Kategorien
#define MSG_NO_THUMBNAIL "Kein Vorschaubild für diese Kategorie verfügbar."
#define LBL_THUMBNAIL "Vorschau\n"
#define BTN_APPLY_THUMBNAIL "Anwenden"

// Registerkarte Einstellungen - Gruppen
#define GRP_FREQUENCY "Wechselhäufigkeit"
#define GRP_ADJUSTMENT "Bildanpassungsmodus"
#define GRP_SYSTEM_OPTIONS "Systemoptionen"

// Registerkarte Einstellungen - Häufigkeit
#define LBL_FREQUENCY "Wechselhäufigkeit"
#define FREQ_MANUAL "Nur manueller Wechsel"
#define FREQ_1H "Jede Stunde"
#define FREQ_3H "Alle 3 Stunden"
#define FREQ_6H "Alle 6 Stunden"
#define FREQ_12H "Alle 12 Stunden"
#define FREQ_24H "Alle 24 Stunden"
#define FREQ_7D "Jede Woche"
#define FREQ_STARTUP "Beim Computerstart"
#define FREQ_CUSTOM "Andere"

// Registerkarte Einstellungen - Benutzerdefinierte Einheiten
#define UNIT_MINUTES "Minuten"
#define UNIT_HOURS "Stunden"
#define UNIT_DAYS "Tage"

// Registerkarte Einstellungen - Startoptionen
#define LBL_STARTUP_WINDOWS "Mit Windows starten"
#define LBL_CHANGE_ON_STARTUP "Hintergrundbild beim\nComputerstart wechseln"

// Registerkarte Einstellungen - Mehrere Bildschirme
#define LBL_MULTI_SCREEN "Anderes Bild auf jedem Bildschirm"
#define LBL_SCREEN_NAME "Bildschirm %1"

// Registerkarte Einstellungen - Anpassung
#define LBL_ADJUSTMENT "Bildanpassung"
#define ADJ_FILL "Füllen"
#define ADJ_FIT "Anpassen"
#define ADJ_SPAN "Überspannen"
#define ADJ_STRETCH "Strecken"
#define ADJ_TILE "Kachel"

// Erklärungen der Anpassungsmodi
#define EXPL_FILL "Das Bild behält sein Seitenverhältnis, wird aber zugeschnitten, um den Bildschirm zu füllen"
#define EXPL_FIT "Das Bild behält sein Seitenverhältnis, aber schwarze Balken werden hinzugefügt, um es an den Bildschirm anzupassen"
#define EXPL_SPAN "Das Bild erstreckt sich über mehrere Bildschirme"
#define EXPL_STRETCH "Das Bild wird gestreckt, um den gesamten Bildschirm zu füllen, aber das Seitenverhältnis wird nicht respektiert"
#define EXPL_TILE "Das Bild wiederholt sich, um den gesamten Bildschirm zu füllen, und das Seitenverhältnis bleibt erhalten"
#define EXPL_DEFAULT "Ausgewählter Anpassungsmodus"

// Bildschirmauswahl
#define MSG_SCREENS_SELECTED " Bildschirm(e) ausgewählt"
#define MSG_ALL_SCREENS "Alle Bildschirme"
#define MSG_CANNOT_DESELECT "Alle Bildschirme können nicht abgewählt werden. Mindestens ein Bildschirm muss ausgewählt sein."
#define MSG_SCREEN_SELECTED "Ausgewählter Bildschirm: "
#define MSG_SCREENS_SELECTED_PLURAL "Ausgewählte Bildschirme: "
#define MSG_NO_SCREEN_SELECTED "Kein Bildschirm ausgewählt\nWählen Sie mindestens einen Bildschirm aus"
#define MSG_SCREEN_NO_HISTORY_TITLE "Bildschirm ohne Verlauf"
#define MSG_SCREEN_NO_HISTORY "Der Bildschirm %1 kann nicht abgewählt werden, da er noch keinen Hintergrundbildverlauf hat.\n\nWenden Sie zuerst mindestens ein Hintergrundbild auf diesen Bildschirm an, um ihn abwählen zu können."
#define MSG_WALLPAPER_APPLIED_SCREENS "Das Hintergrundbild wird auf %1 ausgewählte Bildschirme angewendet."

// Countdown-Widget
#define LBL_NEVER_MODE "Nur manueller Wechsel"
#define LBL_STARTUP_MODE "Beim Computerstart"
#define INFO_STARTUP "Hintergrundbild beim nächsten Computerneustart wechseln"
#define INFO_MANUAL "Nur manueller Hintergrundbildwechsel"
#define TIME_DAYS "T"
#define TIME_HOURS "Std"
#define TIME_MINUTES "Min"
#define TIME_SECONDS "Sek"

// API-Fehlerwarnung
#define ERR_API_TITLE "Fehler beim Abrufen der Kategorien"
#define ERR_RETRY_IN "Neuer Versuch in "
#define ERR_CHECK_CONNECTION "Überprüfen Sie Ihre Internetverbindung und Netzwerkeinstellungen."

// Systemablage
#define TRAY_SHOW "Anzeigen"
#define TRAY_CHANGE_WALLPAPER "Hintergrundbild wechseln"
#define TRAY_QUIT "Beenden"

// Auslösemodi
#define MODE_STARTUP "Start"
#define MODE_AUTOMATIC "Automatisch"
#define MODE_MANUAL "Manuell"

// Schnellanwendungsdialog für Kategorien
#define DIALOG_APPLY_TITLE "Hintergrundbild anwenden"
#define DIALOG_CATEGORY "Kategorie: "
#define DIALOG_TARGET_SCREEN "Zielbildschirm:"
#define DIALOG_SCREEN "Bildschirm %1"
#define DIALOG_APPLY_THIS_IMAGE "Dieses Bild anwenden"
#define DIALOG_APPLY_RANDOM_IMAGE "Ein Bild aus dieser Kategorie anwenden"
#define DIALOG_CLOSE "Schließen"

// Verlaufsmeldungen
#define MSG_HISTORY_SELECTION_REQUIRED_TITLE "Auswahl erforderlich"
#define MSG_HISTORY_SELECTION_REQUIRED "Bitte wählen Sie ein Bild aus dem Verlauf aus."
#define MSG_MULTISCREEN_WARNING_TITLE "Mehrbildschirm-Anwendung"
#define MSG_MULTISCREEN_CONTINUE "Möchten Sie fortfahren?"
#define MSG_MULTISCREEN_DONT_SHOW "Diese Warnung nicht mehr anzeigen"

// API-Fehlerbanner
#define ERR_BANNER_TITLE "Fehler beim Abrufen der Kategorien"
#define ERR_BANNER_RETRY "Neuer Versuch in "
#define ERR_BANNER_CHECK_CONNECTION "Überprüfen Sie Ihre Internetverbindung und Netzwerkeinstellungen."
#define ERR_BANNER_BTN_DETAILS "Details"

#endif // LANG_DE_H
