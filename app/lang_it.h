#ifndef LANG_IT_H
#define LANG_IT_H

// Informazioni dell'applicazione
#define APP_NAME "WallpaperAI"
#define APP_TITLE "WallpaperAI - Sfondi generati dall'IA"

// Schede
#define TAB_WALLPAPER "Sfondo"
#define TAB_CATEGORIES "Categorie"
#define TAB_SETTINGS "Impostazioni"

// Scheda Applicazione
#define BTN_CHANGE_NOW "🖼️ Cambia Ora"
#define LBL_CLICK_TO_CHANGE "Clicca per cambiare lo sfondo"
#define LBL_NEXT_CHANGE "Prossimo Cambio"
#define LBL_HISTORY "Cronologia"
#define BTN_APPLY "Applica"
#define BTN_PREV "◀"
#define BTN_NEXT "▶"
#define MSG_WALLPAPER_APPLIED "Sfondo applicato con successo!"
#define MSG_SAVE_ERROR "Errore durante il salvataggio dell'immagine."
#define MSG_LOAD_ERROR "Errore durante il caricamento dell'immagine."
#define MSG_DOWNLOAD_ERROR "Errore di download: "
#define MSG_NO_IMAGE_AVAILABLE "Nessuna immagine disponibile per questa categoria."
#define MSG_NO_IMAGE_7DAYS "Nessuna immagine trovata per questa categoria (ultimi 7 giorni)."
#define MSG_API_ERROR "Errore API: "
#define MSG_DOWNLOADING_FROM_API "Download dell'immagine dall'API..."
#define MSG_FETCHING_RANDOM_IMAGE "Recupero di un'immagine casuale..."
#define MSG_FETCHING_RANDOM_IMAGES "Recupero di %1 immagini casuali..."
#define MSG_CHANGING_IN_PROGRESS "🔄 Cambio in corso..."
#define MSG_APPLYING_WALLPAPERS "Applicazione degli sfondi..."
#define MSG_APPLYING_WALLPAPER "Applicazione dello sfondo..."
#define MSG_CONNECTION_ERROR "Errore di connessione sfondi per schermo %1: %2"

// Scheda Categorie
#define MSG_NO_THUMBNAIL "Nessuna miniatura disponibile per questa categoria."
#define LBL_THUMBNAIL "Miniatura\n"
#define BTN_APPLY_THUMBNAIL "Applica"

// Scheda Impostazioni - Gruppi
#define GRP_FREQUENCY "Frequenza di cambio"
#define GRP_ADJUSTMENT "Modalità di adattamento dell'immagine"
#define GRP_SYSTEM_OPTIONS "Opzioni di sistema"

// Scheda Impostazioni - Frequenza
#define LBL_FREQUENCY "Frequenza di cambio"
#define FREQ_MANUAL "Solo cambio manuale"
#define FREQ_1H "Ogni ora"
#define FREQ_3H "Ogni 3 ore"
#define FREQ_6H "Ogni 6 ore"
#define FREQ_12H "Ogni 12 ore"
#define FREQ_24H "Ogni 24 ore"
#define FREQ_7D "Ogni settimana"
#define FREQ_STARTUP "All'avvio del computer"
#define FREQ_CUSTOM "Altro"

// Scheda Impostazioni - Unità personalizzate
#define UNIT_MINUTES "minuti"
#define UNIT_HOURS "ore"
#define UNIT_DAYS "giorni"

// Scheda Impostazioni - Opzioni di avvio
#define LBL_STARTUP_WINDOWS "Avvia con Windows"
#define LBL_CHANGE_ON_STARTUP "Cambia lo sfondo\nall'avvio del computer"

// Scheda Impostazioni - Multi-schermi
#define LBL_MULTI_SCREEN "Immagine diversa su ogni schermo"
#define LBL_SCREEN_NAME "Schermo %1"

// Scheda Impostazioni - Adattamento
#define LBL_ADJUSTMENT "Adattamento dell'immagine"
#define ADJ_FILL "Riempi"
#define ADJ_FIT "Adatta"
#define ADJ_SPAN "Espandi"
#define ADJ_STRETCH "Allunga"
#define ADJ_TILE "Affianca"

// Spiegazioni delle modalità di adattamento
#define EXPL_FILL "L'immagine mantiene le proporzioni ma viene ritagliata per riempire lo schermo"
#define EXPL_FIT "L'immagine mantiene le proporzioni ma vengono aggiunte bande nere per adattarsi allo schermo"
#define EXPL_SPAN "L'immagine si estende su più schermi"
#define EXPL_STRETCH "L'immagine viene allungata per riempire tutto lo schermo ma le proporzioni non vengono rispettate"
#define EXPL_TILE "L'immagine si ripete per riempire tutto lo schermo e le proporzioni vengono conservate"
#define EXPL_DEFAULT "Modalità di adattamento selezionata"

// Selettore di schermo
#define MSG_SCREENS_SELECTED " schermo/i selezionato/i"
#define MSG_ALL_SCREENS "Tutti gli schermi"
#define MSG_CANNOT_DESELECT "Impossibile deselezionare tutti gli schermi. Almeno uno schermo deve essere selezionato."
#define MSG_SCREEN_SELECTED "Schermo selezionato: "
#define MSG_SCREENS_SELECTED_PLURAL "Schermi selezionati: "
#define MSG_NO_SCREEN_SELECTED "Nessuno schermo selezionato\nSeleziona almeno uno schermo"
#define MSG_SCREEN_NO_HISTORY_TITLE "Schermo senza cronologia"
#define MSG_SCREEN_NO_HISTORY "Lo schermo %1 non può essere deselezionato perché non ha ancora una cronologia di sfondi.\n\nApplica prima almeno uno sfondo a questo schermo per poterlo deselezionare."
#define MSG_WALLPAPER_APPLIED_SCREENS "Lo sfondo verrà applicato su %1 schermi selezionati."

// Widget di conto alla rovescia
#define LBL_NEVER_MODE "Solo cambio manuale"
#define LBL_STARTUP_MODE "All'avvio del computer"
#define INFO_STARTUP "Cambio sfondo al prossimo riavvio del computer"
#define INFO_MANUAL "Solo cambio manuale dello sfondo"
#define TIME_DAYS "g"
#define TIME_HOURS "h"
#define TIME_MINUTES "m"
#define TIME_SECONDS "s"

// Avviso errore API
#define ERR_API_TITLE "Errore nel recupero delle categorie"
#define ERR_RETRY_IN "Nuovo tentativo tra "
#define ERR_CHECK_CONNECTION "Verifica la tua connessione internet e le impostazioni di rete."

// Barra delle applicazioni
#define TRAY_SHOW "Mostra"
#define TRAY_CHANGE_WALLPAPER "Cambia sfondo"
#define TRAY_QUIT "Esci"

// Modalità di attivazione
#define MODE_STARTUP "Avvio"
#define MODE_AUTOMATIC "Automatico"
#define MODE_MANUAL "Manuale"

// Finestra di dialogo applicazione rapida di categoria
#define DIALOG_APPLY_TITLE "Applica sfondo"
#define DIALOG_CATEGORY "Categoria: "
#define DIALOG_TARGET_SCREEN "Schermo di destinazione:"
#define DIALOG_SCREEN "Schermo %1"
#define DIALOG_APPLY_THIS_IMAGE "Applica questa immagine"
#define DIALOG_APPLY_RANDOM_IMAGE "Applica un'immagine di questa categoria"
#define DIALOG_CLOSE "Chiudi"

// Messaggi di cronologia
#define MSG_HISTORY_SELECTION_REQUIRED_TITLE "Selezione richiesta"
#define MSG_HISTORY_SELECTION_REQUIRED "Seleziona un'immagine dalla cronologia."
#define MSG_MULTISCREEN_WARNING_TITLE "Applicazione multi-schermo"
#define MSG_MULTISCREEN_CONTINUE "Vuoi continuare?"
#define MSG_MULTISCREEN_DONT_SHOW "Non mostrare più questo avviso"

// Banner errore API
#define ERR_BANNER_TITLE "Errore nel recupero delle categorie"
#define ERR_BANNER_RETRY "Nuovo tentativo tra "
#define ERR_BANNER_CHECK_CONNECTION "Verifica la tua connessione internet e le impostazioni di rete."
#define ERR_BANNER_BTN_DETAILS "Dettagli"

#endif // LANG_IT_H
