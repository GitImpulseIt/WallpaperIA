#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QOperatingSystemVersion>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QEvent>
#include <QScrollArea>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPixmap>
#include <QFrame>
#include <QPushButton>
#include <QTabWidget>
#include <QStandardPaths>
#include <QRandomGenerator>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QEvent>
#include <QTimer>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QPainter>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QSettings>
#include <QPainterPath>
#include <QCryptographicHash>
#include <QDateTime>
#include <QStandardItemModel>
#include <QRegularExpression>
#include <QPointer>
#include <string>

// Includes des modules séparés
#include "src/utils/utils.h"
#include "src/utils/path_helper.h"
#include "src/utils/date_helper.h"
#include "src/config/startup_manager.h"
#include "src/system/wallpaper_manager.h"
#include "src/widgets/hover_filters.h"
#include "src/widgets/screen_selector.h"
#include "src/widgets/countdown_widget.h"
#include "src/widgets/toggle_switch.h"
#include "src/core/wallpaper_builder.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>
#include <shobjidl.h>
#include <wingdi.h>
#include <QCoreApplication>
#include <QDir>

// Les définitions Windows sont maintenant dans src/system/wallpaper_manager.h

#endif

// Les classes de gestion des événements de survol sont maintenant dans src/widgets/hover_filters.h

// La classe ScreenSelector est maintenant dans src/widgets/screen_selector.h

// La classe CountdownWidget est maintenant dans src/widgets/countdown_widget.h

// La classe ToggleSwitch est maintenant dans src/widgets/toggle_switch.h

// Les fonctions de gestion du démarrage Windows sont maintenant dans src/config/startup_manager.h

class ModernWindow : public QWidget
{
    Q_OBJECT

public:

    ModernWindow(QWidget *parent = nullptr) : QWidget(parent), networkManager(new QNetworkAccessManager(this)), isLoadingSettings(false), retryCountdownSeconds(0), currentHistoryScreen(0), historyScrollOffset(0), dontShowMultiScreenWarning(false)
    {
        setWindowTitle("WallpaperAI");
        setWindowIcon(QIcon(getImagePath("icon.png")));
        setFixedSize(725, 650);

        // Configuration des boutons de la fenêtre : minimiser et fermer (pas d'agrandissement)
        setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

        // Initialiser le chemin du cache des catégories
        categoriesCachePath = PathHelper::getCategoriesCachePath();

        // Initialiser le timer de retry
        retryTimer = new QTimer(this);
        connect(retryTimer, &QTimer::timeout, this, &ModernWindow::onRetryTimerTick);

        setupUI();
        applyModernStyle();
        setupSystemTray();
        loadCategories();
        loadSettings();

        // Initialiser les états de désélection basés sur les logs
        initializeScreenDeselectionStates();

        // Nettoyer les anciens fichiers wallpapers au démarrage
        cleanupAllWallpapers();

        // Nettoyer les fichiers temporaires du démarrage précédent
        cleanupTemporaryFiles();

        // Gestion du démarrage automatique
        handleStartupBehavior();
    }

    int getCategoryRating(const QString &categoryId) {
        return categoryRatings.value(categoryId, 1); // Retourne 1 par défaut
    }

private slots:
    void onApplySettings() {
        saveSettings();
        updateCountdownFromSettings(); // Mettre à jour le countdown uniquement lors de l'application
        applyButton->setEnabled(false); // Désactiver après avoir appliqué
    }

    void onSettingsChanged() {
        if (!isLoadingSettings) {
            applyButton->setEnabled(true); // Activer quand des changements sont détectés
        }
    }

    void onStartupToggled(bool enabled) {
        // Logique de cohérence : si "Démarrer avec Windows" est désactivé,
        // l'option "Changer le fond d'écran au démarrage" doit être désactivée
        if (!enabled && changeOnStartupToggle) {
            changeOnStartupToggle->setChecked(false);
        }

        // Mettre à jour l'état d'activation de l'option changeOnStartup
        if (changeOnStartupToggle) {
            changeOnStartupToggle->setEnabled(enabled);
        }

        updateFrequencyOptions();

        // Gérer l'ajout/suppression du registre Windows
        applyStartupSetting(enabled);
    }

    void updateFrequencyOptions() {
        if (!frequencyCombo || isLoadingSettings) return;

        int currentIndex = frequencyCombo->currentIndex();
        bool startupEnabled = startupToggle->isChecked();

        // Si "Démarrer avec Windows" est désactivé et "Au démarrage" est sélectionné
        if (!startupEnabled && currentIndex == 7) { // index 7 = "Au démarrage de l'ordinateur"
            // Basculer sur "Changement manuel uniquement"
            frequencyCombo->setCurrentIndex(0);
            onSettingsChanged(); // Marquer comme modifié
        }

        // Désactiver/activer l'option "Au démarrage" selon l'état du toggle
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(frequencyCombo->model());
        if (model) {
            QStandardItem* item = model->item(7); // "Au démarrage de l'ordinateur"
            if (item) {
                item->setEnabled(startupEnabled);
                if (!startupEnabled) {
                    item->setData(QColor(128, 128, 128), Qt::ForegroundRole); // Grisé
                } else {
                    item->setData(QColor(255, 255, 255), Qt::ForegroundRole); // Normal
                }
            }
        }
    }

    void handleStartupBehavior() {
        // S'assurer que l'option "Démarrer avec Windows" est correctement appliquée
        if (startupToggle && startupToggle->isChecked()) {
            applyStartupSetting(true);
        }

        // Vérifier si on est lancé au démarrage et si l'option "Au démarrage" est sélectionnée
        QStringList arguments = QCoreApplication::arguments();
        bool launchedAtStartup = arguments.contains("--startup");

        if (launchedAtStartup && changeOnStartupToggle && changeOnStartupToggle->isChecked()) {
            // Option "Changer le fond d'écran au démarrage" activée
            // Déclencher le changement de fond d'écran
            QTimer::singleShot(2000, [this]() {
                currentTriggerMode = "Démarrage";
                onChangeNowClicked();
            }); // Délai de 2s pour laisser le système démarrer
        }
    }

    void applyStartupSetting(bool enabled) {
        #ifdef Q_OS_WIN
        QString appName = "WallpaperAI";
        QString appPath = QCoreApplication::applicationFilePath();

        if (enabled) {
            // Ajouter l'argument --startup pour démarrer dans le tray
            // Convertir le chemin en format Windows avec backslashes
            QString windowsPath = QDir::toNativeSeparators(appPath);
            QString startupCommand = QString("\"%1\" --startup").arg(windowsPath);
            if (!StartupManager::addToWindowsStartup(appName, startupCommand)) {
                // En cas d'erreur, désactiver le toggle
                startupToggle->setChecked(false);
            }
        } else {
            StartupManager::removeFromWindowsStartup(appName);
        }
        #endif
    }

    bool checkStartupStatus() {
        #ifdef Q_OS_WIN
        return StartupManager::isInWindowsStartup("WallpaperAI");
        #else
        return false;
        #endif
    }

    void onMultiScreenToggled(bool enabled) {
        if (screenSelector) {
            if (enabled && screenSelector->screenCount() > 1) {
                screenSelector->show();
                // Mettre à jour le statut pour indiquer les écrans sélectionnés
                onScreenSelectionChanged(screenSelector->getSelectedScreens());
            } else {
                screenSelector->hide();
                // Remettre le statut par défaut
                statusLabel->setText("Cliquez pour changer le fond d'écran");
            }
        }

        // Gérer la disponibilité du mode "Étendre"
        updateAdjustmentOptions(enabled);
    }

    void updateAdjustmentOptions(bool multiScreenEnabled) {
        if (!adjustmentCombo) return;

        // Le mode "Étendre" (index 2) n'est pas compatible avec "Image différente sur chaque écran"
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(adjustmentCombo->model());
        if (model) {
            QStandardItem* spanItem = model->item(2); // Index 2 = "Étendre"
            if (spanItem) {
                bool spanAvailable = !multiScreenEnabled;
                spanItem->setEnabled(spanAvailable);

                if (!spanAvailable) {
                    spanItem->setData(QColor(128, 128, 128), Qt::ForegroundRole); // Grisé
                    // Si "Étendre" est actuellement sélectionné, basculer vers "Remplir"
                    if (adjustmentCombo->currentIndex() == 2) {
                        adjustmentCombo->setCurrentIndex(0); // "Remplir"
                        onSettingsChanged(); // Marquer comme modifié
                    }
                } else {
                    spanItem->setData(QColor(255, 255, 255), Qt::ForegroundRole); // Normal
                }
            }
        }
    }

    void updateAdjustmentExplanation(int index) {
        if (!adjustmentExplanationLabel) return;

        QString explanation;
        switch (index) {
            case 0: // Remplir
                explanation = "L'image conserve son ratio mais dépasse de l'écran pour le remplir";
                break;
            case 1: // Ajuster
                explanation = "L'image conserve son ratio mais des bandes noires sont ajoutées pour tenir dans l'écran";
                break;
            case 2: // Étendre
                explanation = "L'image s'étend sur plusieurs écrans";
                break;
            case 3: // Étirer
                explanation = "L'image est étirée de façon à remplir tout l'écran mais le ratio n'est pas respecté";
                break;
            case 4: // Mosaïque
                explanation = "L'image se répète de façon à remplir tout l'écran et le ratio est conservé";
                break;
            default:
                explanation = "Mode d'ajustement sélectionné";
                break;
        }

        adjustmentExplanationLabel->setText(explanation);
    }

    void updateCountdownFromSettings() {
        if (!countdownWidget) return;

        int seconds = 3600; // Valeur par défaut (1h)

        // Calculer la durée en secondes selon la sélection
        int frequencyIndex = frequencyCombo->currentIndex();
        switch (frequencyIndex) {
            case 0: seconds = -1; break;       // Changement manuel uniquement (pas de timer)
            case 1: seconds = 3600; break;     // 1h
            case 2: seconds = 10800; break;    // 3h
            case 3: seconds = 21600; break;    // 6h
            case 4: seconds = 43200; break;    // 12h
            case 5: seconds = 86400; break;    // 24h
            case 6: seconds = 604800; break;   // 7j
            case 7: seconds = 0; break;        // Au démarrage (pas de timer)
            case 8: // Autre
                {
                    int value = customValueSpinBox->value();
                    int unit = customUnitCombo->currentIndex();
                    switch (unit) {
                        case 0: seconds = value * 60; break;      // minutes
                        case 1: seconds = value * 3600; break;    // heures
                        case 2: seconds = value * 86400; break;   // jours
                    }
                }
                break;
        }

        countdownWidget->setDuration(seconds);
    }

private:

    void setupUI()
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(15, 15, 15, 15);
        mainLayout->setSpacing(10);

        // Titre principal
        QLabel *titleLabel = new QLabel("WallpaperAI - Fonds d'écrans générés par IA");
        titleLabel->setObjectName("titleLabel");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);

        // Créer le widget d'onglets
        tabWidget = new QTabWidget();
        tabWidget->setObjectName("tabWidget");

        // Onglet Application
        setupApplicationTab();

        // Onglet Catégories
        setupCategoriesTab();

        // Onglet Paramètres
        setupSettingsTab();

        mainLayout->addWidget(tabWidget);
    }

    void setupApplicationTab()
    {
        QWidget *applicationTab = new QWidget();
        QVBoxLayout *applicationLayout = new QVBoxLayout(applicationTab);
        applicationLayout->setContentsMargins(20, 20, 20, 20);
        applicationLayout->setSpacing(20);

        // Layout principal horizontal : zone gauche (contrôles) + zone droite (countdown)
        QHBoxLayout *mainLayout = new QHBoxLayout();
        mainLayout->setSpacing(60); // Espacement réduit entre les groupes

        // === ZONE GAUCHE : Contrôles ===
        QVBoxLayout *leftControlsLayout = new QVBoxLayout();
        leftControlsLayout->setSpacing(10); // Espacement réduit pour un meilleur alignement
        leftControlsLayout->setAlignment(Qt::AlignTop);

        // Container avec largeur fixe pour justification
        QWidget *controlsContainer = new QWidget();
        controlsContainer->setFixedWidth(280); // Largeur standard pour alignement correct du bouton
        QVBoxLayout *containerLayout = new QVBoxLayout(controlsContainer);
        containerLayout->setContentsMargins(0, 0, 0, 0);
        containerLayout->setSpacing(10);

        // Sélecteur d'écran en haut
        screenSelector = new ScreenSelector();
        screenSelector->hide(); // Masqué par défaut
        screenSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // Expansion horizontale
        connect(screenSelector, &ScreenSelector::screenSelectionChanged, this, &ModernWindow::onScreenSelectionChanged);
        connect(screenSelector, &ScreenSelector::screenDeselectionBlocked, this, &ModernWindow::onScreenDeselectionBlocked);
        containerLayout->addWidget(screenSelector);

        // Bouton "Changer Maintenant" sous le sélecteur, prend toute la largeur disponible
        changeNowButton = new QPushButton("🖼️ Changer Maintenant");
        changeNowButton->setFixedHeight(50);
        changeNowButton->setStyleSheet(
            "QPushButton {"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2196F3, stop:1 #1976D2);"
            "color: white;"
            "border: none;"
            "border-radius: 8px;"
            "font-size: 14pt;"
            "font-weight: bold;"
            "padding: 8px 20px;"
            "}"
            "QPushButton:hover {"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #42A5F5, stop:1 #2196F3);"
            "}"
            "QPushButton:pressed {"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1565C0, stop:1 #0D47A1);"
            "}"
            "QPushButton:disabled {"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #BDBDBD, stop:1 #9E9E9E);"
            "color: #757575;"
            "}"
        );
        connect(changeNowButton, &QPushButton::clicked, this, &ModernWindow::onChangeNowClicked);
        containerLayout->addWidget(changeNowButton);

        // Label pour le statut sous le bouton
        statusLabel = new QLabel("Cliquez pour changer le fond d'écran");
        statusLabel->setAlignment(Qt::AlignLeft);
        statusLabel->setWordWrap(true); // Permettre le retour à la ligne
        statusLabel->setStyleSheet("color: #ADD8E6; font-size: 11pt; margin: 10px 0px;");
        containerLayout->addWidget(statusLabel);

        // Ajouter le container au layout principal
        leftControlsLayout->addWidget(controlsContainer);

        // === ZONE DROITE : Countdown ===
        QVBoxLayout *rightCountdownLayout = new QVBoxLayout();
        rightCountdownLayout->setSpacing(0); // Même espacement que les contrôles de gauche
        rightCountdownLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

        // Titre pour le countdown
        QLabel *countdownTitle = new QLabel("Prochain Changement");
        countdownTitle->setAlignment(Qt::AlignCenter);
        countdownTitle->setStyleSheet("color: #ffffff; font-size: 12pt; font-weight: bold;"); // Suppression du margin-bottom
        rightCountdownLayout->addWidget(countdownTitle);

        // Widget de compte à rebours
        countdownWidget = new CountdownWidget();
        connect(countdownWidget, &CountdownWidget::countdownExpired, [this]() {
            currentTriggerMode = "Automatique";
            onChangeNowClicked();
        });
        rightCountdownLayout->addWidget(countdownWidget, 0, Qt::AlignCenter);

        // Assemblage du layout principal - centrer les groupes
        mainLayout->addStretch(); // Espacement à gauche pour centrer
        mainLayout->addLayout(leftControlsLayout);
        mainLayout->addLayout(rightCountdownLayout);
        mainLayout->addStretch(); // Espacement à droite pour centrer

        applicationLayout->addLayout(mainLayout);

        // Espace pour le futur carrousel
        applicationLayout->addStretch();

        // Widget d'alerte erreur API (masqué par défaut)
        errorAlertWidget = new QWidget();
        errorAlertWidget->setFixedHeight(70);
        errorAlertWidget->setStyleSheet(
            "QWidget {"
            "background-color: #d14836;"
            "border: 2px solid #a63929;"
            "border-radius: 8px;"
            "}"
        );
        errorAlertWidget->hide();

        QHBoxLayout *errorAlertLayout = new QHBoxLayout(errorAlertWidget);
        errorAlertLayout->setContentsMargins(15, 10, 15, 10);
        errorAlertLayout->setSpacing(15);

        // Icône warning
        QLabel *warningIcon = new QLabel("⚠️");
        warningIcon->setStyleSheet("font-size: 32pt; background: transparent; border: none;");
        errorAlertLayout->addWidget(warningIcon);

        // Message d'erreur avec compteur
        errorAlertLabel = new QLabel();
        errorAlertLabel->setStyleSheet(
            "color: white;"
            "font-size: 11pt;"
            "font-weight: bold;"
            "background: transparent;"
            "border: none;"
        );
        errorAlertLabel->setWordWrap(true);
        errorAlertLayout->addWidget(errorAlertLabel, 1);

        applicationLayout->addWidget(errorAlertWidget);

        // === CARROUSEL D'HISTORIQUE ===
        QWidget *historyContainer = new QWidget();
        QVBoxLayout *historyMainLayout = new QVBoxLayout(historyContainer);
        historyMainLayout->setContentsMargins(0, 10, 0, 0);
        historyMainLayout->setSpacing(10);

        // Titre et contrôles
        QHBoxLayout *historyHeaderLayout = new QHBoxLayout();
        QLabel *historyTitle = new QLabel("Historique");
        historyTitle->setStyleSheet("color: #ffffff; font-size: 11pt; font-weight: bold;");
        historyHeaderLayout->addWidget(historyTitle);

        historyHeaderLayout->addStretch();

        // Bouton Appliquer
        QPushButton *applyHistoryButton = new QPushButton("Appliquer");
        applyHistoryButton->setFixedWidth(100);
        applyHistoryButton->setStyleSheet(
            "QPushButton {"
            "background-color: #2196F3;"
            "color: white;"
            "border: none;"
            "border-radius: 4px;"
            "padding: 6px 12px;"
            "font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "background-color: #42A5F5;"
            "}"
            "QPushButton:pressed {"
            "background-color: #1565C0;"
            "}"
            "QPushButton:disabled {"
            "background-color: #757575;"
            "color: #BDBDBD;"
            "}"
        );
        connect(applyHistoryButton, &QPushButton::clicked, this, &ModernWindow::onApplyHistoryClicked);
        historyHeaderLayout->addWidget(applyHistoryButton);

        historyMainLayout->addLayout(historyHeaderLayout);

        // Zone carrousel avec boutons de navigation
        QHBoxLayout *carouselNavigationLayout = new QHBoxLayout();
        carouselNavigationLayout->setSpacing(5);

        // Bouton précédent
        historyPrevButton = new QPushButton("◀");
        historyPrevButton->setFixedSize(40, 120);
        historyPrevButton->setStyleSheet(
            "QPushButton {"
            "background-color: #3b3b3b;"
            "color: white;"
            "border: 1px solid #555555;"
            "border-radius: 4px;"
            "font-size: 18pt;"
            "font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "background-color: #2196F3;"
            "border-color: #2196F3;"
            "}"
            "QPushButton:pressed {"
            "background-color: #1565C0;"
            "}"
            "QPushButton:disabled {"
            "background-color: #2b2b2b;"
            "color: #555555;"
            "border-color: #444444;"
            "}"
        );
        connect(historyPrevButton, &QPushButton::clicked, this, &ModernWindow::onHistoryPrevClicked);
        carouselNavigationLayout->addWidget(historyPrevButton);

        // Zone scrollable horizontale pour les miniatures (sans scrollbar)
        historyCarouselScrollArea = new QScrollArea();
        historyCarouselScrollArea->setFixedHeight(120);
        historyCarouselScrollArea->setWidgetResizable(true);
        historyCarouselScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        historyCarouselScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        historyCarouselScrollArea->setStyleSheet(
            "QScrollArea {"
            "background-color: #2b2b2b;"
            "border: 1px solid #555555;"
            "border-radius: 4px;"
            "}"
        );

        historyCarouselWidget = new QWidget();
        historyCarouselLayout = new QHBoxLayout(historyCarouselWidget);
        historyCarouselLayout->setContentsMargins(10, 10, 10, 10);
        historyCarouselLayout->setSpacing(10);
        historyCarouselLayout->setAlignment(Qt::AlignLeft);

        historyCarouselScrollArea->setWidget(historyCarouselWidget);
        carouselNavigationLayout->addWidget(historyCarouselScrollArea, 1);

        // Bouton suivant
        historyNextButton = new QPushButton("▶");
        historyNextButton->setFixedSize(40, 120);
        historyNextButton->setStyleSheet(
            "QPushButton {"
            "background-color: #3b3b3b;"
            "color: white;"
            "border: 1px solid #555555;"
            "border-radius: 4px;"
            "font-size: 18pt;"
            "font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "background-color: #2196F3;"
            "border-color: #2196F3;"
            "}"
            "QPushButton:pressed {"
            "background-color: #1565C0;"
            "}"
            "QPushButton:disabled {"
            "background-color: #2b2b2b;"
            "color: #555555;"
            "border-color: #444444;"
            "}"
        );
        connect(historyNextButton, &QPushButton::clicked, this, &ModernWindow::onHistoryNextClicked);
        carouselNavigationLayout->addWidget(historyNextButton);

        historyMainLayout->addLayout(carouselNavigationLayout);

        applicationLayout->addWidget(historyContainer);

        tabWidget->addTab(applicationTab, "Fond d'écran");
    }

    void setupCategoriesTab()
    {
        QWidget *categoriesTab = new QWidget();
        QVBoxLayout *categoriesLayout = new QVBoxLayout(categoriesTab);
        categoriesLayout->setContentsMargins(0, 0, 0, 0);
        categoriesLayout->setSpacing(10);

        // Zone scrollable pour les catégories
        scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setObjectName("scrollArea");

        // Widget contenant la grille des catégories
        categoriesWidget = new QWidget();
        categoriesGridLayout = new QGridLayout(categoriesWidget);
        categoriesGridLayout->setSpacing(15);
        categoriesGridLayout->setContentsMargins(10, 10, 10, 10);

        scrollArea->setWidget(categoriesWidget);
        categoriesLayout->addWidget(scrollArea);

        tabWidget->addTab(categoriesTab, "Catégories");
    }

    void setupSettingsTab()
    {
        QWidget *settingsTab = new QWidget();
        QVBoxLayout *settingsLayout = new QVBoxLayout(settingsTab);
        settingsLayout->setContentsMargins(20, 20, 20, 20);
        settingsLayout->setSpacing(15);

        // Groupe Fréquence de changement
        QGroupBox *frequencyGroup = new QGroupBox("Fréquence de changement");
        frequencyGroup->setStyleSheet(
            "QGroupBox {"
            "font-weight: 600;"
            "font-size: 14px;"
            "color: #ffffff;"
            "border: 1px solid #555;"
            "border-radius: 12px;"
            "margin-top: 15px;"
            "padding-top: 15px;"
            "background-color: #3a3a3a;"
            "}"
            "QGroupBox::title {"
            "subcontrol-origin: margin;"
            "subcontrol-position: top left;"
            "left: 15px;"
            "padding: 5px 12px;"
            "background-color: #2a2a2a;"
            "border: 1px solid #555;"
            "border-radius: 6px;"
            "color: #ffffff;"
            "}"
        );

        QVBoxLayout *frequencyLayout = new QVBoxLayout(frequencyGroup);
        frequencyLayout->setSpacing(15);

        // ComboBox principal pour la fréquence
        frequencyCombo = new QComboBox();
        frequencyCombo->setObjectName("frequencyCombo");
        frequencyCombo->addItems({
            "Changement manuel uniquement",
            "1h",
            "3h",
            "6h",
            "12h",
            "24h",
            "7j",
            "Autre"
        });
        frequencyCombo->setFixedHeight(32);
        frequencyCombo->setStyleSheet(
            "QComboBox {"
            "border: 1px solid #555;"
            "border-radius: 8px;"
            "padding: 8px 16px;"
            "background: #2a2a2a;"
            "font-size: 13px;"
            "color: #ffffff;"
            "selection-background-color: #0078d4;"
            "}"
            "QComboBox:hover {"
            "border-color: #777;"
            "background: #333;"
            "}"
            "QComboBox:focus {"
            "border-color: #0078d4;"
            "outline: none;"
            "box-shadow: 0 0 0 3px rgba(0, 120, 212, 0.3);"
            "}"
            "QComboBox::drop-down {"
            "subcontrol-origin: padding;"
            "subcontrol-position: top right;"
            "width: 30px;"
            "border-left: 1px solid #555;"
            "border-top-right-radius: 8px;"
            "border-bottom-right-radius: 8px;"
            "background: #444;"
            "}"
            "QComboBox::drop-down:hover {"
            "background: #555;"
            "}"
            "QComboBox::down-arrow {"
            "image: none;"
            "border: 2px solid #ccc;"
            "width: 6px;"
            "height: 6px;"
            "border-top: none;"
            "border-left: none;"
            "margin-top: -2px;"
            "transform: rotate(45deg);"
            "}"
            "QComboBox QAbstractItemView {"
            "border: 1px solid #555;"
            "border-radius: 6px;"
            "background: #2a2a2a;"
            "outline: none;"
            "selection-background-color: #0078d4;"
            "selection-color: #ffffff;"
            "color: #ffffff;"
            "padding: 4px;"
            "}"
        );

        frequencyLayout->addWidget(frequencyCombo);

        // Connecter au détecteur de changements
        connect(frequencyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModernWindow::onSettingsChanged);

        // Widget pour l'option "Autre" (masqué par défaut)
        QWidget *customFrequencyWidget = new QWidget();
        customFrequencyWidget->setObjectName("customFrequencyWidget");
        customFrequencyWidget->setStyleSheet("QWidget { background-color: #3a3a3a; }");
        customFrequencyWidget->hide();

        QHBoxLayout *customLayout = new QHBoxLayout(customFrequencyWidget);
        customLayout->setContentsMargins(0, 0, 0, 0);
        customLayout->setSpacing(10);

        // SpinBox pour la valeur numérique
        customValueSpinBox = new QSpinBox();
        customValueSpinBox->setObjectName("customValueSpinBox");
        customValueSpinBox->setMinimum(1);
        customValueSpinBox->setMaximum(9999);
        customValueSpinBox->setValue(30);
        customValueSpinBox->setFixedHeight(32);
        customValueSpinBox->setFixedWidth(100);
        customValueSpinBox->setStyleSheet(
            "QSpinBox {"
            "border: 1px solid #555;"
            "border-radius: 8px;"
            "padding: 8px 12px;"
            "background: #2a2a2a;"
            "font-size: 13px;"
            "color: #ffffff;"
            "}"
            "QSpinBox:hover {"
            "border-color: #777;"
            "background: #333;"
            "}"
            "QSpinBox:focus {"
            "border-color: #0078d4;"
            "outline: none;"
            "box-shadow: 0 0 0 3px rgba(0, 120, 212, 0.3);"
            "}"
            "QSpinBox::up-button {"
            "subcontrol-origin: border;"
            "subcontrol-position: top right;"
            "width: 25px;"
            "border-left: 1px solid #555;"
            "border-top-right-radius: 8px;"
            "background: #444;"
            "}"
            "QSpinBox::up-button:hover {"
            "background: #555;"
            "}"
            "QSpinBox::down-button {"
            "subcontrol-origin: border;"
            "subcontrol-position: bottom right;"
            "width: 25px;"
            "border-left: 1px solid #555;"
            "border-bottom-right-radius: 8px;"
            "background: #444;"
            "}"
            "QSpinBox::down-button:hover {"
            "background: #555;"
            "}"
            "QSpinBox::up-arrow {"
            "image: none;"
            "border: 2px solid #ccc;"
            "width: 4px;"
            "height: 4px;"
            "border-bottom: none;"
            "border-right: none;"
            "margin-bottom: -1px;"
            "transform: rotate(45deg);"
            "}"
            "QSpinBox::down-arrow {"
            "image: none;"
            "border: 2px solid #ccc;"
            "width: 4px;"
            "height: 4px;"
            "border-top: none;"
            "border-left: none;"
            "margin-top: -1px;"
            "transform: rotate(45deg);"
            "}"
            "QSpinBox:disabled {"
            "border-color: #333;"
            "background: #1a1a1a;"
            "color: #555;"
            "}"
            "QSpinBox::up-button:disabled {"
            "background: #222;"
            "border-left-color: #333;"
            "}"
            "QSpinBox::down-button:disabled {"
            "background: #222;"
            "border-left-color: #333;"
            "}"
            "QSpinBox::up-arrow:disabled {"
            "border-color: #555;"
            "}"
            "QSpinBox::down-arrow:disabled {"
            "border-color: #555;"
            "}"
        );

        // ComboBox pour l'unité de temps
        customUnitCombo = new QComboBox();
        customUnitCombo->setObjectName("customUnitCombo");
        customUnitCombo->addItems({"minutes", "heures", "jours"});
        customUnitCombo->setFixedHeight(32);
        customUnitCombo->setFixedWidth(120);
        customUnitCombo->setStyleSheet(
            "QComboBox {"
            "border: 1px solid #555;"
            "border-radius: 8px;"
            "padding: 8px 16px;"
            "background: #2a2a2a;"
            "font-size: 13px;"
            "color: #ffffff;"
            "selection-background-color: #0078d4;"
            "}"
            "QComboBox:hover {"
            "border-color: #777;"
            "background: #333;"
            "}"
            "QComboBox:focus {"
            "border-color: #0078d4;"
            "outline: none;"
            "box-shadow: 0 0 0 3px rgba(0, 120, 212, 0.3);"
            "}"
            "QComboBox::drop-down {"
            "subcontrol-origin: padding;"
            "subcontrol-position: top right;"
            "width: 25px;"
            "border-left: 1px solid #555;"
            "border-top-right-radius: 8px;"
            "border-bottom-right-radius: 8px;"
            "background: #444;"
            "}"
            "QComboBox::drop-down:hover {"
            "background: #555;"
            "}"
            "QComboBox::down-arrow {"
            "image: none;"
            "border: 2px solid #ccc;"
            "width: 6px;"
            "height: 6px;"
            "border-top: none;"
            "border-left: none;"
            "margin-top: -2px;"
            "transform: rotate(45deg);"
            "}"
            "QComboBox QAbstractItemView {"
            "border: 1px solid #555;"
            "border-radius: 6px;"
            "background: #2a2a2a;"
            "outline: none;"
            "selection-background-color: #0078d4;"
            "selection-color: #ffffff;"
            "color: #ffffff;"
            "padding: 4px;"
            "}"
            "QComboBox:disabled {"
            "border-color: #333;"
            "background: #1a1a1a;"
            "color: #555;"
            "}"
            "QComboBox::drop-down:disabled {"
            "background: #222;"
            "border-left-color: #333;"
            "}"
            "QComboBox::down-arrow:disabled {"
            "border-color: #555;"
            "}"
        );

        customLayout->addWidget(customValueSpinBox);
        customLayout->addWidget(customUnitCombo);
        customLayout->addStretch();

        // Connecter les contrôles personnalisés au détecteur de changements
        connect(customValueSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ModernWindow::onSettingsChanged);
        connect(customUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModernWindow::onSettingsChanged);

        frequencyLayout->addWidget(customFrequencyWidget);

        // S'assurer que le widget est visible et initialiser les contrôles comme désactivés
        customFrequencyWidget->show();
        customValueSpinBox->setEnabled(false);
        customUnitCombo->setEnabled(false);

        // Connexion pour activer/désactiver l'option personnalisée
        connect(frequencyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
            if (index == 7) { // Index de "Autre"
                // Activer les contrôles
                this->customValueSpinBox->setEnabled(true);
                this->customUnitCombo->setEnabled(true);
            } else {
                // Désactiver les contrôles sans les vider
                this->customValueSpinBox->setEnabled(false);
                this->customUnitCombo->setEnabled(false);
            }
        });

        // Layout horizontal principal pour organiser en deux colonnes
        QHBoxLayout *mainSettingsLayout = new QHBoxLayout();
        mainSettingsLayout->setSpacing(20);

        // Colonne de gauche (Fréquence + Options système)
        QVBoxLayout *leftColumnLayout = new QVBoxLayout();
        leftColumnLayout->setSpacing(15);

        leftColumnLayout->addWidget(frequencyGroup);

        // Groupe Mode d'ajustement de l'image
        QGroupBox *adjustmentGroup = new QGroupBox("Mode d'ajustement de l'image");
        adjustmentGroup->setMinimumHeight(320); // Hauteur minimale pour alignement avec les deux cadres de gauche
        adjustmentGroup->setFixedWidth(280); // Largeur fixe pour éviter les variations selon le contenu
        adjustmentGroup->setStyleSheet(
            "QGroupBox {"
            "font-weight: 600;"
            "font-size: 14px;"
            "color: #ffffff;"
            "border: 1px solid #555;"
            "border-radius: 12px;"
            "margin-top: 15px;"
            "padding-top: 15px;"
            "background-color: #3a3a3a;"
            "}"
            "QGroupBox::title {"
            "subcontrol-origin: margin;"
            "subcontrol-position: top left;"
            "left: 15px;"
            "padding: 5px 12px;"
            "background-color: #2a2a2a;"
            "border: 1px solid #555;"
            "border-radius: 6px;"
            "color: #ffffff;"
            "}"
        );

        QVBoxLayout *adjustmentLayout = new QVBoxLayout(adjustmentGroup);
        adjustmentLayout->setSpacing(10);
        adjustmentLayout->setContentsMargins(10, 15, 10, 15);

        // Encadré d'explication en haut (pleine largeur)
        QWidget *explanationWidget = new QWidget();
        explanationWidget->setFixedHeight(80); // Hauteur réduite pour l'explication en haut
        explanationWidget->setStyleSheet(
            "QWidget {"
            "background-color: rgba(33, 150, 243, 30);" // #2196F3 avec transparence
            "border: 2px solid #2196F3;"
            "border-radius: 8px;"
            "}"
        );

        QVBoxLayout *explanationLayout = new QVBoxLayout(explanationWidget);
        explanationLayout->setContentsMargins(10, 5, 10, 10); // Réduction du padding top

        adjustmentExplanationLabel = new QLabel();
        adjustmentExplanationLabel->setObjectName("adjustmentExplanationLabel");
        adjustmentExplanationLabel->setWordWrap(true);
        adjustmentExplanationLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        adjustmentExplanationLabel->setStyleSheet(
            "QLabel {"
            "color: #ffffff;"
            "font-size: 13px;"
            "background-color: transparent;"
            "border: none;"
            "padding: 3px 5px;"
            "}"
        );

        // Texte par défaut pour "Remplir"
        adjustmentExplanationLabel->setText("L'image conserve son ratio mais dépasse de l'écran pour le remplir");

        explanationLayout->addWidget(adjustmentExplanationLabel);

        // Layout horizontal pour centrer le sélecteur (image + combobox)
        QHBoxLayout *selectorContainerLayout = new QHBoxLayout();
        selectorContainerLayout->setAlignment(Qt::AlignCenter);

        // Widget container pour le sélecteur et l'image (partie gauche)
        QWidget *selectorWidget = new QWidget();
        selectorWidget->setStyleSheet("QWidget { background-color: #3a3a3a; }");
        selectorWidget->setFixedWidth(160);
        QVBoxLayout *selectorLayout = new QVBoxLayout(selectorWidget);
        selectorLayout->setSpacing(15);
        selectorLayout->setAlignment(Qt::AlignCenter);

        // ComboBox pour sélectionner le mode
        adjustmentCombo = new QComboBox();
        adjustmentCombo->setObjectName("adjustmentCombo");
        adjustmentCombo->addItem("Remplir", "fill");
        adjustmentCombo->addItem("Ajuster", "fit");
        adjustmentCombo->addItem("Étendre", "span");
        adjustmentCombo->addItem("Étirer", "stretch");
        adjustmentCombo->addItem("Mosaïque", "tile");
        adjustmentCombo->setFixedHeight(32);
        adjustmentCombo->setFixedWidth(140);
        adjustmentCombo->setStyleSheet(
            "QComboBox {"
            "border: 1px solid #555;"
            "border-radius: 8px;"
            "padding: 8px 16px;"
            "background: #2a2a2a;"
            "font-size: 13px;"
            "color: #ffffff;"
            "selection-background-color: #0078d4;"
            "}"
            "QComboBox:hover {"
            "border-color: #777;"
            "background: #333;"
            "}"
            "QComboBox:focus {"
            "border-color: #0078d4;"
            "outline: none;"
            "box-shadow: 0 0 0 3px rgba(0, 120, 212, 0.3);"
            "}"
            "QComboBox::drop-down {"
            "subcontrol-origin: padding;"
            "subcontrol-position: top right;"
            "width: 30px;"
            "border-left: 1px solid #555;"
            "border-top-right-radius: 8px;"
            "border-bottom-right-radius: 8px;"
            "background: #444;"
            "}"
            "QComboBox::drop-down:hover {"
            "background: #555;"
            "}"
            "QComboBox::down-arrow {"
            "image: none;"
            "border: 2px solid #ccc;"
            "width: 6px;"
            "height: 6px;"
            "border-top: none;"
            "border-left: none;"
            "margin-top: -2px;"
            "transform: rotate(45deg);"
            "}"
            "QComboBox QAbstractItemView {"
            "border: 1px solid #555;"
            "border-radius: 6px;"
            "background: #2a2a2a;"
            "outline: none;"
            "selection-background-color: #0078d4;"
            "selection-color: #ffffff;"
            "color: #ffffff;"
            "padding: 4px;"
            "}"
        );

        // Label pour afficher l'image du mode sélectionné
        QLabel *adjustmentImageLabel = new QLabel();
        adjustmentImageLabel->setObjectName("adjustmentImageLabel");
        adjustmentImageLabel->setFixedSize(140, 120);
        adjustmentImageLabel->setAlignment(Qt::AlignCenter);
        adjustmentImageLabel->setStyleSheet(
            "QLabel {"
            "border: 1px solid #555;"
            "border-radius: 8px;"
            "background-color: #e8e8e8;"
            "padding-top: 0px;"
            "}"
        );

        // Afficher l'image par défaut (fill)
        QPixmap defaultPixmap(getImagePath("wallpaper_fill_icon.png"));
        if (!defaultPixmap.isNull()) {
            // Utiliser une taille beaucoup plus grande pour remplir vraiment l'espace
            adjustmentImageLabel->setPixmap(defaultPixmap.scaled(136, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            adjustmentImageLabel->setText("Remplir");
            adjustmentImageLabel->setStyleSheet("QLabel { border: 1px solid #555; border-radius: 8px; background-color: #e8e8e8; padding: 0px; color: #333333; font-weight: bold; }");
        }

        // Connecter le changement de sélection
        connect(adjustmentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, adjustmentImageLabel](int index) {
            QString mode = this->adjustmentCombo->itemData(index).toString();
            QPixmap modePixmap(getImagePath(QString("wallpaper_%1_icon.png").arg(mode)));
            if (!modePixmap.isNull()) {
                // Utiliser une taille beaucoup plus grande pour remplir vraiment l'espace
                adjustmentImageLabel->setPixmap(modePixmap.scaled(136, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                adjustmentImageLabel->setText(adjustmentCombo->currentText());
                adjustmentImageLabel->setStyleSheet("QLabel { border: 1px solid #555; border-radius: 8px; background-color: #e8e8e8; padding: 0px; color: #333333; font-weight: bold; }");
            }

            // Mettre à jour l'explication selon le mode sélectionné
            updateAdjustmentExplanation(index);
        });

        selectorLayout->addWidget(adjustmentCombo, 0, Qt::AlignCenter);
        selectorLayout->addWidget(adjustmentImageLabel, 0, Qt::AlignCenter);

        // Connecter au détecteur de changements (en plus de la connexion existante)
        connect(adjustmentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModernWindow::onSettingsChanged);

        // Ajouter le sélecteur centré au layout horizontal
        selectorContainerLayout->addWidget(selectorWidget);

        // Ajouter le layout horizontal centré au layout principal vertical
        adjustmentLayout->addLayout(selectorContainerLayout);

        // Ajouter l'encadré d'explication en bas
        adjustmentLayout->addWidget(explanationWidget);

        // Groupe Options système
        QGroupBox *systemGroup = new QGroupBox("Options système");
        systemGroup->setStyleSheet(
            "QGroupBox {"
            "font-weight: 600;"
            "font-size: 14px;"
            "color: #ffffff;"
            "border: 1px solid #555;"
            "border-radius: 12px;"
            "margin-top: 15px;"
            "padding-top: 15px;"
            "background-color: #3a3a3a;"
            "}"
            "QGroupBox::title {"
            "subcontrol-origin: margin;"
            "subcontrol-position: top left;"
            "left: 15px;"
            "padding: 5px 12px;"
            "background-color: #2a2a2a;"
            "border: 1px solid #555;"
            "border-radius: 6px;"
            "color: #ffffff;"
            "}"
        );

        QVBoxLayout *systemLayout = new QVBoxLayout(systemGroup);
        systemLayout->setSpacing(15);
        systemLayout->setContentsMargins(15, 15, 15, 15);

        // Option "Démarrer avec Windows"
        QHBoxLayout *startupLayout = new QHBoxLayout();
        QLabel *startupLabel = new QLabel("Démarrer avec Windows");
        startupLabel->setStyleSheet("color: #ffffff; font-size: 13px; background-color: transparent;");

        startupToggle = new ToggleSwitch();
        startupToggle->setObjectName("startupToggle");
        startupToggle->setChecked(true); // Activé par défaut

        startupLayout->addWidget(startupLabel);
        startupLayout->addStretch();
        startupLayout->addWidget(startupToggle);

        systemLayout->addLayout(startupLayout);

        // Option "Changer le fond d'écran au démarrage"
        QHBoxLayout *changeOnStartupLayout = new QHBoxLayout();
        QLabel *changeOnStartupLabel = new QLabel("Changer le fond d'écran\nau démarrage de l'ordinateur");
        changeOnStartupLabel->setWordWrap(true);
        changeOnStartupLabel->setStyleSheet("color: #ffffff; font-size: 13px; background-color: transparent;");

        changeOnStartupToggle = new ToggleSwitch();
        changeOnStartupToggle->setObjectName("changeOnStartupToggle");
        changeOnStartupToggle->setChecked(false); // Désactivé par défaut

        changeOnStartupLayout->addWidget(changeOnStartupLabel);
        changeOnStartupLayout->addStretch();
        changeOnStartupLayout->addWidget(changeOnStartupToggle);

        systemLayout->addLayout(changeOnStartupLayout);

        // Option "Image différente sur chaque écran"
        QHBoxLayout *multiScreenLayout = new QHBoxLayout();
        QLabel *multiScreenLabel = new QLabel("Image différente sur chaque écran");
        multiScreenLabel->setStyleSheet("color: #ffffff; font-size: 13px; background-color: transparent;");

        multiScreenToggle = new ToggleSwitch();
        multiScreenToggle->setObjectName("multiScreenToggle");
        multiScreenToggle->setChecked(true); // Activé par défaut

        multiScreenLayout->addWidget(multiScreenLabel);
        multiScreenLayout->addStretch();
        multiScreenLayout->addWidget(multiScreenToggle);

        systemLayout->addLayout(multiScreenLayout);

        // Connecter les signaux
        connect(startupToggle, &ToggleSwitch::toggled, this, &ModernWindow::onSettingsChanged);
        connect(startupToggle, &ToggleSwitch::toggled, this, &ModernWindow::onStartupToggled);
        connect(changeOnStartupToggle, &ToggleSwitch::toggled, this, &ModernWindow::onSettingsChanged);
        connect(multiScreenToggle, &ToggleSwitch::toggled, this, &ModernWindow::onSettingsChanged);
        connect(multiScreenToggle, &ToggleSwitch::toggled, this, &ModernWindow::onMultiScreenToggled);

        leftColumnLayout->addWidget(systemGroup);
        leftColumnLayout->addStretch();

        // Colonne de droite (Mode d'ajustement de l'image)
        QVBoxLayout *rightColumnLayout = new QVBoxLayout();
        rightColumnLayout->addWidget(adjustmentGroup);

        // Ajouter les deux colonnes au layout principal
        mainSettingsLayout->addLayout(leftColumnLayout);
        mainSettingsLayout->addLayout(rightColumnLayout);

        settingsLayout->addLayout(mainSettingsLayout);
        settingsLayout->addStretch(); // Pousser le bouton tout en bas

        // Bouton Appliquer tout en bas à droite de l'onglet Paramètres
        QHBoxLayout *applyLayout = new QHBoxLayout();
        applyLayout->addStretch();

        applyButton = new QPushButton("Appliquer");
        applyButton->setObjectName("applyButton");
        applyButton->setFixedSize(120, 40);
        applyButton->setEnabled(false); // Désactivé par défaut
        applyButton->setStyleSheet(
            "QPushButton {"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4CAF50, stop:1 #45a049);"
            "border: none;"
            "border-radius: 8px;"
            "color: white;"
            "font-size: 13px;"
            "font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #5CBF60, stop:1 #4CAF50);"
            "}"
            "QPushButton:pressed {"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3d8b40, stop:1 #2e7d32);"
            "}"
            "QPushButton:disabled {"
            "background: #666666;"
            "color: #999999;"
            "}"
        );

        connect(applyButton, &QPushButton::clicked, this, &ModernWindow::onApplySettings);

        applyLayout->addWidget(applyButton);

        settingsLayout->addLayout(applyLayout);

        tabWidget->addTab(settingsTab, "Paramètres");
    }
    
    void applyModernStyle()
    {
        setStyleSheet(R"(
            QWidget {
                background-color: #2b2b2b;
                color: #ffffff;
                font-family: 'Segoe UI', Arial, sans-serif;
                font-size: 10pt;
            }
            
            #titleLabel {
                font-size: 18pt;
                font-weight: bold;
                color: #ffffff;
                margin-bottom: 10px;
            }
            
            #scrollArea {
                border: 1px solid #555;
                border-radius: 8px;
                background-color: #333;
            }
            
            #categoryFrame {
                background-color: #404040;
                border: 1px solid #555;
                border-radius: 12px;
                padding: 15px;
                margin: 5px;
            }
            
            #categoryFrame:hover {
                background-color: #4a4a4a;
                border-color: #777;
            }
            
            #categoryName {
                font-size: 12pt;
                font-weight: bold;
                color: #ffffff;
                text-align: center;
            }
            
            #thumbnailLabel {
                background-color: transparent;
                border: none;
            }

            QTabWidget::pane {
                border: 1px solid #555;
                border-radius: 8px;
                background-color: #2b2b2b;
                margin-top: -1px;
            }

            QTabBar::tab {
                background-color: #404040;
                border: 1px solid #555;
                padding: 8px 16px;
                margin-right: 2px;
                border-top-left-radius: 8px;
                border-top-right-radius: 8px;
                color: #cccccc;
                font-size: 11pt;
                width: 120px;
            }

            QTabBar::tab:hover {
                background-color: #4a4a4a;
                color: #ffffff;
            }

            QTabBar::tab:selected {
                background-color: #0078d4;
                color: #ffffff;
                border-color: #0078d4;
                font-weight: bold;
            }

            QTabBar::tab:!selected {
                margin-top: 2px;
            }
        )");
    }
    
    void setupSystemTray()
    {
        // Créer l'icône du system tray
        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setIcon(QIcon(getImagePath("icon.png")));
        trayIcon->setToolTip("WallpaperAI");

        // Créer le menu contextuel
        trayMenu = new QMenu(this);

        QAction *showAction = new QAction("Afficher", this);
        QAction *quitAction = new QAction("Quitter", this);

        trayMenu->addAction(showAction);
        trayMenu->addSeparator();
        trayMenu->addAction(quitAction);

        trayIcon->setContextMenu(trayMenu);

        // Connecter les signaux
        connect(showAction, &QAction::triggered, this, &ModernWindow::restoreWindow);
        connect(quitAction, &QAction::triggered, this, &ModernWindow::quitApplication);
        connect(trayIcon, &QSystemTrayIcon::activated, this, &ModernWindow::onTrayIconActivated);

        // Afficher l'icône du tray
        trayIcon->show();
    }

protected:
    void changeEvent(QEvent *event) override
    {
        if (event->type() == QEvent::WindowStateChange) {
            if (isMinimized()) {
                // Masquer la fenêtre quand elle est minimisée
                hide();
                event->ignore();
                return;
            }
        }
        QWidget::changeEvent(event);
    }

    void closeEvent(QCloseEvent *event) override
    {
        // Fermer vraiment l'application
        if (trayIcon) {
            trayIcon->hide();
        }
        event->accept();
        QApplication::quit();
    }

#ifdef Q_OS_WIN
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override
    {
        if (eventType == "windows_generic_MSG") {
            MSG *msg = static_cast<MSG *>(message);

            // Détecter les changements de configuration d'écran
            if (msg->message == WM_DISPLAYCHANGE || msg->message == WM_DEVICECHANGE) {
                // Utiliser un timer unique pour éviter les appels multiples rapides
                if (!m_displayChangeTimer) {
                    m_displayChangeTimer = new QTimer(this);
                    m_displayChangeTimer->setSingleShot(true);
                    m_displayChangeTimer->setInterval(500); // 500ms de délai

                    connect(m_displayChangeTimer, &QTimer::timeout, this, &ModernWindow::onDisplayConfigurationChanged);
                }

                // Redémarrer le timer à chaque événement (cela annule les précédents)
                m_displayChangeTimer->start();
            }
        }

        return QWidget::nativeEvent(eventType, message, result);
    }
#endif

    void setupSslErrorHandling(QNetworkReply *reply)
    {
        connect(reply, QOverload<const QList<QSslError>&>::of(&QNetworkReply::sslErrors),
                [reply](const QList<QSslError> &errors) {
            reply->ignoreSslErrors();
        });
    }

    void loadCategories()
    {
        // Charger le cache existant
        loadCategoriesCache();

        // Afficher immédiatement le cache si disponible
        if (!cachedCategories.isEmpty()) {
            displayCategories(cachedCategories);
        }

        // Tenter de charger depuis l'API en arrière-plan
        QNetworkRequest request(QUrl("https://kazflow.com/api/categories"));
        QNetworkReply *reply = networkManager->get(request);
        setupSslErrorHandling(reply);

        connect(reply, &QNetworkReply::finished, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonObject obj = doc.object();

                if (obj["success"].toBool()) {
                    QJsonArray apiCategories = obj["data"].toArray();

                    // Vérifier s'il y a de nouvelles catégories
                    if (cachedCategories.isEmpty() || apiCategories.size() != cachedCategories.size()) {
                        cachedCategories = apiCategories;
                        saveCategoriesCache();

                        // Effacer l'affichage actuel et réafficher avec les nouvelles catégories
                        clearCategoriesDisplay();
                        displayCategories(apiCategories);
                    }
                }
            } else {
                qDebug() << "Erreur API categories:" << reply->errorString();
            }
            reply->deleteLater();
        });
    }

    void loadCategoriesCache()
    {
        QFile cacheFile(categoriesCachePath);
        if (cacheFile.exists() && cacheFile.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(cacheFile.readAll());
            QJsonObject cacheObj = doc.object();

            cachedCategories = cacheObj["categories"].toArray();

            // Charger aussi le cache des miniatures
            QJsonObject thumbnails = cacheObj["thumbnails"].toObject();
            categoryThumbnailCache.clear();
            for (auto it = thumbnails.begin(); it != thumbnails.end(); ++it) {
                categoryThumbnailCache[it.key()] = it.value().toString();
            }

            cacheFile.close();
        }
    }

    void saveCategoriesCache()
    {
        QFile cacheFile(categoriesCachePath);
        if (cacheFile.open(QIODevice::WriteOnly)) {
            // Sauvegarder les catégories ET les filenames des miniatures
            QJsonObject cacheObj;
            cacheObj["categories"] = cachedCategories;

            // Sauvegarder le cache des miniatures
            QJsonObject thumbnails;
            for (auto it = categoryThumbnailCache.constBegin(); it != categoryThumbnailCache.constEnd(); ++it) {
                thumbnails[it.key()] = it.value();
            }
            cacheObj["thumbnails"] = thumbnails;

            QJsonDocument doc(cacheObj);
            cacheFile.write(doc.toJson());
            cacheFile.close();
        }
    }

    void displayCategoriesFromCache()
    {
        if (!cachedCategories.isEmpty()) {
            displayCategories(cachedCategories);
        }
    }

    void clearCategoriesDisplay()
    {
        // Supprimer tous les widgets du layout
        QLayoutItem *item;
        while ((item = categoriesGridLayout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                delete item->widget();
            }
            delete item;
        }
    }
    
    void displayCategories(const QJsonArray &categories)
    {
        int row = 0, col = 0;
        const int maxCols = 3;

        for (const QJsonValue &value : categories) {
            QJsonObject category = value.toObject();
            QString categoryName = category["name"].toString();
            QString categoryId = category["id"].toString();
            QString thumbnail = category["thumbnail"].toString();

            createCategoryWidget(categoryName, categoryId, thumbnail, row, col);

            col++;
            if (col >= maxCols) {
                col = 0;
                row++;
            }
        }
    }
    
    void createCategoryWidget(const QString &name, const QString &id, const QString &thumbnail, int row, int col)
    {
        QFrame *categoryFrame = new QFrame();
        categoryFrame->setObjectName("categoryFrame");
        categoryFrame->setFixedSize(205, 155);
        categoryFrame->setCursor(Qt::PointingHandCursor);

        QVBoxLayout *frameLayout = new QVBoxLayout(categoryFrame);
        frameLayout->setContentsMargins(1, 1, 1, 1);
        frameLayout->setSpacing(5);

        // Container pour la miniature et les contrôles de notation
        QWidget *thumbnailContainer = new QWidget();
        thumbnailContainer->setFixedSize(165, 90);

        // Label pour la miniature (placeholder pour l'instant)
        QLabel *thumbnailLabel = new QLabel(thumbnailContainer);
        thumbnailLabel->setObjectName("thumbnailLabel");
        thumbnailLabel->setFixedSize(165, 90);
        thumbnailLabel->setAlignment(Qt::AlignCenter);
        thumbnailLabel->setText("Miniature\n" + name);
        thumbnailLabel->setWordWrap(true);

        // Système de notation superposé
        createRatingSystem(thumbnailContainer, id);

        // Label pour le nom de la catégorie
        QLabel *nameLabel = new QLabel(name);
        nameLabel->setObjectName("categoryName");
        nameLabel->setFixedWidth(165);
        nameLabel->setWordWrap(true);
        nameLabel->setAlignment(Qt::AlignCenter);

        frameLayout->addWidget(thumbnailContainer, 0, Qt::AlignCenter);
        frameLayout->addWidget(nameLabel, 0, Qt::AlignCenter);

        // Événements de survol pour afficher/masquer les contrôles
        categoryFrame->installEventFilter(new CategoryHoverFilter(categoryFrame, id, this));

        categoriesGridLayout->addWidget(categoryFrame, row, col);

        // Charger la miniature directement depuis le filename fourni par l'API
        if (!thumbnail.isEmpty()) {
            // Sauvegarder le filename dans le cache
            categoryThumbnailCache[id] = thumbnail;
            saveCategoriesCache();

            // Charger l'image de la miniature
            loadThumbnailImage(thumbnail, thumbnailLabel);
        }
    }

    void createRatingSystem(QWidget *parent, const QString &categoryId)
    {
        // Widget pour l'affichage des étoiles (toujours visible)
        QWidget *currentRating = new QWidget(parent);
        currentRating->setObjectName("currentRating_" + categoryId);
        currentRating->setGeometry(5, 5, 60, 20);
        currentRating->setStyleSheet("background: transparent;");
        currentRating->show(); // S'assurer que le widget est visible

        // Utiliser la notation sauvegardée ou 1 par défaut si aucune sauvegarde
        int savedRating = categoryRatings.value(categoryId, 1);
        categoryRatings[categoryId] = savedRating;
        setProperty(("rating_" + categoryId).toLocal8Bit().constData(), savedRating);

        // Appeler updateCategoryRatingDisplay après que tous les widgets soient configurés
        QTimer::singleShot(0, [this, categoryId, savedRating]() {
            updateCategoryRatingDisplay(categoryId, savedRating);
        });

        // Widget pour les contrôles interactifs (masqué par défaut)
        QWidget *ratingWidget = new QWidget(parent);
        ratingWidget->setObjectName("ratingWidget_" + categoryId);
        ratingWidget->setGeometry(5, 5, 80, 20);
        ratingWidget->setStyleSheet("background: rgba(0,0,0,0.8); border-radius: 3px;");
        ratingWidget->hide();

        QHBoxLayout *ratingLayout = new QHBoxLayout(ratingWidget);
        ratingLayout->setContentsMargins(2, 2, 2, 2);
        ratingLayout->setSpacing(2);

        // Bouton "sens interdit"
        QPushButton *disableBtn = new QPushButton();
        disableBtn->setFixedSize(16, 16);
        disableBtn->setObjectName("disable_" + categoryId);

        QPixmap disablePixmap(getImagePath("disable_category.png"));
        if (!disablePixmap.isNull()) {
            disableBtn->setIcon(QIcon(disablePixmap));
        } else {
            disableBtn->setText("X");
        }
        disableBtn->setIconSize(QSize(16, 16));
        disableBtn->setStyleSheet("QPushButton { border: none; background: transparent; color: white; } QPushButton:hover { background: rgba(255,255,255,0.2); }");

        // Événements de survol et clic pour le sens interdit
        connect(disableBtn, &QPushButton::clicked, [this, categoryId]() { setCategoryRating(categoryId, -1); });

        // Installer un filtre d'événements pour gérer le survol du bouton sens interdit
        disableBtn->installEventFilter(new DisableHoverFilter(disableBtn, categoryId, this));

        ratingLayout->addWidget(disableBtn);

        // 3 étoiles interactives
        for (int i = 1; i <= 3; i++) {
            QPushButton *starBtn = new QPushButton();
            starBtn->setFixedSize(16, 16);
            starBtn->setObjectName(QString("star_%1_%2").arg(categoryId).arg(i));

            QPixmap starPixmap(getImagePath("star_inactive.png"));
            if (!starPixmap.isNull()) {
                starBtn->setIcon(QIcon(starPixmap));
            } else {
                starBtn->setText("★");
            }
            starBtn->setIconSize(QSize(16, 16));
            starBtn->setStyleSheet("QPushButton { border: none; background: transparent; color: white; } QPushButton:hover { background: rgba(255,255,255,0.2); }");

            // Événements de survol pour preview
            connect(starBtn, &QPushButton::clicked, [this, categoryId, i]() { setCategoryRating(categoryId, i); });

            // Installer un filtre d'événements pour gérer le survol
            starBtn->installEventFilter(new StarHoverFilter(starBtn, categoryId, i, this));

            ratingLayout->addWidget(starBtn);
        }
    }

    void setCategoryRating(const QString &categoryId, int rating)
    {
        // Sauvegarder la notation (ici on pourrait l'enregistrer dans un fichier/DB)
        categoryRatings[categoryId] = rating;

        // Mettre à jour la propriété dynamique pour les filtres d'événements
        setProperty(("rating_" + categoryId).toLocal8Bit().constData(), rating);

        // Mettre à jour l'affichage permanent
        updateCategoryRatingDisplay(categoryId, rating);

        // Sauvegarder immédiatement la notation
        saveCategoryRating(categoryId, rating);

        // Mettre à jour aussi les étoiles interactives si elles sont visibles
        QWidget *ratingWidget = findChild<QWidget*>("ratingWidget_" + categoryId);
        if (ratingWidget && ratingWidget->isVisible()) {
            for (int i = 1; i <= 3; i++) {
                QPushButton *starBtn = ratingWidget->findChild<QPushButton*>(QString("star_%1_%2").arg(categoryId).arg(i));
                if (starBtn) {
                    if (i <= rating) {
                        QPixmap starPixmap(getImagePath("star_active.png"));
                        if (!starPixmap.isNull()) {
                            starBtn->setIcon(QIcon(starPixmap));
                        } else {
                            starBtn->setText("★");
                            starBtn->setStyleSheet("QPushButton { border: none; background: transparent; color: gold; } QPushButton:hover { background: rgba(255,255,255,0.2); }");
                        }
                    } else {
                        QPixmap starPixmap(getImagePath("star_inactive.png"));
                        if (!starPixmap.isNull()) {
                            starBtn->setIcon(QIcon(starPixmap));
                        } else {
                            starBtn->setText("★");
                            starBtn->setStyleSheet("QPushButton { border: none; background: transparent; color: gray; } QPushButton:hover { background: rgba(255,255,255,0.2); }");
                        }
                    }
                }
            }
        }
    }

    void updateCategoryRatingDisplay(const QString &categoryId, int rating)
    {
        QWidget *currentRating = findChild<QWidget*>("currentRating_" + categoryId);
        if (!currentRating) return;

        // Nettoyer l'affichage précédent en vidant le layout
        QLayout* currentLayout = currentRating->layout();
        if (currentLayout) {
            // Vider le layout en supprimant tous les items
            QLayoutItem* item;
            while ((item = currentLayout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->setParent(nullptr);
                    delete item->widget();
                }
                delete item;
            }
            delete currentLayout;
        }

        // Supprimer spécifiquement l'ancienne icône "sens interdit" si elle existe
        QWidget *parentWidget = currentRating->parentWidget();
        if (parentWidget && parentWidget->parentWidget()) {
            QLabel *oldDisableIcon = parentWidget->parentWidget()->findChild<QLabel*>("disableIcon_" + categoryId);
            if (oldDisableIcon) {
                oldDisableIcon->setParent(nullptr);
                delete oldDisableIcon;
            }
        }

        if (rating == -1) {
            // Affichage "sens interdit" au centre de la miniature (plus gros)
            QWidget *parentWidget = currentRating->parentWidget();
            QLabel *disableIcon = new QLabel(parentWidget->parentWidget()); // Remonter d'un niveau pour être au-dessus du label
            disableIcon->setObjectName("disableIcon_" + categoryId); // Nom pour identification
            disableIcon->setGeometry(55, 25, 100, 100); // Position centrée pour une icône plus grande

            QPixmap disablePixmap(getImagePath("disable_category.png"));
            if (!disablePixmap.isNull()) {
                disableIcon->setPixmap(disablePixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                disableIcon->setStyleSheet("background: transparent;"); // Pas de fond sombre
            } else {
                disableIcon->setText("✗");
                disableIcon->setStyleSheet("color: red; font-size: 40px; font-weight: bold; background: transparent;");
                disableIcon->setAlignment(Qt::AlignCenter);
            }
            disableIcon->show();
            disableIcon->raise(); // Mettre au premier plan devant tous les autres widgets

            // Masquer l'indicateur normal
            currentRating->hide();
        } else if (rating > 0) {
            // Afficher l'indicateur normal
            currentRating->show();

            // Affichage des étoiles en haut à gauche
            QHBoxLayout *layout = new QHBoxLayout(currentRating);
            layout->setContentsMargins(3, 2, 3, 2);
            layout->setSpacing(1);

            for (int i = 1; i <= 3; i++) {
                QLabel *star = new QLabel();
                star->setFixedSize(12, 12);

                if (i <= rating) {
                    QPixmap starPixmap(getImagePath("star_active.png"));
                    if (!starPixmap.isNull()) {
                        star->setPixmap(starPixmap.scaled(12, 12, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    } else {
                        star->setText("★");
                        star->setStyleSheet("color: gold; font-size: 10px;");
                        star->setAlignment(Qt::AlignCenter);
                    }
                } else {
                    QPixmap starPixmap(getImagePath("star_inactive.png"));
                    if (!starPixmap.isNull()) {
                        star->setPixmap(starPixmap.scaled(12, 12, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    } else {
                        star->setText("☆");
                        star->setStyleSheet("color: gray; font-size: 10px;");
                        star->setAlignment(Qt::AlignCenter);
                    }
                }
                layout->addWidget(star);
            }
        }
    }
    
    void loadCategoryThumbnail(const QString &categoryId, QLabel *thumbnailLabel)
    {
        // Vérifier d'abord si on a le filename en cache
        if (categoryThumbnailCache.contains(categoryId)) {
            QString cachedFilename = categoryThumbnailCache[categoryId];
            loadThumbnailImage(cachedFilename, thumbnailLabel);
            return;
        }

        // Sinon, charger depuis l'API
        QString currentDate = getCurrentDateString();
        QString url = QString("https://kazflow.com/api/wallpapers?category=%1&date=%2")
                      .arg(categoryId)
                      .arg(QString(currentDate).replace("/", "%2F"));

        QNetworkRequest request{QUrl(url)};
        QNetworkReply *reply = networkManager->get(request);
        setupSslErrorHandling(reply);

        connect(reply, &QNetworkReply::finished, [this, reply, thumbnailLabel, categoryId, currentDate]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray responseData = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(responseData);
                QJsonObject obj = doc.object();

                if (obj["success"].toBool()) {
                    QJsonArray wallpapers = obj["data"].toArray();
                    if (!wallpapers.isEmpty()) {
                        QString filename = wallpapers[0].toObject()["filename"].toString();

                        // Sauvegarder le filename dans le cache
                        categoryThumbnailCache[categoryId] = filename;
                        saveCategoriesCache();

                        loadThumbnailImage(filename, thumbnailLabel);
                    } else {
                        loadCategoryThumbnailFallback(categoryId, thumbnailLabel, 1);
                    }
                } else {
                    loadCategoryThumbnailFallback(categoryId, thumbnailLabel, 1);
                }
            } else {
                loadCategoryThumbnailFallback(categoryId, thumbnailLabel, 1);
            }
            reply->deleteLater();
        });
    }

    void loadCategoryThumbnailFallback(const QString &categoryId, QLabel *thumbnailLabel, int daysBack)
    {
        if (daysBack > 7) {
            return;
        }

        QString previousDate = getPreviousDateString(daysBack);
        QString url = QString("https://kazflow.com/api/wallpapers?category=%1&date=%2")
                      .arg(categoryId)
                      .arg(QString(previousDate).replace("/", "%2F"));

        QNetworkRequest request{QUrl(url)};
        QNetworkReply *reply = networkManager->get(request);
        setupSslErrorHandling(reply);

        connect(reply, &QNetworkReply::finished, [this, reply, thumbnailLabel, categoryId, daysBack, previousDate]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray responseData = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(responseData);
                QJsonObject obj = doc.object();

                if (obj["success"].toBool()) {
                    QJsonArray wallpapers = obj["data"].toArray();
                    if (!wallpapers.isEmpty()) {
                        QString filename = wallpapers[0].toObject()["filename"].toString();

                        // Sauvegarder le filename dans le cache
                        categoryThumbnailCache[categoryId] = filename;
                        saveCategoriesCache();

                        loadThumbnailImage(filename, thumbnailLabel);
                        return;
                    }
                }
            } else {
            }

            // Essayer le jour suivant
            loadCategoryThumbnailFallback(categoryId, thumbnailLabel, daysBack + 1);
            reply->deleteLater();
        });
    }
    
    void loadThumbnailImage(const QString &filename, QLabel *thumbnailLabel)
    {
        // Construire le chemin du cache pour la miniature
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/thumbnails";
        QDir().mkpath(cacheDir);
        QString cachedThumbnailPath = cacheDir + "/" + filename;

        // Vérifier si la miniature est déjà en cache
        if (QFile::exists(cachedThumbnailPath)) {
            QPixmap pixmap(cachedThumbnailPath);
            if (!pixmap.isNull()) {
                QPixmap scaledPixmap = pixmap.scaled(200, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                thumbnailLabel->setPixmap(scaledPixmap);
                thumbnailLabel->setText("");
                return;
            }
        }

        // Télécharger depuis l'API
        QString miniUrl = QString("https://kazflow.com/api/mini/%1").arg(filename);

        QNetworkRequest request{QUrl(miniUrl)};
        QNetworkReply *reply = networkManager->get(request);
        setupSslErrorHandling(reply);

        connect(reply, &QNetworkReply::finished, [this, reply, thumbnailLabel, filename, cachedThumbnailPath]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    // Sauvegarder dans le cache
                    pixmap.save(cachedThumbnailPath);

                    // Afficher la miniature
                    QPixmap scaledPixmap = pixmap.scaled(200, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    thumbnailLabel->setPixmap(scaledPixmap);
                    thumbnailLabel->setText("");

                    // Nettoyer le cache si trop de fichiers (limite 100)
                    cleanupThumbnailCache();
                }
            } else {
                // En cas d'erreur avec /mini/, fallback vers l'ancienne méthode
                loadThumbnailImageFallback(filename, thumbnailLabel, cachedThumbnailPath);
            }
            reply->deleteLater();
        });
    }
    
    void loadThumbnailImageFallback(const QString &filename, QLabel *thumbnailLabel, const QString &cachedThumbnailPath)
    {
        // Méthode de fallback avec l'image complète (pour compatibilité)
        QNetworkRequest request(QUrl(QString("https://kazflow.com/api/get/%1").arg(filename)));
        QNetworkReply *reply = networkManager->get(request);
        setupSslErrorHandling(reply);

        connect(reply, &QNetworkReply::finished, [this, reply, thumbnailLabel, cachedThumbnailPath, filename]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    // Sauvegarder dans le cache
                    pixmap.save(cachedThumbnailPath);

                    // Redimensionner l'image pour la miniature
                    QPixmap scaledPixmap = pixmap.scaled(200, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    thumbnailLabel->setPixmap(scaledPixmap);
                    thumbnailLabel->setText("");

                    // Nettoyer le cache si trop de fichiers (limite 100)
                    cleanupThumbnailCache();
                }
            }
            reply->deleteLater();
        });
    }

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
    {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            if (isVisible()) {
                hide();
            } else {
                restoreWindow();
            }
        }
    }
    
    void restoreWindow()
    {
        // Restaurer la fenêtre et la mettre au premier plan
        setWindowState(windowState() & ~Qt::WindowMinimized);
        show();
        raise();
        activateWindow();
        setFocus();
        // Forcer la fenêtre à être au premier plan sur Windows
        #ifdef Q_OS_WIN
        SetForegroundWindow((HWND)winId());
        #endif
    }
    
    void quitApplication()
    {
        // Supprimer l'icône du tray avant de fermer
        if (trayIcon) {
            trayIcon->hide();
        }
        QApplication::quit();
    }

private slots:
    void onDisplayConfigurationChanged()
    {
        // Protection contre les appels simultanés
        if (m_isHandlingDisplayChange) {
            return;
        }

        m_isHandlingDisplayChange = true;

        #ifdef Q_OS_WIN
        try {
            QList<QScreen*> screens = QGuiApplication::screens();

            if (screens.isEmpty()) {
                m_isHandlingDisplayChange = false;
                return;
            }

            int screenCount = screens.size();

            // Rafraîchir le sélecteur d'écran si présent
            if (screenSelector) {
                try {
                    screenSelector->refresh();
                } catch (...) {
                    // Ignorer les erreurs de refresh
                }
            }

            // Créer une liste pour stocker les chemins des wallpapers actuels
            QMap<int, QString> currentWallpapers;

            // Créer un mapping : ID unique -> index actuel
            // Cela permet de suivre l'écran physique même si son index change
            QMap<QString, int> screenIdToIndex;
            for (int i = 0; i < screenCount; i++) {
                QString uniqueId = getScreenUniqueId(i);
                screenIdToIndex[uniqueId] = i;
            }

            // Lister tous les fichiers de log disponibles
            QString historyDir = getHistoryDirectory();
            QDir dir(historyDir);
            QStringList logFiles = dir.entryList(QStringList() << "*.log", QDir::Files);

            // Pour chaque fichier de log, trouver à quel écran actuel il correspond
            for (const QString &logFileName : logFiles) {
                QString screenId = logFileName;
                screenId.chop(4); // Enlever ".log"

                // Trouver l'index actuel de cet écran
                if (screenIdToIndex.contains(screenId)) {
                    int currentIndex = screenIdToIndex[screenId];

                    // Lire le wallpaper depuis ce fichier de log
                    QString logFilePath = historyDir + "/" + logFileName;
                    QFile logFile(logFilePath);

                    if (logFile.open(QIODevice::ReadOnly)) {
                        QTextStream stream(&logFile);
                        QString lastLine;
                        QString line;

                        while (stream.readLineInto(&line)) {
                            if (!line.isEmpty()) {
                                lastLine = line;
                            }
                        }
                        logFile.close();

                        if (!lastLine.isEmpty()) {
                            QRegularExpression regex(R"(\[([^\]]+)\] \[([^\]]+)\] \[([^\]]+)\] \[([^\]]+)\])");
                            QRegularExpressionMatch match = regex.match(lastLine);
                            if (match.hasMatch()) {
                                QString filename = match.captured(2);
                                QString wallpapersDir = getWallpapersDirectory();
                                QString fullPath = wallpapersDir + "/" + screenId + "/" + filename;

                                if (QFile::exists(fullPath)) {
                                    currentWallpapers[currentIndex] = fullPath;
                                }
                            }
                        }
                    }
                }
            }

            // Cas spécial : si un seul écran, appliquer directement le wallpaper de cet écran
            if (screenCount == 1 && !currentWallpapers.isEmpty()) {
                QString singleWallpaper = currentWallpapers.value(0);
                if (!singleWallpaper.isEmpty() && QFile::exists(singleWallpaper)) {
                    setWallpaperWithSmoothTransition(singleWallpaper);
                }
                m_isHandlingDisplayChange = false;
                return;
            }

            // Si multi-écrans et on a au moins un wallpaper, reconstruire l'image composite
            if (screenCount > 1 && !currentWallpapers.isEmpty()) {
                rebuildCompositeWallpaper(currentWallpapers);
            }

            m_isHandlingDisplayChange = false;
        } catch (const std::exception &e) {
            m_isHandlingDisplayChange = false;
        } catch (...) {
            m_isHandlingDisplayChange = false;
        }
        #endif
    }

    void onScreenSelectionChanged(const QList<int> &selectedScreens)
    {
        // Gérer l'état du bouton et du countdown selon la sélection
        bool hasSelection = selectedScreens.size() > 0;

        // Désactiver/activer le bouton selon la sélection
        changeNowButton->setEnabled(hasSelection);

        // Mettre en pause/reprendre le countdown selon la sélection
        if (countdownWidget) {
            if (hasSelection) {
                countdownWidget->resumeCountdown();
            } else {
                countdownWidget->pauseCountdown();
            }
        }

        // Mettre à jour le statut pour indiquer les écrans sélectionnés
        if (screenSelector->screenCount() > 1) {
            if (selectedScreens.size() == 1) {
                statusLabel->setText(QString("Écran sélectionné : %1").arg(selectedScreens.first() + 1));
            } else if (selectedScreens.size() > 1) {
                QStringList screenNumbers;
                for (int screen : selectedScreens) {
                    screenNumbers.append(QString::number(screen + 1));
                }
                statusLabel->setText(QString("Écrans sélectionnés : %1").arg(screenNumbers.join(", ")));
            } else {
                statusLabel->setText("Aucun écran sélectionné\nSélectionnez au moins un écran");
            }
        }
    }

    void onScreenDeselectionBlocked(int screenIndex)
    {
        // Afficher un message explicatif quand l'utilisateur essaie de désélectionner un écran sans historique
        QString message = QString("L'écran %1 ne peut pas être désélectionné car il n'a pas encore d'historique de fonds d'écran.\n\n"
                                 "Appliquez d'abord au moins un fond d'écran sur cet écran pour pouvoir le désélectionner.")
                         .arg(screenIndex + 1);

        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Écran sans historique");
        msgBox.setText(message);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStyleSheet(
            "QMessageBox {"
            "background-color: #2b2b2b;"
            "color: #ffffff;"
            "}"
            "QMessageBox QPushButton {"
            "background-color: #0078d4;"
            "color: white;"
            "border: none;"
            "border-radius: 4px;"
            "padding: 8px 16px;"
            "min-width: 80px;"
            "}"
            "QMessageBox QPushButton:hover {"
            "background-color: #106ebe;"
            "}"
        );
        msgBox.exec();
    }

    void onChangeNowClicked()
    {
        // Vérifier qu'au moins un écran est sélectionné en mode multi-écran
        if (multiScreenToggle->isChecked() && screenSelector && screenSelector->screenCount() > 1) {
            QList<int> selectedScreens = screenSelector->getSelectedScreens();
            if (selectedScreens.isEmpty()) {
                // Aucun écran sélectionné, ne rien faire
                return;
            }
        }

        // Par défaut, mode manuel si pas défini
        if (currentTriggerMode.isEmpty()) {
            currentTriggerMode = "Manuel";
        }

        // Désactiver le bouton pendant le processus
        changeNowButton->setEnabled(false);
        changeNowButton->setText("🔄 Changement en cours...");

        // Déterminer quels écrans cibler pour de nouvelles images
        QList<int> targetScreens;
        if (multiScreenToggle->isChecked() && screenSelector && screenSelector->screenCount() > 1) {
            // Mode multi-écran : seulement les écrans sélectionnés reçoivent de nouvelles images
            targetScreens = screenSelector->getSelectedScreens();
        } else {
            // Mode classique : tous les écrans reçoivent la même nouvelle image
            for (int i = 0; i < (screenSelector ? screenSelector->screenCount() : 1); i++) {
                targetScreens.append(i);
            }
        }

        if (targetScreens.size() == 1) {
            statusLabel->setText("Récupération d'une image aléatoire...");
        } else {
            statusLabel->setText(QString("Récupération de %1 images aléatoires...").arg(targetScreens.size()));
        }

        getMultipleRandomWallpapers(targetScreens);
    }

private:
    // Structures pour gérer le téléchargement multiple
    struct MultiWallpaperDownload {
        QList<int> targetScreens;
        QMap<int, QString> downloadedImages; // screenIndex -> imagePath (BMP pour Windows)
        QMap<int, QString> originalImages;   // screenIndex -> imagePath original (PNG)
        int pendingDownloads;
        int completedDownloads;
    };
    MultiWallpaperDownload *currentMultiDownload = nullptr;

    // Structure pour une entrée d'historique
    struct HistoryEntry {
        QString filename;
        QDateTime timestamp;
        int screenIndex;
        QString triggerMode;
        QString adjustMode;

        bool operator<(const HistoryEntry &other) const {
            return timestamp > other.timestamp; // Plus récent en premier
        }
    };

    void rebuildCompositeWallpaper(const QMap<int, QString> &wallpapers)
    {
        #ifdef Q_OS_WIN
        try {
            if (wallpapers.isEmpty()) {
                return;
            }

            // Vérifier que tous les chemins existent avant de continuer
            bool allPathsValid = true;
            for (auto it = wallpapers.constBegin(); it != wallpapers.constEnd(); ++it) {
                if (!QFile::exists(it.value())) {
                    allPathsValid = false;
                    break;
                }
            }

            if (!allPathsValid) {
                return;
            }

            // Utiliser WallpaperBuilder pour créer une nouvelle image composite
            WallpaperBuilder builder;
            QString outputPath = builder.getTemporaryWallpaperPath();

            if (builder.createMultiScreenWallpaper(wallpapers, outputPath)) {
                setWallpaperWithSmoothTransition(outputPath);
            }
        } catch (const std::exception &e) {
            // Ignorer les erreurs
        } catch (...) {
            // Ignorer les erreurs
        }
        #endif
    }

    void getMultipleRandomWallpapers(const QList<int> &targetScreens)
    {
        // Nettoyer un téléchargement précédent si nécessaire
        if (currentMultiDownload) {
            delete currentMultiDownload;
        }

        // Initialiser la structure de téléchargement multiple
        currentMultiDownload = new MultiWallpaperDownload();
        currentMultiDownload->targetScreens = targetScreens;
        currentMultiDownload->pendingDownloads = targetScreens.size();
        currentMultiDownload->completedDownloads = 0;

        // Lancer le téléchargement pour chaque écran
        for (int screenIndex : targetScreens) {
            getRandomWallpaperForScreen(screenIndex);
        }
    }

    void getRandomWallpaperForScreen(int screenIndex)
    {
        // Utiliser le nouveau système de pondération
        tryGetWallpaperWithWeightedCategory(screenIndex, 0); // Commencer avec date du jour (0 jours back)
    }

    void tryGetWallpaperWithWeightedCategory(int screenIndex, int daysBack)
    {
        // Vérifier que le téléchargement multi-écrans est toujours actif
        if (!currentMultiDownload) {
            return;
        }

        // Sélectionner une catégorie pondérée
        QString selectedCategoryId = selectWeightedRandomCategory();

        if (selectedCategoryId.isEmpty()) {
            // Plus aucune catégorie disponible, utiliser un wallpaper de l'historique
            getRandomWallpaperFromHistory(screenIndex);
            return;
        }

        // Calculer la date (actuelle - daysBack)
        QString targetDate = (daysBack == 0) ? getCurrentDateString() : getPreviousDateString(daysBack);

        // Appeler l'API avec la catégorie et la date
        QString url = QString("https://kazflow.com/api/wallpapers?category=%1&date=%2")
                      .arg(selectedCategoryId)
                      .arg(QString(targetDate).replace("/", "%2F")); // URL encode les "/"

        QNetworkRequest request{QUrl(url)};
        QNetworkReply *reply = networkManager->get(request);
        setupSslErrorHandling(reply);

        connect(reply, &QNetworkReply::finished, [this, reply, screenIndex, selectedCategoryId, daysBack, targetDate]() {
            // Vérifier que le téléchargement multi-écrans est toujours actif
            if (!currentMultiDownload) {
                reply->deleteLater();
                return;
            }

            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonObject obj = doc.object();

                if (obj["success"].toBool()) {
                    QJsonArray wallpapers = obj["data"].toArray();
                    if (!wallpapers.isEmpty()) {
                        // Filtrer les wallpapers pour éviter les doublons de l'historique
                        QStringList availableWallpapers;
                        for (const QJsonValue &wallpaperValue : wallpapers) {
                            QJsonObject wallpaper = wallpaperValue.toObject();
                            QString filename = wallpaper["filename"].toString();

                            // Vérifier si ce wallpaper n'est pas dans l'historique de cet écran
                            if (!screenWallpaperHistory[screenIndex].contains(filename)) {
                                availableWallpapers.append(filename);
                            }
                        }

                        if (!availableWallpapers.isEmpty()) {
                            // Choisir un wallpaper aléatoire parmi ceux disponibles
                            int randomIndex = QRandomGenerator::global()->bounded(availableWallpapers.size());
                            QString selectedFilename = availableWallpapers.at(randomIndex);

                            // Télécharger ce wallpaper
                            downloadWallpaperForScreen(selectedFilename, screenIndex);
                            return;
                        }
                    }

                    // Aucun wallpaper nouveau pour cette date
                    // Vérifier si l'API suggère une date antérieure
                    QString nearestPreviousDate = obj["nearest_previous_date"].toString();

                    if (!nearestPreviousDate.isEmpty() && nearestPreviousDate != targetDate) {
                        // Relancer la recherche avec la date suggérée par l'API
                        tryGetWallpaperWithSpecificDate(screenIndex, selectedCategoryId, nearestPreviousDate);
                    } else {
                        // Plus aucune date disponible pour cette catégorie
                        // Exclure cette catégorie et essayer une autre
                        excludedCategories.insert(selectedCategoryId);

                        QString nextCategory = selectWeightedRandomCategory();
                        if (!nextCategory.isEmpty()) {
                            // Recommencer avec une nouvelle catégorie
                            tryGetWallpaperWithWeightedCategory(screenIndex, 0);
                        } else {
                            // Plus aucune catégorie disponible, utiliser l'historique
                            getRandomWallpaperFromHistory(screenIndex);
                        }
                    }
                } else {
                    handleMultiDownloadError(QString("Erreur API wallpapers pour écran %1: %2").arg(screenIndex + 1).arg(obj["error"].toString()));
                }
            } else {
                handleMultiDownloadError(QString("Erreur de connexion wallpapers pour écran %1: %2").arg(screenIndex + 1).arg(reply->errorString()));
            }
            reply->deleteLater();
        });
    }

    void tryGetWallpaperWithSpecificDate(int screenIndex, const QString &categoryId, const QString &date)
    {
        // Vérifier que le téléchargement multi-écrans est toujours actif
        if (!currentMultiDownload) {
            return;
        }

        // Appeler l'API avec la catégorie et la date spécifique
        QString url = QString("https://kazflow.com/api/wallpapers?category=%1&date=%2")
                      .arg(categoryId)
                      .arg(QString(date).replace("/", "%2F"));

        QNetworkRequest request{QUrl(url)};
        QNetworkReply *reply = networkManager->get(request);
        setupSslErrorHandling(reply);

        connect(reply, &QNetworkReply::finished, [this, reply, screenIndex, categoryId, date]() {
            // Vérifier que le téléchargement multi-écrans est toujours actif
            if (!currentMultiDownload) {
                reply->deleteLater();
                return;
            }

            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonObject obj = doc.object();

                if (obj["success"].toBool()) {
                    QJsonArray wallpapers = obj["data"].toArray();

                    if (!wallpapers.isEmpty()) {
                        // Filtrer les wallpapers pour éviter les doublons de l'historique
                        QStringList availableWallpapers;
                        for (const QJsonValue &wallpaperValue : wallpapers) {
                            QJsonObject wallpaper = wallpaperValue.toObject();
                            QString filename = wallpaper["filename"].toString();

                            // Vérifier si ce wallpaper n'est pas dans l'historique de cet écran
                            if (!screenWallpaperHistory[screenIndex].contains(filename)) {
                                availableWallpapers.append(filename);
                            }
                        }

                        if (!availableWallpapers.isEmpty()) {
                            // Choisir un wallpaper aléatoire parmi ceux disponibles
                            int randomIndex = QRandomGenerator::global()->bounded(availableWallpapers.size());
                            QString selectedFilename = availableWallpapers.at(randomIndex);

                            // Télécharger ce wallpaper
                            downloadWallpaperForScreen(selectedFilename, screenIndex);
                            return;
                        }
                    }

                    // Toujours aucun wallpaper nouveau, vérifier la date antérieure suivante
                    QString nearestPreviousDate = obj["nearest_previous_date"].toString();

                    if (!nearestPreviousDate.isEmpty() && nearestPreviousDate != date) {
                        // Relancer avec la date encore plus ancienne
                        tryGetWallpaperWithSpecificDate(screenIndex, categoryId, nearestPreviousDate);
                    } else {
                        // Catégorie épuisée, passer à une autre
                        excludedCategories.insert(categoryId);

                        QString nextCategory = selectWeightedRandomCategory();
                        if (!nextCategory.isEmpty()) {
                            tryGetWallpaperWithWeightedCategory(screenIndex, 0);
                        } else {
                            // Plus aucune catégorie disponible, utiliser l'historique
                            getRandomWallpaperFromHistory(screenIndex);
                        }
                    }
                } else {
                    handleMultiDownloadError(QString("Erreur API wallpapers pour écran %1: %2").arg(screenIndex + 1).arg(obj["error"].toString()));
                }
            } else {
                handleMultiDownloadError(QString("Erreur de connexion wallpapers pour écran %1: %2").arg(screenIndex + 1).arg(reply->errorString()));
            }
            reply->deleteLater();
        });
    }

    void getRandomWallpaperFromHistory(int screenIndex)
    {
        // Vérifier que le téléchargement multi-écrans est toujours actif
        if (!currentMultiDownload) {
            return;
        }

        // Lister tous les wallpapers de l'historique
        QStringList historyFiles = screenWallpaperHistory[screenIndex];

        if (historyFiles.isEmpty()) {
            handleMultiDownloadError(QString("Aucun wallpaper disponible dans l'historique pour l'écran %1").arg(screenIndex + 1));
            return;
        }

        // Choisir un wallpaper aléatoire de l'historique
        int randomIndex = QRandomGenerator::global()->bounded(historyFiles.size());
        QString selectedFilename = historyFiles.at(randomIndex);

        // Vérifier que le fichier existe encore dans le cache local
        QString historyDir = getHistoryDirectory();
        QString localImagePath = historyDir + "/" + selectedFilename;

        if (QFile::exists(localImagePath)) {
            // Utiliser directement le fichier local - simuler un téléchargement réussi
            if (currentMultiDownload) {
                // Créer une version BMP pour Windows si nécessaire
                QString bmpFilePath = localImagePath;
                QPixmap pixmap(localImagePath);
                if (!pixmap.isNull()) {
                    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperAI";
                    QDir().mkpath(tempDir);
                    QString bmpPath = tempDir + "/" + QFileInfo(selectedFilename).baseName() + "_screen" + QString::number(screenIndex) + ".bmp";
                    pixmap.save(bmpPath, "BMP");
                    bmpFilePath = bmpPath;
                }

                currentMultiDownload->downloadedImages[screenIndex] = bmpFilePath;
                currentMultiDownload->originalImages[screenIndex] = localImagePath;
                currentMultiDownload->completedDownloads++;

                // Mettre à jour le statut
                statusLabel->setText(QString("Images téléchargées: %1/%2")
                                   .arg(currentMultiDownload->completedDownloads)
                                   .arg(currentMultiDownload->pendingDownloads));

                // Vérifier si tous les téléchargements sont terminés
                if (currentMultiDownload->completedDownloads >= currentMultiDownload->pendingDownloads) {
                    applyMultipleWallpapers();
                }
            }
        } else {
            // Re-télécharger depuis l'API
            downloadWallpaperForScreen(selectedFilename, screenIndex);
        }
    }

    void downloadWallpaperForScreen(const QString &filename, int screenIndex)
    {
        // Vérifier que le téléchargement multi-écrans est toujours actif
        if (!currentMultiDownload) {
            return;
        }

        // Construire l'URL complète pour le téléchargement
        QString imageUrl = QString("https://kazflow.com/api/get/%1").arg(filename);

        // Télécharger cette image pour cet écran
        downloadImageForScreen(imageUrl, screenIndex);
    }

    void downloadImageForScreen(const QString &imageUrl, int screenIndex)
    {
        // Vérification supplémentaire de l'URL
        if (imageUrl.isEmpty()) {
            handleMultiDownloadError(QString("URL vide passée à downloadImageForScreen pour écran %1").arg(screenIndex + 1));
            return;
        }

        QUrl url(imageUrl);
        if (!url.isValid() || url.scheme().isEmpty()) {
            handleMultiDownloadError(QString("URL invalide pour écran %1: %2").arg(screenIndex + 1).arg(imageUrl));
            return;
        }

        QNetworkRequest request;
        request.setUrl(url);
        QNetworkReply *reply = networkManager->get(request);
        setupSslErrorHandling(reply);

        connect(reply, &QNetworkReply::finished, [this, reply, screenIndex, imageUrl]() {
            // Vérifier que le téléchargement multi-écrans est toujours actif
            if (!currentMultiDownload) {
                reply->deleteLater();
                return;
            }

            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();

                // Sauvegarder l'image temporairement
                QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperAI";
                QDir().mkpath(tempDir);

                QString filename = QUrl(imageUrl).fileName();
                if (filename.isEmpty()) {
                    filename = QString("wallpaper_screen%1_%2.jpg").arg(screenIndex).arg(QDateTime::currentMSecsSinceEpoch());
                }

                // Sauvegarder le fichier original
                QString originalFilePath = tempDir + "/" + filename;
                QFile originalFile(originalFilePath);

                if (originalFile.open(QIODevice::WriteOnly)) {
                    originalFile.write(imageData);
                    originalFile.close();

                    // Créer une version BMP pour Windows si possible
                    QString bmpFilePath = originalFilePath;
                    QPixmap pixmap;
                    if (pixmap.loadFromData(imageData)) {
                        QString bmpPath = tempDir + "/" + QFileInfo(filename).baseName() + "_screen" + QString::number(screenIndex) + ".bmp";
                        pixmap.save(bmpPath, "BMP");
                        bmpFilePath = bmpPath;
                    }

                    // Marquer cette image comme téléchargée pour cet écran
                    if (currentMultiDownload) {
                        currentMultiDownload->downloadedImages[screenIndex] = bmpFilePath; // BMP pour Windows
                        currentMultiDownload->originalImages[screenIndex] = originalFilePath; // PNG original
                        currentMultiDownload->completedDownloads++;

                        // Mettre à jour le statut
                        statusLabel->setText(QString("Images téléchargées: %1/%2")
                                           .arg(currentMultiDownload->completedDownloads)
                                           .arg(currentMultiDownload->pendingDownloads));

                        // Vérifier si tous les téléchargements sont terminés
                        if (currentMultiDownload->completedDownloads >= currentMultiDownload->pendingDownloads) {
                            applyMultipleWallpapers();
                        }
                    } else {
                        // La structure a été détruite (probablement à cause d'une erreur précédente)
                        // Nettoyer les fichiers temporaires et ne rien faire d'autre
                        QFile::remove(originalFilePath);
                        QFile::remove(bmpFilePath);
                    }
                } else {
                    handleMultiDownloadError(QString("Erreur de sauvegarde pour écran %1").arg(screenIndex + 1));
                }
            } else {
                handleMultiDownloadError(QString("Erreur de téléchargement d'image pour écran %1: %2").arg(screenIndex + 1).arg(reply->errorString()));
            }
            reply->deleteLater();
        });
    }

    void applyMultipleWallpapers()
    {
        if (!currentMultiDownload) return;

        statusLabel->setText("Application des fonds d'écran...");

        // Construire une map complète avec nouvelles images + images historiques pour écrans non sélectionnés
        QMap<int, QString> completeImageMap = currentMultiDownload->downloadedImages;

        // Pour les écrans non sélectionnés, utiliser l'image la plus récente de l'historique
        if (screenSelector && multiScreenToggle->isChecked() && screenSelector->screenCount() > 1) {
            for (int i = 0; i < screenSelector->screenCount(); i++) {
                if (!currentMultiDownload->targetScreens.contains(i)) {
                    // Écran non sélectionné, récupérer la dernière image de l'historique
                    if (screenWallpaperHistory.contains(i) && !screenWallpaperHistory[i].isEmpty()) {
                        completeImageMap[i] = screenWallpaperHistory[i].first(); // Premier = le plus récent
                    }
                }
            }
        }

        // Créer une image composite avec les différentes images pour chaque écran
        if (setMultipleWallpapers(completeImageMap)) {
            // Mettre à jour l'historique pour chaque écran (utiliser les fichiers originaux)
            for (auto it = currentMultiDownload->originalImages.constBegin();
                 it != currentMultiDownload->originalImages.constEnd(); ++it) {
                int screenIndex = it.key();
                QString originalImagePath = it.value();

                addToScreenHistory(screenIndex, originalImagePath, currentTriggerMode.isEmpty() ? "Manuel" : currentTriggerMode);
            }

            // Afficher le statut final
            QStringList screenNumbers;
            for (int screen : currentMultiDownload->targetScreens) {
                screenNumbers.append(QString::number(screen + 1));
            }
            statusLabel->setText(QString("Fonds d'écran appliqués sur les écrans %1").arg(screenNumbers.join(", ")));

            // Nettoyer les fichiers temporaires après application réussie
            cleanupTemporaryFiles();

            // Masquer l'alerte d'erreur si elle était affichée
            hideApiErrorAlert();

            restoreButton("Succès !");
        } else {
            restoreButton("Erreur lors de l'application");
        }

        // Nettoyer
        delete currentMultiDownload;
        currentMultiDownload = nullptr;
    }

    void handleMultiDownloadError(const QString &error)
    {
        if (currentMultiDownload) {
            delete currentMultiDownload;
            currentMultiDownload = nullptr;
        }

        // Réactiver le bouton
        changeNowButton->setEnabled(true);
        changeNowButton->setText("🖼️ Changer Maintenant");
        statusLabel->setText("Erreur lors du téléchargement");

        // Afficher l'alerte d'erreur API avec retry automatique
        showApiErrorAlert();
        startRetryTimer();

        // Aussi mettre un message court dans le statut
        restoreButton("Erreur de téléchargement");
    }

    void showApiErrorAlert()
    {
        errorAlertWidget->show();
        updateErrorAlertMessage();
    }

    void hideApiErrorAlert()
    {
        errorAlertWidget->hide();
        retryTimer->stop();
        retryCountdownSeconds = 0;
    }

    void startRetryTimer()
    {
        // 15 minutes = 900 secondes
        retryCountdownSeconds = 900;
        updateErrorAlertMessage();
        retryTimer->start(1000); // Tick chaque seconde
    }

    void onRetryTimerTick()
    {
        retryCountdownSeconds--;

        if (retryCountdownSeconds <= 0) {
            // Temps écoulé, tenter de nouveau changement
            retryTimer->stop();
            hideApiErrorAlert();

            // Relancer le changement de wallpaper
            currentTriggerMode = "Retry automatique";
            onChangeNowClicked();
        } else {
            // Mettre à jour l'affichage du compteur
            updateErrorAlertMessage();
        }
    }

    void updateErrorAlertMessage()
    {
        int minutes = retryCountdownSeconds / 60;
        int seconds = retryCountdownSeconds % 60;

        QString message = QString(
            "Erreur lors du téléchargement du fond d'écran\n"
            "Prochaine tentative dans : %1:%2"
        ).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));

        errorAlertLabel->setText(message);
    }

    // === FONCTIONS CARROUSEL D'HISTORIQUE ===

    void populateHistoryScreenCombo()
    {
        // Cette fonction n'est plus nécessaire car on n'a plus de combobox
        // On charge directement l'historique fusionné
        loadMergedHistory();
    }

    void onHistoryScreenChanged(int index)
    {
        // Cette fonction n'est plus utilisée
    }

    void loadHistoryForScreen(int screenIndex)
    {
        // Cette fonction n'est plus utilisée
        // On utilise maintenant loadMergedHistory()
    }

    void loadMergedHistory()
    {
        selectedHistoryImagePath.clear();
        selectedHistoryScreenIndex = -1;
        historyScrollOffset = 0;

        // Charger et fusionner tous les historiques
        currentHistoryEntries = getAllHistoryEntriesMerged();

        // Rafraîchir l'affichage
        refreshHistoryCarousel();
    }

    QList<HistoryEntry> getAllHistoryEntriesMerged()
    {
        QList<HistoryEntry> allEntries;

        int screenCount = screenSelector ? screenSelector->screenCount() : 1;

        // Parcourir tous les écrans
        for (int screenIndex = 0; screenIndex < screenCount; screenIndex++) {
            QString screenId = getScreenUniqueId(screenIndex);
            QString historyDir = getHistoryDirectory();
            QString logFilePath = historyDir + "/" + screenId + ".log";

            QFile logFile(logFilePath);
            if (!logFile.open(QIODevice::ReadOnly)) {
                continue;
            }

            QTextStream stream(&logFile);
            QString line;
            QRegularExpression regex(R"(\[([^\]]+)\] \[([^\]]+)\] \[([^\]]+)\] \[([^\]]+)\])");

            // Lire toutes les lignes
            while (stream.readLineInto(&line)) {
                if (!line.isEmpty()) {
                    QRegularExpressionMatch match = regex.match(line);
                    if (match.hasMatch()) {
                        HistoryEntry entry;
                        entry.timestamp = QDateTime::fromString(match.captured(1), "yyyy-MM-dd hh:mm:ss");
                        entry.filename = match.captured(2);
                        entry.triggerMode = match.captured(3);
                        entry.adjustMode = match.captured(4);
                        entry.screenIndex = screenIndex;

                        allEntries.append(entry);
                    }
                }
            }
            logFile.close();
        }

        // Trier par date (plus récent en premier)
        std::sort(allEntries.begin(), allEntries.end());

        // Filtrer les doublons consécutifs (même filename)
        QList<HistoryEntry> filteredEntries;
        for (int i = 0; i < allEntries.size(); i++) {
            bool isDuplicate = false;

            if (i > 0) {
                const HistoryEntry &current = allEntries[i];
                const HistoryEntry &previous = allEntries[i - 1];

                // Vérifier si c'est le même fichier (peu importe l'écran dans le carrousel fusionné)
                if (current.filename == previous.filename) {
                    isDuplicate = true;
                }
            }

            if (!isDuplicate) {
                filteredEntries.append(allEntries[i]);
            }
        }

        return filteredEntries;
    }

    void refreshHistoryCarousel()
    {
        // Nettoyer le carrousel
        clearHistoryCarousel();

        if (currentHistoryEntries.isEmpty()) {
            // Pas d'historique
            QLabel *emptyLabel = new QLabel("Aucun historique disponible");
            emptyLabel->setStyleSheet("color: #888888; font-size: 10pt;");
            emptyLabel->setAlignment(Qt::AlignCenter);
            historyCarouselLayout->addWidget(emptyLabel);
            updateHistoryNavigationButtons();
            return;
        }

        // Calculer combien d'images on peut afficher (largeur disponible / largeur miniature)
        // Largeur carrousel ~= 625px (725 - 40 - 40 - marges), miniature = 140px + 10px spacing
        int visibleCount = 4; // On affiche 4 miniatures à la fois

        // Afficher les images visibles selon l'offset
        int startIdx = historyScrollOffset;
        int endIdx = qMin(startIdx + visibleCount, currentHistoryEntries.size());

        for (int i = startIdx; i < endIdx; i++) {
            const HistoryEntry &entry = currentHistoryEntries[i];
            addThumbnailToCarousel(entry.filename, entry.screenIndex);
        }

        // Mettre à jour l'état des boutons de navigation
        updateHistoryNavigationButtons();
    }

    void updateHistoryNavigationButtons()
    {
        if (currentHistoryEntries.isEmpty()) {
            historyPrevButton->setEnabled(false);
            historyNextButton->setEnabled(false);
            return;
        }

        int visibleCount = 4;

        // Bouton précédent : actif si on n'est pas au début
        historyPrevButton->setEnabled(historyScrollOffset > 0);

        // Bouton suivant : actif si on peut encore défiler à droite
        historyNextButton->setEnabled(historyScrollOffset + visibleCount < currentHistoryEntries.size());
    }

    void onHistoryPrevClicked()
    {
        if (historyScrollOffset > 0) {
            historyScrollOffset--;
            refreshHistoryCarousel();
        }
    }

    void onHistoryNextClicked()
    {
        int visibleCount = 4;
        if (historyScrollOffset + visibleCount < currentHistoryEntries.size()) {
            historyScrollOffset++;
            refreshHistoryCarousel();
        }
    }

    QStringList getFullHistoryFromLogs(int screenIndex)
    {
        QStringList fileNames;
        QString screenId = getScreenUniqueId(screenIndex);
        QString historyDir = getHistoryDirectory();
        QString logFilePath = historyDir + "/" + screenId + ".log";

        QFile logFile(logFilePath);
        if (!logFile.open(QIODevice::ReadOnly)) {
            return fileNames;
        }

        QTextStream stream(&logFile);
        QStringList lines;
        QString line;

        // Lire toutes les lignes
        while (stream.readLineInto(&line)) {
            if (!line.isEmpty()) {
                lines.append(line);
            }
        }
        logFile.close();

        // Parser les lignes dans l'ordre inverse (du plus récent au plus ancien)
        QRegularExpression regex(R"(\[([^\]]+)\] \[([^\]]+)\] \[([^\]]+)\] \[([^\]]+)\])");

        for (int i = lines.size() - 1; i >= 0; i--) {
            QRegularExpressionMatch match = regex.match(lines[i]);
            if (match.hasMatch()) {
                QString filename = match.captured(2);

                // Retourner les noms de fichiers (pas les chemins complets)
                // On utilisera l'API pour charger les miniatures
                if (!fileNames.contains(filename)) {
                    fileNames.append(filename);
                }
            }
        }

        return fileNames;
    }

    void clearHistoryCarousel()
    {
        // Supprimer tous les widgets du carrousel
        QLayoutItem *item;
        while ((item = historyCarouselLayout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                delete item->widget();
            }
            delete item;
        }
    }

    void addThumbnailToCarousel(const QString &filename, int screenIndex)
    {
        // Créer un bouton cliquable pour la miniature
        QPushButton *thumbnailButton = new QPushButton();
        thumbnailButton->setFixedSize(140, 80);
        thumbnailButton->setCursor(Qt::PointingHandCursor);
        thumbnailButton->setCheckable(true);

        thumbnailButton->setStyleSheet(
            "QPushButton {"
            "background-color: #3b3b3b;"
            "border: 2px solid #555555;"
            "border-radius: 4px;"
            "padding: 0px;"
            "}"
            "QPushButton:hover {"
            "border-color: #2196F3;"
            "}"
            "QPushButton:checked {"
            "border-color: #2196F3;"
            "border-width: 3px;"
            "background-color: #2b2b2b;"
            "}"
        );

        // Stocker le filename et screenIndex dans des propriétés
        thumbnailButton->setProperty("filename", filename);
        thumbnailButton->setProperty("screenIndex", screenIndex);

        connect(thumbnailButton, &QPushButton::clicked, [this, thumbnailButton]() {
            onHistoryThumbnailClicked(thumbnailButton);
        });

        historyCarouselLayout->addWidget(thumbnailButton);

        // Charger la miniature via l'API de manière asynchrone
        loadHistoryThumbnail(filename, thumbnailButton);
    }

    void loadHistoryThumbnail(const QString &filename, QPushButton *thumbnailButton)
    {
        // Vérifier d'abord si la miniature est en cache (répertoire partagé avec les catégories)
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/thumbnails";
        QDir().mkpath(cacheDir);
        QString cachedPath = cacheDir + "/" + filename;

        if (QFile::exists(cachedPath)) {
            // Charger depuis le cache
            QPixmap pixmap(cachedPath);
            if (!pixmap.isNull()) {
                QPixmap scaledPixmap = pixmap.scaled(140, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                thumbnailButton->setIcon(QIcon(scaledPixmap));
                thumbnailButton->setIconSize(QSize(140, 80));
            }
            return;
        }

        // Sinon télécharger via l'API
        QString miniUrl = QString("https://kazflow.com/api/mini/%1").arg(filename);
        QNetworkRequest request{QUrl(miniUrl)};
        QNetworkReply *reply = networkManager->get(request);
        setupSslErrorHandling(reply);

        // Utiliser QPointer pour éviter les crashes si le bouton est détruit pendant la requête
        QPointer<QPushButton> safeButton = thumbnailButton;

        connect(reply, &QNetworkReply::finished, [this, reply, safeButton, filename, cachedPath]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    // Sauvegarder dans le cache
                    pixmap.save(cachedPath);

                    // Afficher uniquement si le bouton existe toujours
                    if (safeButton) {
                        QPixmap scaledPixmap = pixmap.scaled(140, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        safeButton->setIcon(QIcon(scaledPixmap));
                        safeButton->setIconSize(QSize(140, 80));
                    }

                    // Nettoyer le cache si trop de fichiers (limite 100)
                    cleanupThumbnailCache();
                }
            }
            reply->deleteLater();
        });
    }

    void cleanupThumbnailCache()
    {
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/thumbnails";
        QDir dir(cacheDir);
        if (!dir.exists()) return;

        QFileInfoList files = dir.entryInfoList(QDir::Files, QDir::Time);
        int maxCacheFiles = 100;

        // Supprimer les fichiers les plus anciens si on dépasse la limite
        if (files.size() > maxCacheFiles) {
            for (int i = maxCacheFiles; i < files.size(); i++) {
                QFile::remove(files[i].absoluteFilePath());
            }
        }
    }

    void onHistoryThumbnailClicked(QPushButton *clickedButton)
    {
        // Désélectionner tous les autres boutons
        for (int i = 0; i < historyCarouselLayout->count(); ++i) {
            QWidget *widget = historyCarouselLayout->itemAt(i)->widget();
            if (QPushButton *btn = qobject_cast<QPushButton*>(widget)) {
                if (btn != clickedButton) {
                    btn->setChecked(false);
                }
            }
        }

        // Marquer le bouton comme sélectionné
        clickedButton->setChecked(true);

        // Stocker le filename et l'écran d'origine, puis reconstruire le chemin complet
        QString filename = clickedButton->property("filename").toString();
        int screenIndex = clickedButton->property("screenIndex").toInt();
        selectedHistoryScreenIndex = screenIndex;

        QString screenId = getScreenUniqueId(screenIndex);
        QString wallpapersDir = getWallpapersDirectory();
        selectedHistoryImagePath = wallpapersDir + "/" + screenId + "/" + filename;
    }

    void onApplyHistoryClicked()
    {
        if (selectedHistoryImagePath.isEmpty()) {
            QMessageBox::information(this, "Sélection requise", "Veuillez sélectionner une image de l'historique.");
            return;
        }

        // Vérifier si le fichier existe encore
        if (!QFile::exists(selectedHistoryImagePath)) {
            // Le fichier n'existe plus, il faut le re-télécharger depuis l'API
            QString filename = QFileInfo(selectedHistoryImagePath).fileName();
            redownloadHistoryImage(filename);
            return;
        }

        // Déterminer quels écrans cibler (logique identique à onChangeNowClicked)
        QList<int> selectedScreens;
        if (multiScreenToggle->isChecked() && screenSelector && screenSelector->screenCount() > 1) {
            // Mode multi-écran activé : utiliser la sélection des écrans
            selectedScreens = screenSelector->getSelectedScreens();
        } else {
            // Mode multi-écran désactivé : appliquer sur tous les écrans
            for (int i = 0; i < (screenSelector ? screenSelector->screenCount() : 1); i++) {
                selectedScreens.append(i);
            }
        }

        // Si plusieurs écrans et que l'avertissement n'est pas désactivé
        if (selectedScreens.size() > 1 && !dontShowMultiScreenWarning) {
            showMultiScreenWarningDialog(selectedScreens);
        } else {
            applyHistoryImageToScreens(selectedScreens);
        }
    }

    void showMultiScreenWarningDialog(const QList<int> &selectedScreens)
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Application multi-écrans");
        msgBox.setText(QString("Le fond d'écran sera appliqué sur %1 écrans sélectionnés.").arg(selectedScreens.size()));
        msgBox.setInformativeText("Voulez-vous continuer ?");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        // Ajouter une checkbox "Ne plus afficher"
        QCheckBox *dontShowAgain = new QCheckBox("Ne plus afficher cet avertissement");
        msgBox.setCheckBox(dontShowAgain);

        msgBox.setStyleSheet(
            "QMessageBox {"
            "background-color: #2b2b2b;"
            "color: #ffffff;"
            "}"
            "QMessageBox QPushButton {"
            "background-color: #2196F3;"
            "color: white;"
            "border: none;"
            "border-radius: 4px;"
            "padding: 8px 16px;"
            "min-width: 80px;"
            "}"
            "QMessageBox QPushButton:hover {"
            "background-color: #42A5F5;"
            "}"
            "QCheckBox {"
            "color: white;"
            "}"
        );

        int result = msgBox.exec();

        // Sauvegarder la préférence si la checkbox est cochée
        if (dontShowAgain->isChecked()) {
            dontShowMultiScreenWarning = true;
            QSettings settings("WallpaperAI", "WallpaperSettings");
            settings.setValue("dontShowMultiScreenWarning", true);
        }

        if (result == QMessageBox::Yes) {
            applyHistoryImageToScreens(selectedScreens);
        }
    }

    void redownloadHistoryImage(const QString &filename)
    {
        statusLabel->setText("Téléchargement de l'image depuis l'API...");

        QString imageUrl = QString("https://kazflow.com/api/get/%1").arg(filename);
        QNetworkRequest request{QUrl(imageUrl)};
        QNetworkReply *reply = networkManager->get(request);
        setupSslErrorHandling(reply);

        connect(reply, &QNetworkReply::finished, [this, reply, filename]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();

                // Sauvegarder l'image téléchargée
                QString screenId = getScreenUniqueId(currentHistoryScreen);
                QString wallpapersDir = getWallpapersDirectory();
                QString screenDir = wallpapersDir + "/" + screenId;
                QDir().mkpath(screenDir);

                QString savedPath = screenDir + "/" + filename;
                QFile file(savedPath);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(imageData);
                    file.close();

                    selectedHistoryImagePath = savedPath;
                    statusLabel->setText("Image téléchargée, prête à être appliquée");

                    // Réessayer l'application
                    onApplyHistoryClicked();
                } else {
                    statusLabel->setText("Erreur lors de la sauvegarde de l'image");
                }
            } else {
                statusLabel->setText("Erreur lors du téléchargement de l'image");
            }
            reply->deleteLater();
        });
    }

    void applyHistoryImageToScreens(const QList<int> &selectedScreens)
    {
        // Appliquer l'image sélectionnée sur les écrans choisis
        statusLabel->setText("Application du fond d'écran...");

        // Construire une map complète avec l'image sélectionnée + images historiques pour écrans non sélectionnés
        QMap<int, QString> completeImageMap;

        // Ajouter l'image sélectionnée pour les écrans ciblés
        for (int screenIndex : selectedScreens) {
            completeImageMap[screenIndex] = selectedHistoryImagePath;
        }

        // Pour les écrans non sélectionnés, utiliser l'image la plus récente de l'historique
        // (même logique que applyMultipleWallpapers)
        if (screenSelector && multiScreenToggle->isChecked() && screenSelector->screenCount() > 1) {
            for (int i = 0; i < screenSelector->screenCount(); i++) {
                if (!selectedScreens.contains(i)) {
                    // Écran non sélectionné, récupérer la dernière image de l'historique
                    QString lastWallpaper = getLastWallpaperFromHistory(i);
                    if (!lastWallpaper.isEmpty()) {
                        completeImageMap[i] = lastWallpaper;
                    }
                }
            }
        }

        if (setMultipleWallpapers(completeImageMap)) {
            // Ajouter à l'historique pour chaque écran ciblé (comme un changement normal)
            for (int screenIndex : selectedScreens) {
                addToScreenHistory(screenIndex, selectedHistoryImagePath, "Historique");
            }

            statusLabel->setText("Fond d'écran appliqué avec succès !");
        } else {
            statusLabel->setText("Erreur lors de l'application");
        }
    }

    bool setWindowsWallpaper(const QString &imagePath, int screenIndex = -1)
    {
        #ifdef Q_OS_WIN
        QFileInfo fileInfo(imagePath);
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            return false;
        }

        // Si screenIndex est -1 ou qu'il n'y a qu'un écran, utiliser la méthode traditionnelle
        if (screenIndex == -1 || QApplication::screens().count() <= 1) {
            // Utiliser la nouvelle méthode avec transitions fluides préservées
            return setWallpaperWithSmoothTransition(imagePath);
        }

        // Pour un écran spécifique, utiliser WallpaperBuilder (méthode moderne)
        QMap<int, QString> imagePaths;

        // Récupérer les wallpapers actuels pour tous les autres écrans
        int screenCount = QApplication::screens().size();
        for (int i = 0; i < screenCount; i++) {
            if (i == screenIndex) {
                // Nouvelle image pour l'écran cible
                imagePaths[i] = imagePath;
            } else {
                // Garder le wallpaper actuel pour les autres écrans
                QString currentPath = getCurrentWallpaperFromLog(i);
                if (!currentPath.isEmpty() && QFile::exists(currentPath)) {
                    imagePaths[i] = currentPath;
                }
            }
        }

        // Utiliser WallpaperBuilder pour créer l'image composite
        WallpaperBuilder builder;
        QString outputPath = builder.getTemporaryWallpaperPath();

        if (builder.createMultiScreenWallpaper(imagePaths, outputPath)) {
            return setWallpaperWithSmoothTransition(outputPath);
        }

        return false;
        #else
        return false;
        #endif
    }

    // Fonction moderne qui utilise IDesktopWallpaper pour préserver les transitions fluides
    bool setWallpaperWithSmoothTransition(const QString &imagePath)
    {
        #ifdef Q_OS_WIN
        // Initialiser COM si nécessaire
        HRESULT hr = CoInitialize(nullptr);
        bool comInitialized = SUCCEEDED(hr);

        IDesktopWallpaper *pDesktopWallpaper = nullptr;

        // Créer l'instance IDesktopWallpaper (Windows 8+)
        hr = CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_INPROC_SERVER,
                              IID_IDesktopWallpaper, (void**)&pDesktopWallpaper);

        if (SUCCEEDED(hr)) {
            // Convertir le chemin en LPCWSTR
            std::wstring wImagePath = imagePath.toStdWString();

            // Définir le fond d'écran avec transitions préservées
            hr = pDesktopWallpaper->SetWallpaper(nullptr, wImagePath.c_str());

            pDesktopWallpaper->Release();

            if (comInitialized) {
                CoUninitialize();
            }

            return SUCCEEDED(hr);
        }

        if (comInitialized) {
            CoUninitialize();
        }

        // Fallback vers SystemParametersInfo si IDesktopWallpaper échoue
        std::wstring wImagePath = imagePath.toStdWString();
        BOOL result = SystemParametersInfoW(
            SPI_SETDESKWALLPAPER,
            0,
            (PVOID)wImagePath.c_str(),
            SPIF_SENDCHANGE  // Enlever SPIF_UPDATEINIFILE pour préserver les transitions
        );

        return result != 0;
        #else
        return false;
        #endif
    }

    bool setWallpaperForSpecificScreen(const QString &imagePath, int screenIndex)
    {
        #ifdef Q_OS_WIN
        // Méthode DualMonitorTools : Composer une image pour tous les écrans
        // mais changer seulement l'écran spécifié
        return setWallpaperUsingComposition(imagePath, screenIndex);
        #else
        return false;
        #endif
    }

    bool setWallpaperUsingComposition(const QString &imagePath, int targetScreenIndex)
    {
        #ifdef Q_OS_WIN
        // Obtenir les informations sur tous les écrans
        QList<QScreen*> screens = QApplication::screens();
        if (targetScreenIndex >= screens.count()) {
            return false;
        }

        // Calculer le rectangle virtuel total (comme les écrans sont disposés)
        QRect virtualDesktop = getVirtualDesktopRect(screens);

        // Créer une image composite qui couvre tous les écrans
        QPixmap compositeImage = createCompositeWallpaper(imagePath, targetScreenIndex, screens, virtualDesktop);

        if (compositeImage.isNull()) {
            return false;
        }

        // Sauvegarder l'image composite
        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperAI";
        QDir().mkpath(tempDir);
        QString compositePath = tempDir + "/wallpaper_composite.bmp";

        if (!compositeImage.save(compositePath, "BMP")) {
            return false;
        }

        // Configurer le registre pour le mode "tile"
        if (!setSpanWallpaperMode()) {
            return false;
        }

        // Appliquer le fond d'écran composite
        std::wstring wCompositePath = compositePath.toStdWString();
        BOOL result = SystemParametersInfoW(
            SPI_SETDESKWALLPAPER,
            0,
            (PVOID)wCompositePath.c_str(),
            SPIF_SENDCHANGE  // Enlever SPIF_UPDATEINIFILE pour préserver les transitions
        );

        return (result != FALSE);
        #else
        return false;
        #endif
    }

    QRect getVirtualDesktopRect(const QList<QScreen*> &screens)
    {
        QRect virtualRect;
        for (QScreen* screen : screens) {
            if (virtualRect.isNull()) {
                virtualRect = screen->geometry();
            } else {
                virtualRect = virtualRect.united(screen->geometry());
            }
        }
        return virtualRect;
    }

    QPixmap createCompositeWallpaper(const QString &newImagePath, int targetScreenIndex,
                                    const QList<QScreen*> &screens, const QRect &virtualDesktop)
    {
        // Capturer le fond d'écran actuel en premier
        QPixmap currentWallpaper = captureCurrentWallpaper(virtualDesktop);

        // Créer une image de la taille du bureau virtuel
        QPixmap composite(virtualDesktop.size());

        // Si on a capturé le fond actuel, l'utiliser comme base
        if (!currentWallpaper.isNull()) {
            composite = currentWallpaper;
        } else {
            composite.fill(Qt::black); // Fallback vers noir
        }

        QPainter painter(&composite);

        // Charger la nouvelle image
        QPixmap newImage(newImagePath);
        if (newImage.isNull()) {
            return QPixmap();
        }

        // Appliquer la nouvelle image seulement sur l'écran cible
        if (targetScreenIndex >= 0 && targetScreenIndex < screens.count()) {
            QScreen* targetScreen = screens[targetScreenIndex];
            QRect screenRect = targetScreen->geometry();

            // Calculer la position relative dans l'image composite
            QRect relativeRect = screenRect.translated(-virtualDesktop.topLeft());

            // Appliquer la nouvelle image sur l'écran cible
            QPixmap scaledImage = newImage.scaled(relativeRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

            // Centrer l'image si elle dépasse
            QRect sourceRect = QRect(0, 0, relativeRect.width(), relativeRect.height());
            if (scaledImage.width() > relativeRect.width()) {
                sourceRect.setX((scaledImage.width() - relativeRect.width()) / 2);
                sourceRect.setWidth(relativeRect.width());
            }
            if (scaledImage.height() > relativeRect.height()) {
                sourceRect.setY((scaledImage.height() - relativeRect.height()) / 2);
                sourceRect.setHeight(relativeRect.height());
            }

            painter.drawPixmap(relativeRect, scaledImage, sourceRect);
        }

        painter.end();
        return composite;
    }

    QPixmap captureCurrentWallpaper(const QRect &virtualDesktop)
    {
        #ifdef Q_OS_WIN
        // Capturer une screenshot de tout le bureau virtuel
        QPixmap screenshot = QApplication::primaryScreen()->grabWindow(0,
                                                                      virtualDesktop.x(),
                                                                      virtualDesktop.y(),
                                                                      virtualDesktop.width(),
                                                                      virtualDesktop.height());
        return screenshot;
        #else
        return QPixmap();
        #endif
    }

    bool setFillWallpaperMode()
    {
        #ifdef Q_OS_WIN
        // Modifier le registre pour forcer le mode "Fill" (meilleur pour images composites multi-écrans)
        HKEY hkey;
        LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_SET_VALUE, &hkey);

        if (result != ERROR_SUCCESS) {
            return false;
        }

        // Configurer TileWallpaper = "0" et WallpaperStyle = "10" (Fill)
        const wchar_t* tileValue = L"0";
        const wchar_t* styleValue = L"10";

        bool success = true;

        result = RegSetValueExW(hkey, L"TileWallpaper", 0, REG_SZ,
                               (const BYTE*)tileValue, (wcslen(tileValue) + 1) * sizeof(wchar_t));
        if (result != ERROR_SUCCESS) {
            success = false;
        }

        result = RegSetValueExW(hkey, L"WallpaperStyle", 0, REG_SZ,
                               (const BYTE*)styleValue, (wcslen(styleValue) + 1) * sizeof(wchar_t));
        if (result != ERROR_SUCCESS) {
            success = false;
        }

        RegCloseKey(hkey);
        return success;
        #else
        return false;
        #endif
    }


    bool setSpanWallpaperMode()
    {
        #ifdef Q_OS_WIN
        // Modifier le registre pour forcer le mode "Span" (étendre sur plusieurs écrans)
        HKEY hkey;
        LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_SET_VALUE, &hkey);

        if (result != ERROR_SUCCESS) {
            return false;
        }

        // Configurer TileWallpaper = "0" et WallpaperStyle = "22" pour le mode Span
        const wchar_t* tileValue = L"0";
        const wchar_t* styleValue = L"22";

        bool success = true;

        result = RegSetValueExW(hkey, L"TileWallpaper", 0, REG_SZ,
                               (const BYTE*)tileValue, (wcslen(tileValue) + 1) * sizeof(wchar_t));
        if (result != ERROR_SUCCESS) {
            success = false;
        }

        result = RegSetValueExW(hkey, L"WallpaperStyle", 0, REG_SZ,
                               (const BYTE*)styleValue, (wcslen(styleValue) + 1) * sizeof(wchar_t));
        if (result != ERROR_SUCCESS) {
            success = false;
        }

        RegCloseKey(hkey);
        return success;
        #else
        return false;
        #endif
    }

    bool trySetWallpaperWithIDesktopWallpaper(const QString &imagePath, int screenIndex)
    {
        #ifdef Q_OS_WIN
        // Initialiser COM
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
            return false;
        }

        bool success = false;
        IDesktopWallpaper* pDesktopWallpaper = nullptr;

        // Créer l'interface IDesktopWallpaper
        hr = CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_INPROC_SERVER,
                             IID_IDesktopWallpaper, (void**)&pDesktopWallpaper);

        if (SUCCEEDED(hr) && pDesktopWallpaper) {
            // D'abord, s'assurer que le mode n'est pas "span" (qui synchronise tous les écrans)
            DESKTOP_WALLPAPER_POSITION currentPosition;
            hr = pDesktopWallpaper->GetPosition(&currentPosition);
            if (SUCCEEDED(hr) && currentPosition == DWPOS_SPAN) {
                // Changer vers FILL pour permettre des fonds d'écran différents par moniteur
                pDesktopWallpaper->SetPosition(DWPOS_FILL);
            }

            // Obtenir la liste des moniteurs
            UINT monitorCount = 0;
            hr = pDesktopWallpaper->GetMonitorDevicePathCount(&monitorCount);

            if (SUCCEEDED(hr) && screenIndex >= 0 && screenIndex < (int)monitorCount) {
                // Obtenir le chemin du moniteur spécifique
                LPWSTR monitorId = nullptr;
                hr = pDesktopWallpaper->GetMonitorDevicePathAt(screenIndex, &monitorId);

                if (SUCCEEDED(hr) && monitorId) {
                    // Convertir le chemin de l'image
                    std::wstring wImagePath = imagePath.toStdWString();

                    // Définir le fond d'écran pour ce moniteur spécifique
                    hr = pDesktopWallpaper->SetWallpaper(monitorId, wImagePath.c_str());

                    if (SUCCEEDED(hr)) {
                        success = true;
                    }

                    // Libérer la mémoire du monitorId
                    CoTaskMemFree(monitorId);
                }
            }

            // Libérer l'interface
            pDesktopWallpaper->Release();
        }

        // Nettoyer COM seulement si on l'a initialisé
        if (hr != RPC_E_CHANGED_MODE) {
            CoUninitialize();
        }

        return success;
        #else
        return false;
        #endif
    }

    bool trySetWallpaperWithSystemParameters(const QString &imagePath, int screenIndex)
    {
        #ifdef Q_OS_WIN
        // Fallback: Si IDesktopWallpaper échoue, utiliser la méthode traditionnelle
        // Note: Cette méthode changera tous les écrans, mais c'est mieux que rien

        QFileInfo fileInfo(imagePath);
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            return false;
        }

        // Convertir le chemin pour Windows
        std::wstring wImagePath = imagePath.toStdWString();

        // Utiliser la version Unicode
        BOOL result = SystemParametersInfoW(
            SPI_SETDESKWALLPAPER,
            0,
            (PVOID)wImagePath.c_str(),
            SPIF_SENDCHANGE  // Enlever SPIF_UPDATEINIFILE pour préserver les transitions
        );

        return (result != FALSE);
        #else
        return false;
        #endif
    }

    void restoreButton(const QString &message = "")
    {
        changeNowButton->setEnabled(true);
        changeNowButton->setText("🖼️ Changer Maintenant");
        if (!message.isEmpty()) {
            statusLabel->setText(message);
        }

        // Réinitialiser le mode de déclenchement pour le prochain changement
        currentTriggerMode = "Manuel";
    }

    bool setWindowsWallpaperMultiScreen(const QString &imagePath, const QList<int> &targetScreens, const QString &filename)
    {
        if (targetScreens.contains(-1)) {
            // Mode classique : tous les écrans
            return setWindowsWallpaper(imagePath, -1);
        } else {
            // Mode multi-écran : composer une image avec l'historique
            return setWallpaperUsingHistoryComposition(imagePath, targetScreens);
        }
    }

    bool setMultipleWallpapers(const QMap<int, QString> &imagePaths)
    {
        // Utiliser la nouvelle classe WallpaperBuilder
        WallpaperBuilder builder;
        QString compositePath = builder.getTemporaryWallpaperPath();

        if (builder.createMultiScreenWallpaper(imagePaths, compositePath)) {
            // Utiliser le mode Tile avec calculs précis comme DualMonitorTools
            setSpanWallpaperMode();

            #ifdef Q_OS_WIN
            std::wstring wImagePath = compositePath.toStdWString();
            BOOL result = SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (LPVOID)wImagePath.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
            return result != 0;
            #else
            return false;
            #endif
        }

        return false;
    }





    bool setWallpaperUsingHistoryComposition(const QString &newImagePath, const QList<int> &targetScreens)
    {
        #ifdef Q_OS_WIN
        // Obtenir les informations sur tous les écrans
        QList<QScreen*> screens = QApplication::screens();

        // Calculer le rectangle virtuel total
        QRect virtualDesktop = getVirtualDesktopRect(screens);

        // Créer une image composite qui utilise l'historique
        QPixmap compositeImage = createCompositeWallpaperWithHistory(newImagePath, targetScreens, screens, virtualDesktop);

        if (compositeImage.isNull()) {
            return false;
        }

        // Sauvegarder l'image composite
        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperAI";
        QDir().mkpath(tempDir);
        QString compositePath = tempDir + "/wallpaper_composite.bmp";

        if (!compositeImage.save(compositePath, "BMP")) {
            return false;
        }

        // Configurer le registre pour le mode "tile"
        if (!setSpanWallpaperMode()) {
            return false;
        }

        // Appliquer le fond d'écran composite
        std::wstring wCompositePath = compositePath.toStdWString();
        BOOL result = SystemParametersInfoW(
            SPI_SETDESKWALLPAPER,
            0,
            (PVOID)wCompositePath.c_str(),
            SPIF_SENDCHANGE  // Enlever SPIF_UPDATEINIFILE pour préserver les transitions
        );

        return (result != FALSE);
        #else
        return false;
        #endif
    }

    QPixmap createCompositeWallpaperWithHistory(const QString &newImagePath, const QList<int> &targetScreens,
                                              const QList<QScreen*> &screens, const QRect &virtualDesktop)
    {
        // Créer une image de la taille du bureau virtuel
        QPixmap composite(virtualDesktop.size());
        composite.fill(Qt::black);

        QPainter painter(&composite);

        // Charger la nouvelle image
        QPixmap newImage(newImagePath);
        if (newImage.isNull()) {
            return QPixmap();
        }

        for (int i = 0; i < screens.count(); ++i) {
            QScreen* screen = screens[i];
            QRect screenRect = screen->geometry();

            // Calculer la position relative dans l'image composite
            QRect relativeRect = screenRect.translated(-virtualDesktop.topLeft());

            if (targetScreens.contains(i)) {
                // Appliquer la nouvelle image sur les écrans cibles
                QPixmap scaledImage = newImage.scaled(relativeRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

                // Centrer l'image si elle dépasse
                QRect sourceRect = QRect(0, 0, relativeRect.width(), relativeRect.height());
                if (scaledImage.width() > relativeRect.width()) {
                    sourceRect.setX((scaledImage.width() - relativeRect.width()) / 2);
                    sourceRect.setWidth(relativeRect.width());
                }
                if (scaledImage.height() > relativeRect.height()) {
                    sourceRect.setY((scaledImage.height() - relativeRect.height()) / 2);
                    sourceRect.setHeight(relativeRect.height());
                }

                painter.drawPixmap(relativeRect, scaledImage, sourceRect);
            } else {
                // Utiliser l'historique pour les autres écrans
                QString lastWallpaper = getLastWallpaperFromHistory(i);
                if (!lastWallpaper.isEmpty()) {
                    QPixmap historyImage(lastWallpaper);
                    if (!historyImage.isNull()) {
                        QPixmap scaledHistoryImage = historyImage.scaled(relativeRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

                        QRect sourceRect = QRect(0, 0, relativeRect.width(), relativeRect.height());
                        if (scaledHistoryImage.width() > relativeRect.width()) {
                            sourceRect.setX((scaledHistoryImage.width() - relativeRect.width()) / 2);
                            sourceRect.setWidth(relativeRect.width());
                        }
                        if (scaledHistoryImage.height() > relativeRect.height()) {
                            sourceRect.setY((scaledHistoryImage.height() - relativeRect.height()) / 2);
                            sourceRect.setHeight(relativeRect.height());
                        }

                        painter.drawPixmap(relativeRect, scaledHistoryImage, sourceRect);
                    } else {
                        // Fallback vers noir si l'image de l'historique n'est plus disponible
                        painter.fillRect(relativeRect, Qt::black);
                    }
                } else {
                    // Pas d'historique, utiliser du noir
                    painter.fillRect(relativeRect, Qt::black);
                }
            }
        }

        painter.end();
        return composite;
    }

    void addToScreenHistory(int screenIndex, const QString &imagePath, const QString &triggerMode = "Manuel")
    {
        if (screenIndex < 0) return;

        // Copier l'image dans le répertoire wallpapers
        QString localImagePath = copyImageToWallpapers(imagePath, screenIndex);
        if (localImagePath.isEmpty()) {
            return; // Échec de la copie
        }

        // Obtenir le mode d'ajustement actuel
        QString adjustMode = "Remplir"; // Mode par défaut
        if (adjustmentCombo) {
            QStringList modes = {"Remplir", "Ajuster", "Étendre", "Étirer", "Mosaïque"};
            int index = adjustmentCombo->currentIndex();
            if (index >= 0 && index < modes.size()) {
                adjustMode = modes[index];
            }
        }

        // Extraire le nom de fichier original
        QFileInfo originalFile(imagePath);
        QString filename = originalFile.fileName();

        // Enregistrer dans le log
        logWallpaperChange(screenIndex, filename, triggerMode, adjustMode);

        // Nettoyer les anciens fichiers wallpapers (ne garder que le fichier actuel)
        cleanupOldWallpapers(screenIndex, localImagePath);

        // Ajouter au début de la liste (ancien système conservé pour compatibilité)
        screenWallpaperHistory[screenIndex].prepend(localImagePath);

        // Limiter à MAX_HISTORY_SIZE
        if (screenWallpaperHistory[screenIndex].size() > MAX_HISTORY_SIZE) {
            QString removedPath = screenWallpaperHistory[screenIndex].takeLast();
            // Supprimer le fichier de l'historique s'il n'est plus utilisé
            removeUnusedHistoryFile(removedPath);
        }

        // Nettoyer le répertoire d'historique si nécessaire
        cleanupHistoryDirectory();

        // Sauvegarder l'historique
        saveHistoryToSettings();

        // Mettre à jour le carrousel d'historique fusionné
        loadMergedHistory();

        // Marquer cet écran comme pouvant être désélectionné (maintenant qu'il a un log)
        if (screenSelector) {
            screenSelector->setScreenCanBeDeselected(screenIndex, true);
        }
    }

    QString copyImageToHistory(const QString &sourcePath)
    {
        QString historyDir = getHistoryDirectory();

        // Générer un nom de fichier unique basé sur le hash du contenu
        QFile sourceFile(sourcePath);
        if (!sourceFile.open(QIODevice::ReadOnly)) {
            return QString();
        }

        QByteArray imageData = sourceFile.readAll();
        sourceFile.close();

        // Créer un hash pour le nom de fichier
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(imageData);
        QString hashString = hash.result().toHex();

        // Conserver l'extension originale
        QFileInfo sourceInfo(sourcePath);
        QString extension = sourceInfo.suffix();
        QString localFileName = QString("%1.%2").arg(hashString, extension);
        QString localPath = historyDir + "/" + localFileName;

        // Si le fichier existe déjà, pas besoin de le copier à nouveau
        if (QFile::exists(localPath)) {
            return localPath;
        }

        // Copier le fichier
        if (QFile::copy(sourcePath, localPath)) {
            return localPath;
        }

        return QString();
    }

    void removeUnusedHistoryFile(const QString &filePath)
    {
        // Vérifier si le fichier est encore utilisé dans l'historique d'autres écrans
        for (auto it = screenWallpaperHistory.constBegin(); it != screenWallpaperHistory.constEnd(); ++it) {
            if (it.value().contains(filePath)) {
                return; // Encore utilisé, ne pas supprimer
            }
        }

        // Plus utilisé, on peut supprimer
        QFile::remove(filePath);
    }

    void cleanupHistoryDirectory()
    {
        QString historyDir = getHistoryDirectory();
        QDir dir(historyDir);

        // Calculer la taille totale du répertoire
        qint64 totalSize = 0;
        QFileInfoList files = dir.entryInfoList(QDir::Files, QDir::Time);

        for (const QFileInfo &fileInfo : files) {
            totalSize += fileInfo.size();
        }

        // Si on dépasse la limite, supprimer les plus anciens fichiers
        while (totalSize > MAX_HISTORY_DIR_SIZE && !files.isEmpty()) {
            QFileInfo oldestFile = files.takeLast();

            // Vérifier que le fichier n'est pas dans l'historique actuel
            QString oldestPath = oldestFile.absoluteFilePath();
            bool inUse = false;
            for (auto it = screenWallpaperHistory.constBegin(); it != screenWallpaperHistory.constEnd(); ++it) {
                if (it.value().contains(oldestPath)) {
                    inUse = true;
                    break;
                }
            }

            if (!inUse) {
                totalSize -= oldestFile.size();
                QFile::remove(oldestPath);
            }
        }
    }

    void cleanupOldWallpapers(int screenIndex, const QString &currentFilePath)
    {
        QString wallpapersDir = getWallpapersDirectory();
        QString screenId = getScreenUniqueId(screenIndex);
        QString screenDir = wallpapersDir + "/" + screenId;

        QDir dir(screenDir);
        if (!dir.exists()) {
            return;
        }

        // Lister tous les fichiers dans le répertoire de cet écran
        QFileInfoList files = dir.entryInfoList(QDir::Files);

        // Supprimer tous les fichiers sauf le fichier actuel (nettoyage strict)
        for (const QFileInfo &fileInfo : files) {
            QString filePath = fileInfo.absoluteFilePath();

            // Ne pas supprimer le fichier actuel
            if (filePath != currentFilePath) {
                QFile::remove(filePath);
            }
        }
    }

    QStringList getFileNamesFromLogs(int screenIndex)
    {
        QStringList fileNames;
        QString screenId = getScreenUniqueId(screenIndex);
        QString historyDir = getHistoryDirectory();
        QString logFilePath = historyDir + "/" + screenId + ".log";

        QFile logFile(logFilePath);
        if (!logFile.open(QIODevice::ReadOnly)) {
            return fileNames;
        }

        QTextStream stream(&logFile);
        QStringList lines;
        QString line;

        // Lire toutes les lignes
        while (stream.readLineInto(&line)) {
            if (!line.isEmpty()) {
                lines.append(line);
            }
        }
        logFile.close();

        // Parser les lignes dans l'ordre inverse (du plus récent au plus ancien)
        QRegularExpression regex(R"(\[([^\]]+)\] \[([^\]]+)\] \[([^\]]+)\] \[([^\]]+)\])");

        for (int i = lines.size() - 1; i >= 0; i--) {
            QRegularExpressionMatch match = regex.match(lines[i]);
            if (match.hasMatch()) {
                QString filename = match.captured(2);

                // Ajouter uniquement si pas déjà dans la liste (dédoublonnage)
                if (!fileNames.contains(filename)) {
                    fileNames.append(filename);
                }
            }
        }

        return fileNames;
    }

    void cleanupAllWallpapers()
    {
        // Nettoyer les anciens fichiers wallpapers de tous les écrans
        for (int i = 0; i < QApplication::screens().size(); i++) {
            QString currentWallpaper = getCurrentWallpaperFromLog(i);
            if (!currentWallpaper.isEmpty()) {
                cleanupOldWallpapers(i, currentWallpaper);
            } else {
                // Si aucun fichier actuel, supprimer tout le répertoire de cet écran
                QString wallpapersDir = getWallpapersDirectory();
                QString screenId = getScreenUniqueId(i);
                QString screenDir = wallpapersDir + "/" + screenId;
                QDir dir(screenDir);
                if (dir.exists()) {
                    dir.removeRecursively();
                }
            }
        }
    }

    void cleanupTemporaryFiles()
    {
        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperAI";
        QDir dir(tempDir);

        if (!dir.exists()) {
            return;
        }

        // Lister tous les fichiers BMP et PNG temporaires
        QStringList filters;
        filters << "*.bmp" << "*.png";
        QFileInfoList tempFiles = dir.entryInfoList(filters, QDir::Files);

        // Supprimer tous les fichiers temporaires SAUF composite_wallpaper.bmp
        // qui est utilisé par Windows comme source du wallpaper
        for (const QFileInfo &fileInfo : tempFiles) {
            QString filename = fileInfo.fileName();

            // Ne pas supprimer le composite wallpaper actif
            if (filename != "composite_wallpaper.bmp") {
                QFile::remove(fileInfo.absoluteFilePath());
            }
        }
    }

    QString getLastWallpaperFromHistory(int screenIndex) const
    {
        // Récupérer uniquement depuis les logs
        return getCurrentWallpaperFromLog(screenIndex);
    }

    void saveSettings()
    {
        QSettings settings("WallpaperAI", "WallpaperSettings");

        // Sauvegarder la fréquence de changement
        settings.setValue("frequency/combo", frequencyCombo->currentIndex());
        settings.setValue("frequency/customValue", customValueSpinBox->value());
        settings.setValue("frequency/customUnit", customUnitCombo->currentIndex());

        // Sauvegarder le mode d'ajustement
        settings.setValue("adjustment/mode", adjustmentCombo->currentIndex());

        // Sauvegarder les options système
        settings.setValue("system/startupToggle", startupToggle->isChecked());
        settings.setValue("system/changeOnStartup", changeOnStartupToggle->isChecked());
        settings.setValue("system/multiScreen", multiScreenToggle->isChecked());

        // Sauvegarder l'historique des fonds d'écran
        saveHistoryToSettings();
    }

    void saveHistoryToSettings()
    {
        QSettings settings("WallpaperAI", "WallpaperSettings");

        // Effacer l'historique précédent
        settings.remove("ScreenHistory");

        // Sauvegarder l'historique par écran
        settings.beginGroup("ScreenHistory");
        for (auto it = screenWallpaperHistory.constBegin(); it != screenWallpaperHistory.constEnd(); ++it) {
            int screenIndex = it.key();
            const QStringList &history = it.value();

            settings.beginWriteArray(QString("Screen%1").arg(screenIndex));
            for (int i = 0; i < history.size(); ++i) {
                settings.setArrayIndex(i);
                settings.setValue("imagePath", history[i]);
            }
            settings.endArray();
        }
        settings.endGroup();
    }

    void loadSettings()
    {
        isLoadingSettings = true; // Empêcher l'activation du bouton pendant le chargement
        QSettings settings("WallpaperAI", "WallpaperSettings");

        // Charger la fréquence de changement
        int frequencyIndex = settings.value("frequency/combo", 0).toInt();
        if (frequencyIndex >= 0 && frequencyIndex < frequencyCombo->count()) {
            frequencyCombo->setCurrentIndex(frequencyIndex);
        }

        int customValue = settings.value("frequency/customValue", 30).toInt();
        customValueSpinBox->setValue(customValue);

        int customUnitIndex = settings.value("frequency/customUnit", 0).toInt();
        if (customUnitIndex >= 0 && customUnitIndex < customUnitCombo->count()) {
            customUnitCombo->setCurrentIndex(customUnitIndex);
        }

        // Charger le mode d'ajustement
        int adjustmentIndex = settings.value("adjustment/mode", 0).toInt();
        if (adjustmentIndex >= 0 && adjustmentIndex < adjustmentCombo->count()) {
            adjustmentCombo->setCurrentIndex(adjustmentIndex);
        }

        // Charger les options système
        bool startupEnabled = settings.value("system/startupToggle", true).toBool();
        startupToggle->setChecked(startupEnabled);

        bool changeOnStartupEnabled = settings.value("system/changeOnStartup", false).toBool();
        changeOnStartupToggle->setChecked(changeOnStartupEnabled);
        // Appliquer la logique de dépendance
        changeOnStartupToggle->setEnabled(startupEnabled);

        bool multiScreenEnabled = settings.value("system/multiScreen", true).toBool();
        multiScreenToggle->setChecked(multiScreenEnabled);

        // Charger les notations des catégories
        settings.beginGroup("categoryRatings");
        QStringList categoryKeys = settings.childKeys();
        for (const QString &categoryId : categoryKeys) {
            int rating = settings.value(categoryId, 1).toInt();
            categoryRatings[categoryId] = rating;
        }
        settings.endGroup();

        // Charger l'historique des fonds d'écran
        loadHistoryFromSettings();

        // Charger la préférence de l'avertissement multi-écrans
        dontShowMultiScreenWarning = settings.value("dontShowMultiScreenWarning", false).toBool();

        // Peupler le carrousel d'historique
        populateHistoryScreenCombo();

        isLoadingSettings = false; // Réactiver la détection des changements

        // Initialiser le countdown avec les paramètres chargés
        updateCountdownFromSettings();

        // Vérifier la cohérence des options de fréquence
        updateFrequencyOptions();

        // Initialiser l'affichage du sélecteur d'écran
        if (screenSelector) {
            onMultiScreenToggled(multiScreenToggle->isChecked());
        }
    }

    void loadHistoryFromSettings()
    {
        QSettings settings("WallpaperAI", "WallpaperSettings");

        // Charger l'historique par écran
        settings.beginGroup("ScreenHistory");
        QStringList screenGroups = settings.childGroups();

        for (const QString &screenGroup : screenGroups) {
            // Extraire l'index de l'écran (ex: "Screen0" -> 0)
            if (screenGroup.startsWith("Screen")) {
                bool ok;
                int screenIndex = screenGroup.mid(6).toInt(&ok); // "Screen" = 6 caractères

                if (ok) {
                    QStringList history;
                    int size = settings.beginReadArray(screenGroup);

                    for (int i = 0; i < size; ++i) {
                        settings.setArrayIndex(i);
                        QString imagePath = settings.value("imagePath").toString();

                        // Vérifier que le fichier existe encore
                        if (QFile::exists(imagePath)) {
                            history.append(imagePath);
                        }
                    }
                    settings.endArray();

                    if (!history.isEmpty()) {
                        screenWallpaperHistory[screenIndex] = history;
                    }

                    // Marquer cet écran comme pouvant être désélectionné selon les logs
                    if (screenSelector && hasScreenLog(screenIndex)) {
                        screenSelector->setScreenCanBeDeselected(screenIndex, true);
                    }
                }
            }
        }
        settings.endGroup();
    }

    void saveCategoryRating(const QString &categoryId, int rating)
    {
        QSettings settings("WallpaperAI", "WallpaperSettings");
        settings.beginGroup("categoryRatings");
        settings.setValue(categoryId, rating);
        settings.endGroup();
    }

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QNetworkAccessManager *networkManager;
    QTabWidget *tabWidget;
    QScrollArea *scrollArea;
    QWidget *categoriesWidget;
    QGridLayout *categoriesGridLayout;
    QPushButton *changeNowButton;
    QLabel *statusLabel;
    CountdownWidget *countdownWidget;
    QLabel *adjustmentExplanationLabel; // Label pour l'explication du mode d'ajustement
    QMap<QString, int> categoryRatings; // Stockage des notations des catégories
    QSet<QString> excludedCategories; // Catégories exclues pour cette session
    ScreenSelector *screenSelector; // Sélecteur d'écran

    // Protection contre les appels multiples de onDisplayConfigurationChanged
    bool m_isHandlingDisplayChange = false;
    QTimer *m_displayChangeTimer = nullptr;

    // Widget d'alerte erreur API
    QWidget *errorAlertWidget;
    QLabel *errorAlertLabel;
    QTimer *retryTimer;
    int retryCountdownSeconds;

    // Cache des catégories
    QJsonArray cachedCategories;
    QString categoriesCachePath;
    QMap<QString, QString> categoryThumbnailCache; // Map categoryId -> filename de la miniature

    // Historique des fonds d'écran par écran (max 8 images)
    QMap<int, QStringList> screenWallpaperHistory;
    static const int MAX_HISTORY_SIZE = 8;
    static const qint64 MAX_HISTORY_DIR_SIZE = 3 * 1024 * 1024; // 3 Mo

    // Carrousel d'historique
    QScrollArea *historyCarouselScrollArea;
    QWidget *historyCarouselWidget;
    QHBoxLayout *historyCarouselLayout;
    QComboBox *historyScreenCombo;
    QPushButton *historyPrevButton;
    QPushButton *historyNextButton;
    int currentHistoryScreen;
    int historyScrollOffset; // Offset de défilement en nombre d'images
    QString selectedHistoryImagePath;
    int selectedHistoryScreenIndex; // Écran d'origine de l'image sélectionnée
    bool dontShowMultiScreenWarning;
    QList<HistoryEntry> currentHistoryEntries; // Liste complète des entrées chargées

    // Sélectionne une catégorie aléatoire en utilisant la pondération par étoiles
    QString selectWeightedRandomCategory() const
    {
        QStringList weightedCategories;

        // Créer une liste pondérée basée sur les étoiles
        for (auto it = categoryRatings.constBegin(); it != categoryRatings.constEnd(); ++it) {
            const QString &categoryId = it.key();
            int rating = it.value();

            // Ignorer les catégories exclues pour cette session
            if (excludedCategories.contains(categoryId)) {
                continue;
            }

            // Ajouter la catégorie autant de fois que son nombre d'étoiles
            for (int i = 0; i < rating; ++i) {
                weightedCategories.append(categoryId);
            }
        }

        if (weightedCategories.isEmpty()) {
            return QString(); // Aucune catégorie disponible
        }

        // Tirage aléatoire dans la liste pondérée
        int randomIndex = QRandomGenerator::global()->bounded(weightedCategories.size());
        return weightedCategories.at(randomIndex);
    }

    // Obtient la date du jour au format DD/MM/YYYY
    QString getCurrentDateString() const
    {
        return DateHelper::getCurrentDateString();
    }

    // Obtient une date antérieure au format DD/MM/YYYY
    QString getPreviousDateString(int daysBack) const
    {
        return DateHelper::getPreviousDateString(daysBack);
    }

    QString getHistoryDirectory() const
    {
        return PathHelper::getHistoryDirectory();
    }

    QString getWallpapersDirectory() const
    {
        return PathHelper::getWallpapersDirectory();
    }

    QString getScreenUniqueId(int screenIndex) const
    {
        #ifdef Q_OS_WIN
        QList<QScreen*> screens = QApplication::screens();
        if (screenIndex < 0 || screenIndex >= screens.size()) {
            return QString("unknown_%1").arg(screenIndex);
        }

        QScreen* screen = screens[screenIndex];
        QString screenName = screen->name();

        // Utiliser QueryDisplayConfig pour obtenir l'identifiant unique
        UINT32 pathCount = 0;
        UINT32 modeCount = 0;

        // Première appelé pour obtenir la taille nécessaire
        LONG result = GetDisplayConfigBufferSizes(QDC_ALL_PATHS, &pathCount, &modeCount);
        if (result != ERROR_SUCCESS) {
            // Fallback sur le nom de l'écran nettoyé
            QString cleanName = screenName;
            cleanName.replace(QRegularExpression("[^A-Za-z0-9_-]"), "_");
            return QString("%1-unknown-unknown").arg(cleanName);
        }

        // Allouer les buffers
        QVector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
        QVector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

        // Obtenir la configuration de tous les chemins
        result = QueryDisplayConfig(QDC_ALL_PATHS, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);
        if (result != ERROR_SUCCESS) {
            QString cleanName = screenName;
            cleanName.replace(QRegularExpression("[^A-Za-z0-9_-]"), "_");
            return QString("%1-unknown-unknown").arg(cleanName);
        }

        // Chercher tous les écrans actifs et les mapper à l'index Qt
        int qtScreenIndex = 0;
        for (UINT32 i = 0; i < pathCount; i++) {
            // Vérifier si ce chemin est actif
            if (!(paths[i].flags & DISPLAYCONFIG_PATH_ACTIVE)) {
                continue;
            }

            DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
            targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
            targetName.header.size = sizeof(targetName);
            targetName.header.adapterId = paths[i].targetInfo.adapterId;
            targetName.header.id = paths[i].targetInfo.id;

            // Si c'est l'écran correspondant à notre index Qt
            if (qtScreenIndex == screenIndex) {
                // Obtenir le nom de l'écran
                if (DisplayConfigGetDeviceInfo(&targetName.header) == ERROR_SUCCESS) {
                    QString deviceName = QString::fromWCharArray(targetName.monitorFriendlyDeviceName);

                    // Si le nom de l'appareil est vide, utiliser IM (integrated monitor)
                    if (deviceName.isEmpty()) {
                        deviceName = "IM";
                    }

                    // Nettoyer le nom de l'appareil
                    QString cleanDeviceName = deviceName;
                    cleanDeviceName.replace(QRegularExpression("[^A-Za-z0-9_-]"), "_");

                    // Format: <DeviceName>-<adapter>-<output>
                    QString uniqueId = QString("%1-%2-%3")
                        .arg(cleanDeviceName)
                        .arg(paths[i].targetInfo.adapterId.HighPart)
                        .arg(paths[i].targetInfo.id);

                    return uniqueId;
                }
            }
            qtScreenIndex++;
        }
        #endif

        // Fallback : utiliser un ID basé sur l'index et le nom
        QString cleanName = screen->name();
        cleanName.replace(QRegularExpression("[^A-Za-z0-9_-]"), "_");
        QString fallbackId = QString("%1-%2-unknown").arg(cleanName).arg(screenIndex);
        return fallbackId;
    }

    void logWallpaperChange(int screenIndex, const QString &filename, const QString &triggerMode, const QString &adjustMode)
    {
        QString screenId = getScreenUniqueId(screenIndex);
        QString historyDir = getHistoryDirectory();
        QString logFilePath = historyDir + "/" + screenId + ".log";

        QFile logFile(logFilePath);
        if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
            QTextStream stream(&logFile);
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

            // Format: [timestamp] [filename] [trigger mode] [adjust mode]
            stream << "[" << timestamp << "] [" << filename << "] [" << triggerMode << "] [" << adjustMode << "]\n";
            logFile.close();
        }
    }

    QString getCurrentWallpaperFromLog(int screenIndex) const
    {
        QString screenId = getScreenUniqueId(screenIndex);
        return getCurrentWallpaperByScreenId(screenId);
    }

    QString getCurrentWallpaperByScreenId(const QString &screenId) const
    {
        QString historyDir = getHistoryDirectory();
        QString logFilePath = historyDir + "/" + screenId + ".log";

        QFile logFile(logFilePath);
        if (logFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&logFile);
            QString lastLine;
            QString line;

            // Lire toutes les lignes pour obtenir la dernière
            while (stream.readLineInto(&line)) {
                if (!line.isEmpty()) {
                    lastLine = line;
                }
            }
            logFile.close();

            // Parser la dernière ligne pour extraire le nom de fichier
            if (!lastLine.isEmpty()) {
                QRegularExpression regex(R"(\[([^\]]+)\] \[([^\]]+)\] \[([^\]]+)\] \[([^\]]+)\])");
                QRegularExpressionMatch match = regex.match(lastLine);
                if (match.hasMatch()) {
                    QString filename = match.captured(2);

                    // Construire le chemin complet du fichier dans le sous-répertoire de l'écran
                    QString wallpapersDir = getWallpapersDirectory();
                    QString fullPath = wallpapersDir + "/" + screenId + "/" + filename;

                    // Vérifier si le fichier existe
                    if (QFile::exists(fullPath)) {
                        return fullPath;
                    }
                }
            }
        }
        return QString(); // Aucun fond d'écran trouvé
    }

    bool hasScreenLog(int screenIndex) const
    {
        QString screenId = getScreenUniqueId(screenIndex);
        QString historyDir = getHistoryDirectory();
        QString logFilePath = historyDir + "/" + screenId + ".log";

        QFile logFile(logFilePath);
        if (!logFile.exists()) {
            return false;
        }

        // Vérifier que le fichier n'est pas vide
        if (logFile.open(QIODevice::ReadOnly)) {
            bool hasContent = !logFile.readAll().trimmed().isEmpty();
            logFile.close();
            return hasContent;
        }

        return false;
    }

    void initializeScreenDeselectionStates()
    {
        if (!screenSelector) return;

        // Marquer tous les écrans ayant des logs comme pouvant être désélectionnés
        for (int i = 0; i < screenSelector->screenCount(); i++) {
            if (hasScreenLog(i)) {
                screenSelector->setScreenCanBeDeselected(i, true);
            }
        }
    }

    QString copyImageToWallpapers(const QString &sourcePath, int screenIndex)
    {
        // Vérifier que le fichier source existe
        if (!QFile::exists(sourcePath)) {
            return QString();
        }

        QString wallpapersDir = getWallpapersDirectory();
        QString screenId = getScreenUniqueId(screenIndex);

        // Créer un sous-répertoire par écran pour éviter les conflits
        QString screenDir = wallpapersDir + "/" + screenId;
        QDir().mkpath(screenDir);

        // Utiliser le nom de fichier original
        QFileInfo sourceInfo(sourcePath);
        QString originalFileName = sourceInfo.fileName();
        QString localPath = screenDir + "/" + originalFileName;

        // Supprimer le fichier de destination s'il existe déjà
        if (QFile::exists(localPath)) {
            QFile::remove(localPath);
        }

        // Copier le fichier
        if (QFile::copy(sourcePath, localPath)) {
            // Si le fichier source se trouve dans le répertoire temporaire, le supprimer après copie
            QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperAI";
            if (sourcePath.startsWith(tempDir)) {
                QFile::remove(sourcePath);
            }
            return localPath;
        }

        return QString(); // Échec de la copie
    }

    // Contrôles de paramètres pour la sauvegarde/chargement
    QComboBox *frequencyCombo;
    QSpinBox *customValueSpinBox;
    QComboBox *customUnitCombo;
    QComboBox *adjustmentCombo;
    ToggleSwitch *startupToggle;
    ToggleSwitch *changeOnStartupToggle;
    ToggleSwitch *multiScreenToggle;
    QPushButton *applyButton;
    bool isLoadingSettings;
    QString currentTriggerMode;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ModernWindow window;

    // Vérifier si on est lancé au démarrage via les arguments de ligne de commande
    QStringList arguments = QCoreApplication::arguments();
    bool launchedAtStartup = arguments.contains("--startup");

    if (!launchedAtStartup) window.show();

    return app.exec();
}

#include "main.moc"