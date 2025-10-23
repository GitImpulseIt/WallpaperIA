#ifndef LANG_PT_H
#define LANG_PT_H

// Informações da aplicação
#define APP_NAME "WallpaperAI"
#define APP_TITLE "WallpaperAI - Papéis de parede gerados por IA"

// Abas
#define TAB_WALLPAPER "Papel de parede"
#define TAB_CATEGORIES "Categorias"
#define TAB_SETTINGS "Configurações"

// Aba Aplicação
#define BTN_CHANGE_NOW "🖼️ Mudar Agora"
#define LBL_CLICK_TO_CHANGE "Clique para mudar o papel de parede"
#define LBL_NEXT_CHANGE "Próxima Mudança"
#define LBL_HISTORY "Histórico"
#define BTN_APPLY "Aplicar"
#define BTN_PREV "◀"
#define BTN_NEXT "▶"
#define MSG_WALLPAPER_APPLIED "Papel de parede aplicado com sucesso!"
#define MSG_SAVE_ERROR "Erro ao salvar a imagem."
#define MSG_LOAD_ERROR "Erro ao carregar a imagem."
#define MSG_DOWNLOAD_ERROR "Erro de download: "
#define MSG_NO_IMAGE_AVAILABLE "Nenhuma imagem disponível para esta categoria."
#define MSG_NO_IMAGE_7DAYS "Nenhuma imagem encontrada para esta categoria (últimos 7 dias)."
#define MSG_API_ERROR "Erro de API: "
#define MSG_DOWNLOADING_FROM_API "Baixando imagem da API..."
#define MSG_FETCHING_RANDOM_IMAGE "Obtendo uma imagem aleatória..."
#define MSG_FETCHING_RANDOM_IMAGES "Obtendo %1 imagens aleatórias..."
#define MSG_CHANGING_IN_PROGRESS "🔄 Mudança em andamento..."
#define MSG_APPLYING_WALLPAPERS "Aplicando papéis de parede..."
#define MSG_APPLYING_WALLPAPER "Aplicando papel de parede..."
#define MSG_CONNECTION_ERROR "Erro de conexão papéis de parede para tela %1: %2"

// Aba Categorias
#define MSG_NO_THUMBNAIL "Nenhuma miniatura disponível para esta categoria."
#define LBL_THUMBNAIL "Miniatura\n"
#define BTN_APPLY_THUMBNAIL "Aplicar"

// Aba Configurações - Grupos
#define GRP_FREQUENCY "Frequência de mudança"
#define GRP_ADJUSTMENT "Modo de ajuste da imagem"
#define GRP_SYSTEM_OPTIONS "Opções do sistema"

// Aba Configurações - Frequência
#define LBL_FREQUENCY "Frequência de mudança"
#define FREQ_MANUAL "Apenas mudança manual"
#define FREQ_1H "A cada hora"
#define FREQ_3H "A cada 3 horas"
#define FREQ_6H "A cada 6 horas"
#define FREQ_12H "A cada 12 horas"
#define FREQ_24H "A cada 24 horas"
#define FREQ_7D "A cada semana"
#define FREQ_STARTUP "Na inicialização do computador"
#define FREQ_CUSTOM "Outro"

// Aba Configurações - Unidades personalizadas
#define UNIT_MINUTES "minutos"
#define UNIT_HOURS "horas"
#define UNIT_DAYS "dias"

// Aba Configurações - Opções de inicialização
#define LBL_STARTUP_WINDOWS "Iniciar com Windows"
#define LBL_CHANGE_ON_STARTUP "Mudar o papel de parede\nna inicialização do computador"

// Aba Configurações - Multi-telas
#define LBL_MULTI_SCREEN "Imagem diferente em cada tela"
#define LBL_SCREEN_NAME "Tela %1"

// Aba Configurações - Ajuste
#define LBL_ADJUSTMENT "Ajuste da imagem"
#define ADJ_FILL "Preencher"
#define ADJ_FIT "Ajustar"
#define ADJ_SPAN "Expandir"
#define ADJ_STRETCH "Esticar"
#define ADJ_TILE "Mosaico"

// Explicações dos modos de ajuste
#define EXPL_FILL "A imagem mantém sua proporção mas é cortada para preencher a tela"
#define EXPL_FIT "A imagem mantém sua proporção mas barras pretas são adicionadas para caber na tela"
#define EXPL_SPAN "A imagem se expande por várias telas"
#define EXPL_STRETCH "A imagem é esticada para preencher toda a tela mas a proporção não é respeitada"
#define EXPL_TILE "A imagem se repete para preencher toda a tela e a proporção é conservada"
#define EXPL_DEFAULT "Modo de ajuste selecionado"

// Seletor de tela
#define MSG_SCREENS_SELECTED " tela(s) selecionada(s)"
#define MSG_ALL_SCREENS "Todas as telas"
#define MSG_CANNOT_DESELECT "Não é possível desmarcar todas as telas. Pelo menos uma tela deve estar selecionada."
#define MSG_SCREEN_SELECTED "Tela selecionada: "
#define MSG_SCREENS_SELECTED_PLURAL "Telas selecionadas: "
#define MSG_NO_SCREEN_SELECTED "Nenhuma tela selecionada\nSelecione pelo menos uma tela"
#define MSG_SCREEN_NO_HISTORY_TITLE "Tela sem histórico"
#define MSG_SCREEN_NO_HISTORY "A tela %1 não pode ser desmarcada porque ainda não tem histórico de papéis de parede.\n\nPrimeiro aplique pelo menos um papel de parede a esta tela para poder desmarcá-la."
#define MSG_WALLPAPER_APPLIED_SCREENS "O papel de parede será aplicado em %1 telas selecionadas."

// Widget de contagem regressiva
#define LBL_NEVER_MODE "Apenas mudança manual"
#define LBL_STARTUP_MODE "Na inicialização do computador"
#define INFO_STARTUP "Mudança de papel de parede na próxima reinicialização do computador"
#define INFO_MANUAL "Apenas mudança manual de papel de parede"
#define TIME_DAYS "d"
#define TIME_HOURS "h"
#define TIME_MINUTES "m"
#define TIME_SECONDS "s"

// Alerta de erro de API
#define ERR_API_TITLE "Erro ao recuperar as categorias"
#define ERR_RETRY_IN "Tentando novamente em "
#define ERR_CHECK_CONNECTION "Verifique sua conexão com a internet e configurações de rede."

// Bandeja do sistema
#define TRAY_SHOW "Mostrar"
#define TRAY_CHANGE_WALLPAPER "Mudar papel de parede"
#define TRAY_QUIT "Sair"

// Modos de ativação
#define MODE_STARTUP "Inicialização"
#define MODE_AUTOMATIC "Automático"
#define MODE_MANUAL "Manual"

// Diálogo de aplicação rápida de categoria
#define DIALOG_APPLY_TITLE "Aplicar papel de parede"
#define DIALOG_CATEGORY "Categoria: "
#define DIALOG_TARGET_SCREEN "Tela de destino:"
#define DIALOG_SCREEN "Tela %1"
#define DIALOG_APPLY_THIS_IMAGE "Aplicar esta imagem"
#define DIALOG_APPLY_RANDOM_IMAGE "Aplicar uma imagem desta categoria"
#define DIALOG_CLOSE "Fechar"

// Mensagens de histórico
#define MSG_HISTORY_SELECTION_REQUIRED_TITLE "Seleção necessária"
#define MSG_HISTORY_SELECTION_REQUIRED "Por favor selecione uma imagem do histórico."
#define MSG_MULTISCREEN_WARNING_TITLE "Aplicação multi-telas"
#define MSG_MULTISCREEN_CONTINUE "Deseja continuar?"
#define MSG_MULTISCREEN_DONT_SHOW "Não mostrar este aviso novamente"

// Banner de erro de API
#define ERR_BANNER_TITLE "Erro ao recuperar as categorias"
#define ERR_BANNER_RETRY "Tentando novamente em "
#define ERR_BANNER_CHECK_CONNECTION "Verifique sua conexão com a internet e configurações de rede."
#define ERR_BANNER_BTN_DETAILS "Detalhes"

#endif // LANG_PT_H
