#ifndef LANG_ES_H
#define LANG_ES_H

// Informaciones de la aplicación
#define APP_NAME "WallpaperAI"
#define APP_TITLE "WallpaperAI - Fondos de pantalla generados por IA"

// Pestañas
#define TAB_WALLPAPER "Fondo de pantalla"
#define TAB_CATEGORIES "Categorías"
#define TAB_SETTINGS "Configuración"

// Pestaña Aplicación
#define BTN_CHANGE_NOW "🖼️ Cambiar Ahora"
#define LBL_CLICK_TO_CHANGE "Haz clic para cambiar el fondo de pantalla"
#define LBL_NEXT_CHANGE "Próximo Cambio"
#define LBL_HISTORY "Historial"
#define BTN_APPLY "Aplicar"
#define BTN_PREV "◀"
#define BTN_NEXT "▶"
#define MSG_WALLPAPER_APPLIED "¡Fondo de pantalla aplicado con éxito!"
#define MSG_SAVE_ERROR "Error al guardar la imagen."
#define MSG_LOAD_ERROR "Error al cargar la imagen."
#define MSG_DOWNLOAD_ERROR "Error de descarga: "
#define MSG_NO_IMAGE_AVAILABLE "No hay imagen disponible para esta categoría."
#define MSG_NO_IMAGE_7DAYS "No se encontró imagen para esta categoría (últimos 7 días)."
#define MSG_API_ERROR "Error de API: "
#define MSG_DOWNLOADING_FROM_API "Descargando imagen desde la API..."
#define MSG_FETCHING_RANDOM_IMAGE "Obteniendo una imagen aleatoria..."
#define MSG_FETCHING_RANDOM_IMAGES "Obteniendo %1 imágenes aleatorias..."
#define MSG_CHANGING_IN_PROGRESS "🔄 Cambio en progreso..."
#define MSG_APPLYING_WALLPAPERS "Aplicando fondos de pantalla..."
#define MSG_APPLYING_WALLPAPER "Aplicando fondo de pantalla..."
#define MSG_CONNECTION_ERROR "Error de conexión fondos de pantalla para pantalla %1: %2"

// Pestaña Categorías
#define MSG_NO_THUMBNAIL "No hay miniatura disponible para esta categoría."
#define LBL_THUMBNAIL "Miniatura\n"
#define BTN_APPLY_THUMBNAIL "Aplicar"

// Pestaña Configuración - Grupos
#define GRP_FREQUENCY "Frecuencia de cambio"
#define GRP_ADJUSTMENT "Modo de ajuste de imagen"
#define GRP_SYSTEM_OPTIONS "Opciones del sistema"

// Pestaña Configuración - Frecuencia
#define LBL_FREQUENCY "Frecuencia de cambio"
#define FREQ_MANUAL "Solo cambio manual"
#define FREQ_1H "Cada hora"
#define FREQ_3H "Cada 3 horas"
#define FREQ_6H "Cada 6 horas"
#define FREQ_12H "Cada 12 horas"
#define FREQ_24H "Cada 24 horas"
#define FREQ_7D "Cada semana"
#define FREQ_STARTUP "Al iniciar la computadora"
#define FREQ_CUSTOM "Otro"

// Pestaña Configuración - Unidades personalizadas
#define UNIT_MINUTES "minutos"
#define UNIT_HOURS "horas"
#define UNIT_DAYS "días"

// Pestaña Configuración - Opciones de inicio
#define LBL_STARTUP_WINDOWS "Iniciar con Windows"
#define LBL_CHANGE_ON_STARTUP "Cambiar el fondo de pantalla\nal iniciar la computadora"

// Pestaña Configuración - Multi-pantallas
#define LBL_MULTI_SCREEN "Imagen diferente en cada pantalla"
#define LBL_SCREEN_NAME "Pantalla %1"

// Pestaña Configuración - Ajuste
#define LBL_ADJUSTMENT "Ajuste de imagen"
#define ADJ_FILL "Rellenar"
#define ADJ_FIT "Ajustar"
#define ADJ_SPAN "Expandir"
#define ADJ_STRETCH "Estirar"
#define ADJ_TILE "Mosaico"

// Explicaciones de los modos de ajuste
#define EXPL_FILL "La imagen mantiene su relación de aspecto pero se recorta para llenar la pantalla"
#define EXPL_FIT "La imagen mantiene su relación de aspecto pero se añaden bandas negras para ajustarse a la pantalla"
#define EXPL_SPAN "La imagen se expande a través de múltiples pantallas"
#define EXPL_STRETCH "La imagen se estira para llenar toda la pantalla pero la relación de aspecto no se respeta"
#define EXPL_TILE "La imagen se repite para llenar toda la pantalla y la relación de aspecto se conserva"
#define EXPL_DEFAULT "Modo de ajuste seleccionado"

// Selector de pantalla
#define MSG_SCREENS_SELECTED " pantalla(s) seleccionada(s)"
#define MSG_ALL_SCREENS "Todas las pantallas"
#define MSG_CANNOT_DESELECT "No se puede deseleccionar todas las pantallas. Al menos una pantalla debe estar seleccionada."
#define MSG_SCREEN_SELECTED "Pantalla seleccionada: "
#define MSG_SCREENS_SELECTED_PLURAL "Pantallas seleccionadas: "
#define MSG_NO_SCREEN_SELECTED "Ninguna pantalla seleccionada\nSeleccione al menos una pantalla"
#define MSG_SCREEN_NO_HISTORY_TITLE "Pantalla sin historial"
#define MSG_SCREEN_NO_HISTORY "La pantalla %1 no se puede deseleccionar porque aún no tiene historial de fondos de pantalla.\n\nPrimero aplique al menos un fondo de pantalla a esta pantalla para poder deseleccionarla."
#define MSG_WALLPAPER_APPLIED_SCREENS "El fondo de pantalla se aplicará en %1 pantallas seleccionadas."

// Widget de cuenta regresiva
#define LBL_NEVER_MODE "Solo cambio manual"
#define LBL_STARTUP_MODE "Al iniciar la computadora"
#define INFO_STARTUP "Cambio de fondo de pantalla en el próximo reinicio de la computadora"
#define INFO_MANUAL "Solo cambio manual de fondo de pantalla"
#define TIME_DAYS "d"
#define TIME_HOURS "h"
#define TIME_MINUTES "m"
#define TIME_SECONDS "s"

// Alerta de error de API
#define ERR_API_TITLE "Error al recuperar las categorías"
#define ERR_RETRY_IN "Reintentando en "
#define ERR_CHECK_CONNECTION "Verifique su conexión a internet y configuración de red."

// Bandeja del sistema
#define TRAY_SHOW "Mostrar"
#define TRAY_CHANGE_WALLPAPER "Cambiar fondo de pantalla"
#define TRAY_QUIT "Salir"

// Modos de activación
#define MODE_STARTUP "Inicio"
#define MODE_AUTOMATIC "Automático"
#define MODE_MANUAL "Manual"

// Diálogo de aplicación rápida de categoría
#define DIALOG_APPLY_TITLE "Aplicar fondo de pantalla"
#define DIALOG_CATEGORY "Categoría: "
#define DIALOG_TARGET_SCREEN "Pantalla de destino:"
#define DIALOG_SCREEN "Pantalla %1"
#define DIALOG_APPLY_THIS_IMAGE "Aplicar esta imagen"
#define DIALOG_APPLY_RANDOM_IMAGE "Aplicar una imagen de esta categoría"
#define DIALOG_CLOSE "Cerrar"

// Mensajes de historial
#define MSG_HISTORY_SELECTION_REQUIRED_TITLE "Selección requerida"
#define MSG_HISTORY_SELECTION_REQUIRED "Por favor seleccione una imagen del historial."
#define MSG_MULTISCREEN_WARNING_TITLE "Aplicación multipantalla"
#define MSG_MULTISCREEN_CONTINUE "¿Desea continuar?"
#define MSG_MULTISCREEN_DONT_SHOW "No mostrar esta advertencia nuevamente"

// Banner de error de API
#define ERR_BANNER_TITLE "Error al recuperar las categorías"
#define ERR_BANNER_RETRY "Reintentando en "
#define ERR_BANNER_CHECK_CONNECTION "Verifique su conexión a internet y configuración de red."
#define ERR_BANNER_BTN_DETAILS "Detalles"

#endif // LANG_ES_H
