#ifndef LANG_RU_H
#define LANG_RU_H

// Информация о приложении
#define APP_NAME "WallpaperAI"
#define APP_TITLE "WallpaperAI - Обои, созданные ИИ"

// Вкладки
#define TAB_WALLPAPER "Обои"
#define TAB_CATEGORIES "Категории"
#define TAB_SETTINGS "Настройки"

// Вкладка Приложение
#define BTN_CHANGE_NOW "🖼️ Изменить Сейчас"
#define LBL_CLICK_TO_CHANGE "Нажмите, чтобы изменить обои"
#define LBL_NEXT_CHANGE "Следующее Изменение"
#define LBL_HISTORY "История"
#define BTN_APPLY "Применить"
#define BTN_PREV "◀"
#define BTN_NEXT "▶"
#define MSG_WALLPAPER_APPLIED "Обои успешно применены!"
#define MSG_SAVE_ERROR "Ошибка при сохранении изображения."
#define MSG_LOAD_ERROR "Ошибка при загрузке изображения."
#define MSG_DOWNLOAD_ERROR "Ошибка загрузки: "
#define MSG_NO_IMAGE_AVAILABLE "Нет доступных изображений для этой категории."
#define MSG_NO_IMAGE_7DAYS "Изображение не найдено для этой категории (последние 7 дней)."
#define MSG_API_ERROR "Ошибка API: "
#define MSG_DOWNLOADING_FROM_API "Загрузка изображения из API..."
#define MSG_FETCHING_RANDOM_IMAGE "Получение случайного изображения..."
#define MSG_FETCHING_RANDOM_IMAGES "Получение %1 случайных изображений..."
#define MSG_CHANGING_IN_PROGRESS "🔄 Изменение в процессе..."
#define MSG_APPLYING_WALLPAPERS "Применение обоев..."
#define MSG_APPLYING_WALLPAPER "Применение обоев..."
#define MSG_CONNECTION_ERROR "Ошибка подключения обои для экрана %1: %2"

// Вкладка Категории
#define MSG_NO_THUMBNAIL "Нет доступной миниатюры для этой категории."
#define LBL_THUMBNAIL "Миниатюра\n"
#define BTN_APPLY_THUMBNAIL "Применить"

// Вкладка Настройки - Группы
#define GRP_FREQUENCY "Частота изменения"
#define GRP_ADJUSTMENT "Режим подгонки изображения"
#define GRP_SYSTEM_OPTIONS "Системные настройки"

// Вкладка Настройки - Частота
#define LBL_FREQUENCY "Частота изменения"
#define FREQ_MANUAL "Только ручное изменение"
#define FREQ_1H "Каждый час"
#define FREQ_3H "Каждые 3 часа"
#define FREQ_6H "Каждые 6 часов"
#define FREQ_12H "Каждые 12 часов"
#define FREQ_24H "Каждые 24 часа"
#define FREQ_7D "Каждую неделю"
#define FREQ_STARTUP "При запуске компьютера"
#define FREQ_CUSTOM "Другое"

// Вкладка Настройки - Пользовательские единицы
#define UNIT_MINUTES "минут"
#define UNIT_HOURS "часов"
#define UNIT_DAYS "дней"

// Вкладка Настройки - Параметры запуска
#define LBL_STARTUP_WINDOWS "Запускать с Windows"
#define LBL_CHANGE_ON_STARTUP "Менять обои\nпри запуске компьютера"

// Вкладка Настройки - Несколько экранов
#define LBL_MULTI_SCREEN "Разное изображение на каждом экране"
#define LBL_SCREEN_NAME "Экран %1"

// Вкладка Настройки - Подгонка
#define LBL_ADJUSTMENT "Подгонка изображения"
#define ADJ_FILL "Заполнить"
#define ADJ_FIT "Вписать"
#define ADJ_SPAN "Растянуть"
#define ADJ_STRETCH "Растянуть"
#define ADJ_TILE "Мозаика"

// Объяснения режимов подгонки
#define EXPL_FILL "Изображение сохраняет пропорции, но обрезается для заполнения экрана"
#define EXPL_FIT "Изображение сохраняет пропорции, но добавляются черные полосы для размещения на экране"
#define EXPL_SPAN "Изображение растягивается на несколько экранов"
#define EXPL_STRETCH "Изображение растягивается для заполнения всего экрана, но пропорции не соблюдаются"
#define EXPL_TILE "Изображение повторяется для заполнения всего экрана, пропорции сохраняются"
#define EXPL_DEFAULT "Выбранный режим подгонки"

// Селектор экрана
#define MSG_SCREENS_SELECTED " экран(ов) выбрано"
#define MSG_ALL_SCREENS "Все экраны"
#define MSG_CANNOT_DESELECT "Невозможно отменить выбор всех экранов. Должен быть выбран хотя бы один экран."
#define MSG_SCREEN_SELECTED "Выбран экран: "
#define MSG_SCREENS_SELECTED_PLURAL "Выбраны экраны: "
#define MSG_NO_SCREEN_SELECTED "Нет выбранных экранов\nВыберите хотя бы один экран"
#define MSG_SCREEN_NO_HISTORY_TITLE "Экран без истории"
#define MSG_SCREEN_NO_HISTORY "Экран %1 нельзя отменить, потому что у него еще нет истории обоев.\n\nСначала примените хотя бы одни обои к этому экрану, чтобы иметь возможность отменить выбор."
#define MSG_WALLPAPER_APPLIED_SCREENS "Обои будут применены к %1 выбранным экранам."

// Виджет обратного отсчета
#define LBL_NEVER_MODE "Только ручное изменение"
#define LBL_STARTUP_MODE "При запуске компьютера"
#define INFO_STARTUP "Смена обоев при следующей перезагрузке компьютера"
#define INFO_MANUAL "Только ручная смена обоев"
#define TIME_DAYS "д"
#define TIME_HOURS "ч"
#define TIME_MINUTES "м"
#define TIME_SECONDS "с"

// Предупреждение об ошибке API
#define ERR_API_TITLE "Ошибка при получении категорий"
#define ERR_RETRY_IN "Повторная попытка через "
#define ERR_CHECK_CONNECTION "Проверьте подключение к Интернету и сетевые настройки."

// Системный трей
#define TRAY_SHOW "Показать"
#define TRAY_CHANGE_WALLPAPER "Изменить обои"
#define TRAY_QUIT "Выйти"

// Режимы активации
#define MODE_STARTUP "Запуск"
#define MODE_AUTOMATIC "Автоматически"
#define MODE_MANUAL "Вручную"

// Диалог быстрого применения категории
#define DIALOG_APPLY_TITLE "Применить обои"
#define DIALOG_CATEGORY "Категория: "
#define DIALOG_TARGET_SCREEN "Целевой экран:"
#define DIALOG_SCREEN "Экран %1"
#define DIALOG_APPLY_THIS_IMAGE "Применить это изображение"
#define DIALOG_APPLY_RANDOM_IMAGE "Применить изображение из этой категории"
#define DIALOG_CLOSE "Закрыть"

// Сообщения истории
#define MSG_HISTORY_SELECTION_REQUIRED_TITLE "Требуется выбор"
#define MSG_HISTORY_SELECTION_REQUIRED "Пожалуйста, выберите изображение из истории."
#define MSG_MULTISCREEN_WARNING_TITLE "Применение на несколько экранов"
#define MSG_MULTISCREEN_CONTINUE "Хотите продолжить?"
#define MSG_MULTISCREEN_DONT_SHOW "Больше не показывать это предупреждение"

// Баннер ошибки API
#define ERR_BANNER_TITLE "Ошибка при получении категорий"
#define ERR_BANNER_RETRY "Повторная попытка через "
#define ERR_BANNER_CHECK_CONNECTION "Проверьте подключение к Интернету и сетевые настройки."
#define ERR_BANNER_BTN_DETAILS "Подробности"

#endif // LANG_RU_H
