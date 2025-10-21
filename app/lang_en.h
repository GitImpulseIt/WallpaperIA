#ifndef LANG_EN_H
#define LANG_EN_H

// Application information
#define APP_NAME "WallpaperAI"
#define APP_TITLE "WallpaperAI - AI-Generated Wallpapers"

// Tabs
#define TAB_WALLPAPER "Wallpaper"
#define TAB_CATEGORIES "Categories"
#define TAB_SETTINGS "Settings"

// Application Tab
#define BTN_CHANGE_NOW "🖼️ Change Now"
#define LBL_CLICK_TO_CHANGE "Click to change wallpaper"
#define LBL_NEXT_CHANGE "Next Change"
#define LBL_HISTORY "History"
#define BTN_APPLY "Apply"
#define BTN_PREV "◀"
#define BTN_NEXT "▶"
#define MSG_WALLPAPER_APPLIED "Wallpaper applied successfully!"
#define MSG_SAVE_ERROR "Error saving image."
#define MSG_LOAD_ERROR "Error loading image."
#define MSG_DOWNLOAD_ERROR "Download error: "
#define MSG_NO_IMAGE_AVAILABLE "No image available for this category."
#define MSG_NO_IMAGE_7DAYS "No image found for this category (last 7 days)."
#define MSG_API_ERROR "API error: "
#define MSG_DOWNLOADING_FROM_API "Downloading image from API..."
#define MSG_FETCHING_RANDOM_IMAGE "Fetching a random image..."
#define MSG_FETCHING_RANDOM_IMAGES "Fetching %1 random images..."
#define MSG_CONNECTION_ERROR "Wallpaper connection error for screen %1: %2"

// Categories Tab
#define MSG_NO_THUMBNAIL "No thumbnail available for this category."
#define LBL_THUMBNAIL "Thumbnail\n"
#define BTN_APPLY_THUMBNAIL "Apply"

// Settings Tab - Groups
#define GRP_FREQUENCY "Change frequency"
#define GRP_ADJUSTMENT "Image adjustment mode"
#define GRP_SYSTEM_OPTIONS "System options"

// Settings Tab - Frequency
#define LBL_FREQUENCY "Change frequency"
#define FREQ_MANUAL "Manual change only"
#define FREQ_1H "Every hour"
#define FREQ_3H "Every 3 hours"
#define FREQ_6H "Every 6 hours"
#define FREQ_12H "Every 12 hours"
#define FREQ_24H "Every 24 hours"
#define FREQ_7D "Every week"
#define FREQ_STARTUP "At computer startup"
#define FREQ_CUSTOM "Custom"

// Settings Tab - Custom units
#define UNIT_MINUTES "minutes"
#define UNIT_HOURS "hours"
#define UNIT_DAYS "days"

// Settings Tab - Startup options
#define LBL_STARTUP_WINDOWS "Start with Windows"
#define LBL_CHANGE_ON_STARTUP "Change wallpaper\nat computer startup"

// Settings Tab - Multi-screen
#define LBL_MULTI_SCREEN "Different image on each screen"

// Settings Tab - Adjustment
#define LBL_ADJUSTMENT "Image adjustment"
#define ADJ_FILL "Fill"
#define ADJ_FIT "Fit"
#define ADJ_SPAN "Span"
#define ADJ_STRETCH "Stretch"
#define ADJ_TILE "Tile"

// Adjustment mode explanations
#define EXPL_FILL "Image keeps its aspect ratio but exceeds the screen to fill it"
#define EXPL_FIT "Image keeps its aspect ratio but black bars are added to fit the screen"
#define EXPL_SPAN "Image spans across multiple screens"
#define EXPL_STRETCH "Image is stretched to fill the entire screen but aspect ratio is not preserved"
#define EXPL_TILE "Image repeats to fill the entire screen and aspect ratio is preserved"
#define EXPL_DEFAULT "Selected adjustment mode"

// Screen selector
#define MSG_SCREENS_SELECTED " screen(s) selected"
#define MSG_ALL_SCREENS "All screens"
#define MSG_CANNOT_DESELECT "Cannot deselect all screens. At least one screen must be selected."
#define MSG_SCREEN_SELECTED "Screen selected: "
#define MSG_SCREENS_SELECTED_PLURAL "Screens selected: "
#define MSG_NO_SCREEN_SELECTED "No screen selected\nSelect at least one screen"
#define MSG_SCREEN_NO_HISTORY_TITLE "Screen without history"
#define MSG_SCREEN_NO_HISTORY "Screen %1 cannot be deselected because it has no wallpaper history yet.\n\nApply at least one wallpaper to this screen first before you can deselect it."
#define MSG_WALLPAPER_APPLIED_SCREENS "The wallpaper will be applied to %1 selected screens."

// Countdown widget
#define LBL_NEVER_MODE "Manual change only"
#define LBL_STARTUP_MODE "At computer startup"
#define INFO_STARTUP "Wallpaper change at next computer restart"
#define INFO_MANUAL "Manual wallpaper change only"
#define TIME_DAYS "d"
#define TIME_HOURS "h"
#define TIME_MINUTES "m"
#define TIME_SECONDS "s"

// API error alert
#define ERR_API_TITLE "Error retrieving categories"
#define ERR_RETRY_IN "Retry in "
#define ERR_CHECK_CONNECTION "Check your internet connection and network settings."

// System Tray
#define TRAY_SHOW "Show"
#define TRAY_CHANGE_WALLPAPER "Change wallpaper"
#define TRAY_QUIT "Quit"

// Trigger modes
#define MODE_STARTUP "Startup"
#define MODE_AUTOMATIC "Automatic"
#define MODE_MANUAL "Manual"

// Quick apply category dialog
#define DIALOG_APPLY_TITLE "Apply Wallpaper"
#define DIALOG_CATEGORY "Category: "
#define DIALOG_TARGET_SCREEN "Target screen:"
#define DIALOG_SCREEN "Screen %1"
#define DIALOG_APPLY_THIS_IMAGE "Apply this image"
#define DIALOG_APPLY_RANDOM_IMAGE "Apply a random image from this category"
#define DIALOG_CLOSE "Close"

// History messages
#define MSG_HISTORY_SELECTION_REQUIRED_TITLE "Selection Required"
#define MSG_HISTORY_SELECTION_REQUIRED "Please select an image from the history."
#define MSG_MULTISCREEN_WARNING_TITLE "Multi-Screen Application"
#define MSG_MULTISCREEN_CONTINUE "Do you want to continue?"
#define MSG_MULTISCREEN_DONT_SHOW "Don't show this warning again"

// API error banner
#define ERR_BANNER_TITLE "Error retrieving categories"
#define ERR_BANNER_RETRY "Retry in "
#define ERR_BANNER_CHECK_CONNECTION "Check your internet connection and network settings."
#define ERR_BANNER_BTN_DETAILS "Details"

#endif // LANG_EN_H
