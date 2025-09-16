#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
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
#include <QPainter>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QSettings>
#include <QPainterPath>
#include <string>
#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>
#include <shobjidl.h>
#endif

// Classe pour gérer les événements de survol des catégories
class CategoryHoverFilter : public QObject
{
    Q_OBJECT

public:
    CategoryHoverFilter(QWidget *categoryFrame, const QString &categoryId, QWidget *parent)
        : QObject(parent), m_categoryFrame(categoryFrame), m_categoryId(categoryId), m_parent(parent) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (obj == m_categoryFrame) {
            if (event->type() == QEvent::Enter) {
                // Afficher les contrôles de notation au survol
                QWidget *ratingWidget = m_categoryFrame->findChild<QWidget*>("ratingWidget_" + m_categoryId);
                QWidget *currentRating = m_categoryFrame->findChild<QWidget*>("currentRating_" + m_categoryId);
                if (ratingWidget) {
                    ratingWidget->show();
                    // Masquer l'affichage permanent pour éviter la superposition
                    if (currentRating) currentRating->hide();

                    // Masquer aussi l'icône "sens interdit" si elle existe
                    QLabel *disableIcon = m_categoryFrame->parentWidget()->findChild<QLabel*>("disableIcon_" + m_categoryId);
                    if (disableIcon) disableIcon->hide();
                }
            } else if (event->type() == QEvent::Leave) {
                // Masquer les contrôles de notation
                QWidget *ratingWidget = m_categoryFrame->findChild<QWidget*>("ratingWidget_" + m_categoryId);
                QWidget *currentRating = m_categoryFrame->findChild<QWidget*>("currentRating_" + m_categoryId);
                if (ratingWidget) {
                    ratingWidget->hide();
                    // Réafficher l'affichage permanent
                    if (currentRating) currentRating->show();

                    // Réafficher l'icône "sens interdit" si la catégorie est désactivée
                    QVariant ratingVariant = m_parent->property(("rating_" + m_categoryId).toLocal8Bit().constData());
                    int currentRating = ratingVariant.isValid() ? ratingVariant.toInt() : 1;
                    if (currentRating == -1) {
                        QLabel *disableIcon = m_categoryFrame->parentWidget()->findChild<QLabel*>("disableIcon_" + m_categoryId);
                        if (disableIcon) disableIcon->show();
                    }
                }
            }
        }
        return QObject::eventFilter(obj, event);
    }

private:
    QWidget *m_categoryFrame;
    QString m_categoryId;
    QWidget *m_parent;
};

// Classe pour gérer les événements de survol des étoiles
class StarHoverFilter : public QObject
{
    Q_OBJECT

public:
    StarHoverFilter(QPushButton *starBtn, const QString &categoryId, int starIndex, QWidget *parent)
        : QObject(parent), m_starBtn(starBtn), m_categoryId(categoryId), m_starIndex(starIndex), m_parent(parent) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (obj == m_starBtn) {
            if (event->type() == QEvent::Enter) {
                // Preview: activer les étoiles jusqu'à celle survolée
                updateStarPreview(m_categoryId, m_starIndex);
            } else if (event->type() == QEvent::Leave) {
                // Restaurer l'affichage normal selon la notation actuelle
                // Chercher la valeur dans la map via une propriété dynamique du parent
                QVariant ratingVariant = m_parent->property(("rating_" + m_categoryId).toLocal8Bit().constData());
                int currentRating = ratingVariant.isValid() ? ratingVariant.toInt() : 1;
                updateStarPreview(m_categoryId, currentRating);
            }
        }
        return QObject::eventFilter(obj, event);
    }

private:
    void updateStarPreview(const QString &categoryId, int rating) {
        QWidget *ratingWidget = m_parent->findChild<QWidget*>("ratingWidget_" + categoryId);
        if (!ratingWidget) return;

        for (int i = 1; i <= 3; i++) {
            QPushButton *starBtn = ratingWidget->findChild<QPushButton*>(QString("star_%1_%2").arg(categoryId).arg(i));
            if (starBtn) {
                if (i <= rating) {
                    QPixmap starPixmap("star_active.png");
                    if (!starPixmap.isNull()) {
                        starBtn->setIcon(QIcon(starPixmap));
                    } else {
                        starBtn->setText("★");
                        starBtn->setStyleSheet("QPushButton { border: none; background: transparent; color: gold; } QPushButton:hover { background: rgba(255,255,255,0.2); }");
                    }
                } else {
                    QPixmap starPixmap("star_inactive.png");
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

    QPushButton *m_starBtn;
    QString m_categoryId;
    int m_starIndex;
    QWidget *m_parent;
};

// Classe pour gérer les événements de survol du bouton "sens interdit"
class DisableHoverFilter : public QObject
{
    Q_OBJECT

public:
    DisableHoverFilter(QPushButton *disableBtn, const QString &categoryId, QWidget *parent)
        : QObject(parent), m_disableBtn(disableBtn), m_categoryId(categoryId), m_parent(parent) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (obj == m_disableBtn) {
            if (event->type() == QEvent::Enter) {
                // Preview: désactiver toutes les étoiles (afficher comme "sens interdit")
                updateStarPreview(m_categoryId, -1);
            } else if (event->type() == QEvent::Leave) {
                // Restaurer l'affichage normal selon la notation actuelle
                QVariant ratingVariant = m_parent->property(("rating_" + m_categoryId).toLocal8Bit().constData());
                int currentRating = ratingVariant.isValid() ? ratingVariant.toInt() : 1;
                updateStarPreview(m_categoryId, currentRating);
            }
        }
        return QObject::eventFilter(obj, event);
    }

private:
    void updateStarPreview(const QString &categoryId, int rating) {
        QWidget *ratingWidget = m_parent->findChild<QWidget*>("ratingWidget_" + categoryId);
        if (!ratingWidget) return;

        if (rating == -1) {
            // Mode "sens interdit" : désactiver toutes les étoiles
            for (int i = 1; i <= 3; i++) {
                QPushButton *starBtn = ratingWidget->findChild<QPushButton*>(QString("star_%1_%2").arg(categoryId).arg(i));
                if (starBtn) {
                    QPixmap starPixmap("star_inactive.png");
                    if (!starPixmap.isNull()) {
                        starBtn->setIcon(QIcon(starPixmap));
                    } else {
                        starBtn->setText("★");
                        starBtn->setStyleSheet("QPushButton { border: none; background: transparent; color: gray; } QPushButton:hover { background: rgba(255,255,255,0.2); }");
                    }
                }
            }
        } else {
            // Mode normal : afficher selon la notation
            for (int i = 1; i <= 3; i++) {
                QPushButton *starBtn = ratingWidget->findChild<QPushButton*>(QString("star_%1_%2").arg(categoryId).arg(i));
                if (starBtn) {
                    if (i <= rating) {
                        QPixmap starPixmap("star_active.png");
                        if (!starPixmap.isNull()) {
                            starBtn->setIcon(QIcon(starPixmap));
                        } else {
                            starBtn->setText("★");
                            starBtn->setStyleSheet("QPushButton { border: none; background: transparent; color: gold; } QPushButton:hover { background: rgba(255,255,255,0.2); }");
                        }
                    } else {
                        QPixmap starPixmap("star_inactive.png");
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

    QPushButton *m_disableBtn;
    QString m_categoryId;
    QWidget *m_parent;
};

// Classe pour le sélecteur d'écran avec esthétique moderne
class ScreenSelector : public QWidget
{
    Q_OBJECT

public:
    ScreenSelector(QWidget *parent = nullptr) : QWidget(parent), m_currentScreen(0)
    {
        setFixedHeight(35);
        setupScreens();
        setupUI();
    }

    void setCurrentScreen(int screenIndex)
    {
        if (screenIndex >= 0 && screenIndex < m_screenCount && screenIndex != m_currentScreen) {
            m_currentScreen = screenIndex;
            update();
            emit screenChanged(screenIndex);
        }
    }

    int currentScreen() const { return m_currentScreen; }
    int screenCount() const { return m_screenCount; }

signals:
    void screenChanged(int screenIndex);

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int tabWidth = width() / m_screenCount;
        int borderRadius = 6;

        // Dessiner d'abord le fond global avec coins arrondis
        QRect globalRect(0, 0, width(), height());
        painter.setBrush(QColor("#8b4513")); // Couleur de fond par défaut
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(globalRect, borderRadius, borderRadius);

        // Puis dessiner l'onglet actif par-dessus avec clipping pour respecter les coins arrondis
        painter.save();

        // Créer un chemin de clipping avec les coins arrondis
        QPainterPath clipPath;
        clipPath.addRoundedRect(globalRect, borderRadius, borderRadius);
        painter.setClipPath(clipPath);

        for (int i = 0; i < m_screenCount; i++) {
            QRect tabRect(i * tabWidth, 0, tabWidth, height());

            // Dessiner seulement l'onglet actif avec une couleur différente
            if (i == m_currentScreen) {
                painter.setBrush(QColor("#d14836")); // Rouge-orange actif
                painter.setPen(Qt::NoPen);
                painter.drawRect(tabRect);
            }
        }

        painter.restore();

        // Dessiner les séparateurs entre les onglets
        painter.setPen(QPen(QColor("#5a3a1a"), 1)); // Ligne de séparation plus foncée
        for (int i = 1; i < m_screenCount; i++) {
            int x = i * tabWidth;
            painter.drawLine(x, 2, x, height() - 2);
        }

        // Dessiner le texte pour tous les onglets
        painter.setPen(QColor("#ffffff"));
        painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
        for (int i = 0; i < m_screenCount; i++) {
            QRect tabRect(i * tabWidth, 0, tabWidth, height());
            QString text = QString("Écran %1").arg(i + 1);
            painter.drawText(tabRect, Qt::AlignCenter, text);
        }
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            int tabWidth = width() / m_screenCount;
            int clickedScreen = static_cast<int>(event->position().x()) / tabWidth;
            if (clickedScreen >= 0 && clickedScreen < m_screenCount) {
                setCurrentScreen(clickedScreen);
            }
        }
    }

private:
    void setupScreens()
    {
        // Détecter le nombre d'écrans
        m_screenCount = QApplication::screens().count();
        if (m_screenCount < 1) {
            m_screenCount = 1; // Au moins un écran par défaut
        }
    }

    void setupUI()
    {
        // Calculer la largeur optimale basée sur le nombre d'écrans
        int optimalWidth = m_screenCount * 80; // 80px par onglet
        setMinimumSize(optimalWidth, 35);
        setMaximumHeight(35);
        // Permettre l'expansion horizontale pour l'alignement justifié
    }

private:
    int m_currentScreen;
    int m_screenCount;
};

// Classe pour afficher un compte à rebours avec camembert de progression
class CountdownWidget : public QWidget
{
    Q_OBJECT

public:
    CountdownWidget(QWidget *parent = nullptr) : QWidget(parent), m_totalSeconds(3600), m_remainingSeconds(3600), m_isStartupMode(false)
    {
        setFixedSize(200, 200);

        // Timer pour mise à jour chaque seconde
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &CountdownWidget::updateCountdown);
        m_timer->start(1000); // Mise à jour chaque seconde
    }

    void setDuration(int seconds)
    {
        m_totalSeconds = seconds;
        m_remainingSeconds = seconds;
        m_isStartupMode = (seconds == 0);
        update();
    }

    void setRemainingTime(int seconds)
    {
        m_remainingSeconds = seconds;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        if (m_isStartupMode) {
            // Mode "Au démarrage" : affichage textuel sans camembert
            painter.setPen(QColor("#ffffff"));
            painter.setFont(QFont("Segoe UI", 11, QFont::Bold));

            QRect textRect = rect().adjusted(10, 10, -10, -10);
            painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap,
                "Changement de fond d'écran au prochain redémarrage de l'ordinateur");
            return;
        }

        QRect circleRect = rect().adjusted(20, 20, -20, -20);

        // Calculer le pourcentage de progression (0% = plein, 100% = vide)
        double progressPercent = 0.0;
        if (m_totalSeconds > 0) {
            progressPercent = (double)(m_totalSeconds - m_remainingSeconds) / m_totalSeconds;
        }

        // Fond du cercle
        painter.setBrush(QColor("#404040"));
        painter.setPen(QPen(QColor("#555"), 2));
        painter.drawEllipse(circleRect);

        // Arc de progression (commence en haut, sens horaire)
        if (progressPercent > 0) {
            painter.setBrush(QColor("#0078d4"));
            painter.setPen(QPen(QColor("#0078d4"), 3));

            int startAngle = 90 * 16; // Commencer en haut (90 degrés)
            int spanAngle = -(int)(progressPercent * 360 * 16); // Sens horaire (négatif)

            painter.drawPie(circleRect, startAngle, spanAngle);
        }

        // Texte central - temps restant uniquement
        painter.setPen(QColor("#ffffff"));
        painter.setFont(QFont("Segoe UI", 12, QFont::Bold));

        // Formatage du temps restant
        int hours = m_remainingSeconds / 3600;
        int minutes = (m_remainingSeconds % 3600) / 60;
        int seconds = m_remainingSeconds % 60;

        QString timeText;
        if (hours > 0) {
            timeText = QString("%1h %2m").arg(hours).arg(minutes);
        } else if (minutes > 0) {
            timeText = QString("%1m %2s").arg(minutes).arg(seconds);
        } else {
            timeText = QString("%1s").arg(seconds);
        }

        QRect textRect = circleRect.adjusted(10, -10, -10, 10);
        painter.drawText(textRect, Qt::AlignCenter, timeText);
    }

private slots:
    void updateCountdown()
    {
        if (m_isStartupMode) {
            // Pas de countdown en mode démarrage
            return;
        }

        if (m_remainingSeconds > 0) {
            m_remainingSeconds--;
            update();
        } else {
            // Temps écoulé, réinitialiser
            m_remainingSeconds = m_totalSeconds;
            update();
        }
    }

private:
    int m_totalSeconds;
    int m_remainingSeconds;
    bool m_isStartupMode;
    QTimer *m_timer;
};

// Classe pour créer un bouton à bascule personnalisé avec animation
class ToggleSwitch : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal animationProgress READ animationProgress WRITE setAnimationProgress)

public:
    ToggleSwitch(QWidget *parent = nullptr) : QWidget(parent), m_checked(false), m_animationProgress(0.0)
    {
        setFixedSize(54, 30);
        setCursor(Qt::PointingHandCursor);

        // Configuration de l'animation
        m_animation = new QPropertyAnimation(this, "animationProgress");
        m_animation->setDuration(200); // 200ms pour une animation fluide
        m_animation->setEasingCurve(QEasingCurve::OutCubic);

        connect(m_animation, &QPropertyAnimation::valueChanged, this, QOverload<>::of(&QWidget::update));
    }

    bool isChecked() const { return m_checked; }

    void setChecked(bool checked)
    {
        if (m_checked != checked) {
            m_checked = checked;

            // Démarrer l'animation
            m_animation->setStartValue(m_animationProgress);
            m_animation->setEndValue(checked ? 1.0 : 0.0);
            m_animation->start();

            emit toggled(m_checked);
        }
    }

    qreal animationProgress() const { return m_animationProgress; }
    void setAnimationProgress(qreal progress)
    {
        m_animationProgress = progress;
        update();
    }

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // Fond du switch avec interpolation de couleur
        QRect switchRect = rect().adjusted(3, 3, -3, -3);

        // Interpoler entre gris et bleu selon l'animation
        QColor backgroundColor = QColor::fromRgb(
            static_cast<int>(102 + (0 - 102) * m_animationProgress),     // Rouge: 102 -> 0
            static_cast<int>(102 + (120 - 102) * m_animationProgress),   // Vert: 102 -> 120
            static_cast<int>(102 + (212 - 102) * m_animationProgress)    // Bleu: 102 -> 212
        );

        painter.setBrush(backgroundColor);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(switchRect, 12, 12);

        // Bouton circulaire avec position animée - plus petit pour plus d'espace
        int buttonSize = switchRect.height() - 8;  // Plus d'espace autour (8 au lieu de 4)
        int leftPos = switchRect.left() + 4;       // Plus de marge
        int rightPos = switchRect.right() - buttonSize - 4;  // Plus de marge
        int buttonX = static_cast<int>(leftPos + (rightPos - leftPos) * m_animationProgress);
        int buttonY = switchRect.top() + (switchRect.height() - buttonSize) / 2;

        painter.setBrush(QColor("#ffffff"));
        painter.drawEllipse(buttonX, buttonY, buttonSize, buttonSize);
    }

    void mousePressEvent(QMouseEvent *) override
    {
        setChecked(!m_checked);
    }

private:
    bool m_checked;
    qreal m_animationProgress;
    QPropertyAnimation *m_animation;
};

class ModernWindow : public QWidget
{
    Q_OBJECT

public:
    ModernWindow(QWidget *parent = nullptr) : QWidget(parent), networkManager(new QNetworkAccessManager(this)), isLoadingSettings(false)
    {
        setWindowTitle("WallpaperIA - Gestionnaire de Fonds d'écran");
        setWindowIcon(QIcon("icon.png"));
        setFixedSize(725, 650);

        // Configuration des boutons de la fenêtre : minimiser et fermer (pas d'agrandissement)
        setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

        setupUI();
        applyModernStyle();
        setupSystemTray();
        loadCategories();
        loadSettings();
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

    void onMultiScreenToggled(bool enabled) {
        if (screenSelector) {
            if (enabled && screenSelector->screenCount() > 1) {
                screenSelector->show();
                // Mettre à jour le statut pour indiquer l'écran sélectionné
                statusLabel->setText(QString("Écran sélectionné : %1").arg(screenSelector->currentScreen() + 1));
            } else {
                screenSelector->hide();
                // Remettre le statut par défaut
                statusLabel->setText("Cliquez pour changer le fond d'écran");
            }
        }
    }

    void updateCountdownFromSettings() {
        if (!countdownWidget) return;

        int seconds = 3600; // Valeur par défaut (1h)

        // Calculer la durée en secondes selon la sélection
        int frequencyIndex = frequencyCombo->currentIndex();
        switch (frequencyIndex) {
            case 0: seconds = 3600; break;     // 1h
            case 1: seconds = 10800; break;    // 3h
            case 2: seconds = 21600; break;    // 6h
            case 3: seconds = 43200; break;    // 12h
            case 4: seconds = 86400; break;    // 24h
            case 5: seconds = 604800; break;   // 7j
            case 6: seconds = 0; break;        // Au démarrage (pas de timer)
            case 7: // Autre
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
        QLabel *titleLabel = new QLabel("WallpaperIA - Gestionnaire de Fonds d'écran");
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
        controlsContainer->setFixedWidth(280); // Largeur fixe pour alignement justifié
        QVBoxLayout *containerLayout = new QVBoxLayout(controlsContainer);
        containerLayout->setContentsMargins(0, 0, 0, 0);
        containerLayout->setSpacing(10);

        // Sélecteur d'écran en haut
        screenSelector = new ScreenSelector();
        screenSelector->hide(); // Masqué par défaut
        screenSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // Expansion horizontale
        connect(screenSelector, &ScreenSelector::screenChanged, this, &ModernWindow::onScreenChanged);
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
        );
        connect(changeNowButton, &QPushButton::clicked, this, &ModernWindow::onChangeNowClicked);
        containerLayout->addWidget(changeNowButton);

        // Label pour le statut sous le bouton
        statusLabel = new QLabel("Cliquez pour changer le fond d'écran");
        statusLabel->setAlignment(Qt::AlignLeft);
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
        rightCountdownLayout->addWidget(countdownWidget, 0, Qt::AlignCenter);

        // Assemblage du layout principal - centrer les groupes
        mainLayout->addStretch(); // Espacement à gauche pour centrer
        mainLayout->addLayout(leftControlsLayout);
        mainLayout->addLayout(rightCountdownLayout);
        mainLayout->addStretch(); // Espacement à droite pour centrer

        applicationLayout->addLayout(mainLayout);

        // Espace pour le futur carrousel
        applicationLayout->addStretch();

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
            "1h",
            "3h",
            "6h",
            "12h",
            "24h",
            "7j",
            "Au démarrage de l'ordinateur",
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
        adjustmentGroup->setMinimumHeight(282); // Hauteur minimale pour alignement avec les deux cadres de gauche
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
        adjustmentLayout->setSpacing(15);
        adjustmentLayout->setContentsMargins(15, 15, 15, 15);

        // Layout horizontal pour centrer les éléments
        QHBoxLayout *centeringLayout = new QHBoxLayout();
        centeringLayout->addStretch(); // Espacement à gauche

        // Widget container pour le sélecteur et l'image
        QWidget *selectorWidget = new QWidget();
        selectorWidget->setStyleSheet("QWidget { background-color: #3a3a3a; }");
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
        adjustmentCombo->setFixedWidth(150);
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
        QPixmap defaultPixmap("wallpaper_fill_icon.png");
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
            QPixmap modePixmap(QString("wallpaper_%1_icon.png").arg(mode));
            if (!modePixmap.isNull()) {
                // Utiliser une taille beaucoup plus grande pour remplir vraiment l'espace
                adjustmentImageLabel->setPixmap(modePixmap.scaled(136, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                adjustmentImageLabel->setText(adjustmentCombo->currentText());
                adjustmentImageLabel->setStyleSheet("QLabel { border: 1px solid #555; border-radius: 8px; background-color: #e8e8e8; padding: 0px; color: #333333; font-weight: bold; }");
            }
            // TODO: Sauvegarder le mode d'ajustement sélectionné
        });

        selectorLayout->addWidget(adjustmentCombo, 0, Qt::AlignCenter);
        selectorLayout->addWidget(adjustmentImageLabel, 0, Qt::AlignCenter);

        // Connecter au détecteur de changements (en plus de la connexion existante)
        connect(adjustmentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModernWindow::onSettingsChanged);

        centeringLayout->addWidget(selectorWidget);
        centeringLayout->addStretch(); // Espacement à droite

        adjustmentLayout->addLayout(centeringLayout);

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
        connect(multiScreenToggle, &ToggleSwitch::toggled, this, &ModernWindow::onSettingsChanged);
        connect(multiScreenToggle, &ToggleSwitch::toggled, this, &ModernWindow::onMultiScreenToggled);

        leftColumnLayout->addWidget(systemGroup);
        leftColumnLayout->addStretch();

        // Colonne de droite (Mode d'ajustement de l'image)
        QVBoxLayout *rightColumnLayout = new QVBoxLayout();
        rightColumnLayout->addWidget(adjustmentGroup);
        rightColumnLayout->addStretch();

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
        trayIcon->setIcon(QIcon("icon.png"));
        trayIcon->setToolTip("WallpaperIA");
        
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

    void loadCategories()
    {
        QNetworkRequest request(QUrl("http://localhost:8080/WallpaperIA/api/categories"));
        QNetworkReply *reply = networkManager->get(request);
        
        connect(reply, &QNetworkReply::finished, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonObject obj = doc.object();
                
                if (obj["success"].toBool()) {
                    QJsonArray categories = obj["data"].toArray();
                    displayCategories(categories);
                }
            }
            reply->deleteLater();
        });
    }
    
    void displayCategories(const QJsonArray &categories)
    {
        int row = 0, col = 0;
        const int maxCols = 3;
        
        for (const QJsonValue &value : categories) {
            QJsonObject category = value.toObject();
            QString categoryName = category["name"].toString();
            QString categoryId = category["id"].toString();
            
            createCategoryWidget(categoryName, categoryId, row, col);
            
            col++;
            if (col >= maxCols) {
                col = 0;
                row++;
            }
        }
    }
    
    void createCategoryWidget(const QString &name, const QString &id, int row, int col)
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

        // Charger la première image de la catégorie comme miniature
        loadCategoryThumbnail(id, thumbnailLabel);
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

        QPixmap disablePixmap("disable_category.png");
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

            QPixmap starPixmap("star_inactive.png");
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
                        QPixmap starPixmap("star_active.png");
                        if (!starPixmap.isNull()) {
                            starBtn->setIcon(QIcon(starPixmap));
                        } else {
                            starBtn->setText("★");
                            starBtn->setStyleSheet("QPushButton { border: none; background: transparent; color: gold; } QPushButton:hover { background: rgba(255,255,255,0.2); }");
                        }
                    } else {
                        QPixmap starPixmap("star_inactive.png");
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

            QPixmap disablePixmap("disable_category.png");
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
                    QPixmap starPixmap("star_active.png");
                    if (!starPixmap.isNull()) {
                        star->setPixmap(starPixmap.scaled(12, 12, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    } else {
                        star->setText("★");
                        star->setStyleSheet("color: gold; font-size: 10px;");
                        star->setAlignment(Qt::AlignCenter);
                    }
                } else {
                    QPixmap starPixmap("star_inactive.png");
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
        QNetworkRequest request(QUrl(QString("http://localhost:8080/WallpaperIA/api/wallpapers?category=%1").arg(categoryId)));
        QNetworkReply *reply = networkManager->get(request);
        
        connect(reply, &QNetworkReply::finished, [this, reply, thumbnailLabel]() {
            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonObject obj = doc.object();
                
                if (obj["success"].toBool()) {
                    QJsonArray wallpapers = obj["data"].toArray();
                    if (!wallpapers.isEmpty()) {
                        QString filename = wallpapers[0].toObject()["filename"].toString();
                        loadThumbnailImage(filename, thumbnailLabel);
                    }
                }
            }
            reply->deleteLater();
        });
    }
    
    void loadThumbnailImage(const QString &filename, QLabel *thumbnailLabel)
    {
        // Utiliser l'endpoint /mini/ pour les miniatures optimisées (90x50 JPG)
        QNetworkRequest request(QUrl(QString("http://localhost:8080/WallpaperIA/api/mini/%1").arg(filename)));
        QNetworkReply *reply = networkManager->get(request);
        
        connect(reply, &QNetworkReply::finished, [this, reply, thumbnailLabel, filename]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    // Les miniatures sont déjà optimisées (90x50), on les scale juste à la taille du label
                    QPixmap scaledPixmap = pixmap.scaled(200, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    thumbnailLabel->setPixmap(scaledPixmap);
                    thumbnailLabel->setText(""); // Enlever le texte placeholder
                }
            } else {
                // En cas d'erreur avec /mini/, fallback vers l'ancienne méthode
                loadThumbnailImageFallback(filename, thumbnailLabel);
            }
            reply->deleteLater();
        });
    }
    
    void loadThumbnailImageFallback(const QString &filename, QLabel *thumbnailLabel)
    {
        // Méthode de fallback avec l'image complète (pour compatibilité)
        QNetworkRequest request(QUrl(QString("http://localhost:8080/WallpaperIA/api/get/%1").arg(filename)));
        QNetworkReply *reply = networkManager->get(request);
        
        connect(reply, &QNetworkReply::finished, [this, reply, thumbnailLabel]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    // Redimensionner l'image pour la miniature
                    QPixmap scaledPixmap = pixmap.scaled(200, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    thumbnailLabel->setPixmap(scaledPixmap);
                    thumbnailLabel->setText(""); // Enlever le texte placeholder
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
    void onScreenChanged(int screenIndex)
    {
        // Mettre à jour le statut pour indiquer l'écran sélectionné
        if (screenSelector->screenCount() > 1) {
            statusLabel->setText(QString("Écran sélectionné : %1").arg(screenIndex + 1));
        }
    }

    void onChangeNowClicked()
    {
        // Désactiver le bouton pendant le processus
        changeNowButton->setEnabled(false);
        changeNowButton->setText("🔄 Changement en cours...");
        statusLabel->setText("Récupération d'une image aléatoire...");

        // Obtenir toutes les catégories et choisir une image au hasard
        getRandomWallpaper();
    }

private:
    void getRandomWallpaper()
    {
        QNetworkRequest request(QUrl("http://localhost:8080/WallpaperIA/api/categories"));
        QNetworkReply *reply = networkManager->get(request);

        connect(reply, &QNetworkReply::finished, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonObject obj = doc.object();

                if (obj["success"].toBool()) {
                    QJsonArray categories = obj["data"].toArray();
                    if (!categories.isEmpty()) {
                        // Choisir une catégorie au hasard
                        int randomCategoryIndex = QRandomGenerator::global()->bounded(categories.size());
                        QString categoryId = categories[randomCategoryIndex].toObject()["id"].toString();

                        // Obtenir les images de cette catégorie
                        getRandomImageFromCategory(categoryId);
                    } else {
                        restoreButton("Aucune catégorie disponible");
                    }
                } else {
                    restoreButton("Erreur API");
                }
            } else {
                restoreButton("Erreur de connexion");
            }
            reply->deleteLater();
        });
    }

    void getRandomImageFromCategory(const QString &categoryId)
    {
        QNetworkRequest request(QUrl(QString("http://localhost:8080/WallpaperIA/api/wallpapers?category=%1").arg(categoryId)));
        QNetworkReply *reply = networkManager->get(request);

        connect(reply, &QNetworkReply::finished, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonObject obj = doc.object();

                if (obj["success"].toBool()) {
                    QJsonArray wallpapers = obj["data"].toArray();
                    if (!wallpapers.isEmpty()) {
                        // Choisir une image au hasard
                        int randomIndex = QRandomGenerator::global()->bounded(wallpapers.size());
                        QString filename = wallpapers[randomIndex].toObject()["filename"].toString();

                        statusLabel->setText("Téléchargement de l'image...");
                        downloadAndSetWallpaper(filename);
                    } else {
                        restoreButton("Aucune image dans cette catégorie");
                    }
                } else {
                    restoreButton("Erreur lors de la récupération des images");
                }
            } else {
                restoreButton("Erreur de connexion");
            }
            reply->deleteLater();
        });
    }

    void downloadAndSetWallpaper(const QString &filename)
    {
        QNetworkRequest request(QUrl(QString("http://localhost:8080/WallpaperIA/api/get/%1").arg(filename)));
        QNetworkReply *reply = networkManager->get(request);

        connect(reply, &QNetworkReply::finished, [this, reply, filename]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();

                // Créer un dossier temp pour WallpaperIA
                QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperIA";
                QDir().mkpath(tempDir);

                // Utiliser le nom original pour permettre le cache Windows
                QString tempFilePath = tempDir + "/" + filename;

                // Convertir l'image en BMP comme Firefox (plus fiable pour Windows)
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    // Sauvegarder en BMP pour une compatibilité maximale avec Windows
                    QString bmpPath = tempDir + "/" + QFileInfo(filename).baseName() + ".bmp";
                    pixmap.save(bmpPath, "BMP");
                    tempFilePath = bmpPath;
                } else {
                    // Fallback: écrire le fichier original
                    QFile file(tempFilePath);
                    if (!file.exists() || file.size() != imageData.size()) {
                        if (file.open(QIODevice::WriteOnly)) {
                            file.write(imageData);
                            file.close();
                        }
                    }
                }

                statusLabel->setText("Application du fond d'écran...");

                // Appliquer le fond d'écran
                if (setWindowsWallpaper(tempFilePath)) {
                    statusLabel->setText(QString("Fond d'écran: %1").arg(filename));
                    restoreButton("Succès !");
                } else {
                    restoreButton("Erreur lors de l'application");
                }
            } else {
                restoreButton("Erreur de téléchargement");
            }
            reply->deleteLater();
        });
    }

    bool setWindowsWallpaper(const QString &imagePath)
    {
        #ifdef Q_OS_WIN
        QFileInfo fileInfo(imagePath);
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            return false;
        }

        // Convertir le chemin pour Windows
        std::wstring wImagePath = imagePath.toStdWString();

        // Utiliser la version Unicode comme Firefox pour plus de compatibilité
        BOOL result = SystemParametersInfoW(
            SPI_SETDESKWALLPAPER,
            0,
            (PVOID)wImagePath.c_str(),
            SPIF_UPDATEINIFILE | SPIF_SENDCHANGE
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
    }

    void saveSettings()
    {
        QSettings settings("WallpaperIA", "WallpaperSettings");

        // Sauvegarder la fréquence de changement
        settings.setValue("frequency/combo", frequencyCombo->currentIndex());
        settings.setValue("frequency/customValue", customValueSpinBox->value());
        settings.setValue("frequency/customUnit", customUnitCombo->currentIndex());

        // Sauvegarder le mode d'ajustement
        settings.setValue("adjustment/mode", adjustmentCombo->currentIndex());

        // Sauvegarder les options système
        settings.setValue("system/startupToggle", startupToggle->isChecked());
        settings.setValue("system/multiScreen", multiScreenToggle->isChecked());
    }

    void loadSettings()
    {
        isLoadingSettings = true; // Empêcher l'activation du bouton pendant le chargement
        QSettings settings("WallpaperIA", "WallpaperSettings");

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

        isLoadingSettings = false; // Réactiver la détection des changements

        // Initialiser le countdown avec les paramètres chargés
        updateCountdownFromSettings();

        // Initialiser l'affichage du sélecteur d'écran
        if (screenSelector) {
            onMultiScreenToggled(multiScreenToggle->isChecked());
        }
    }

    void saveCategoryRating(const QString &categoryId, int rating)
    {
        QSettings settings("WallpaperIA", "WallpaperSettings");
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
    QMap<QString, int> categoryRatings; // Stockage des notations des catégories
    ScreenSelector *screenSelector; // Sélecteur d'écran

    // Contrôles de paramètres pour la sauvegarde/chargement
    QComboBox *frequencyCombo;
    QSpinBox *customValueSpinBox;
    QComboBox *customUnitCombo;
    QComboBox *adjustmentCombo;
    ToggleSwitch *startupToggle;
    ToggleSwitch *multiScreenToggle;
    QPushButton *applyButton;
    bool isLoadingSettings;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ModernWindow window;
    window.show();
    
    return app.exec();
}

#include "main.moc"