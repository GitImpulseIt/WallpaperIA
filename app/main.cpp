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
#include <QPainter>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QSettings>
#include <QPainterPath>
#include <QCryptographicHash>
#include <QDateTime>
#include <QStandardItemModel>
#include <QRegularExpression>
#include <string>
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

// Fonction utilitaire globale pour obtenir le chemin absolu vers les images
QString getImagePath(const QString &imageName) {
    return QCoreApplication::applicationDirPath() + "/" + imageName;
}

// Définitions pour IDesktopWallpaper si non disponibles dans MinGW
#ifndef __IDesktopWallpaper_INTERFACE_DEFINED__
#define __IDesktopWallpaper_INTERFACE_DEFINED__

DEFINE_GUID(CLSID_DesktopWallpaper, 0xC2CF3110, 0x460E, 0x4fc1, 0xB9, 0xD0, 0x8A, 0x1C, 0x0C, 0x9C, 0xC4, 0xBD);

// Énumérations pour les positions de fond d'écran
typedef enum DESKTOP_WALLPAPER_POSITION {
    DWPOS_CENTER = 0,
    DWPOS_TILE = 1,
    DWPOS_STRETCH = 2,
    DWPOS_FIT = 3,
    DWPOS_FILL = 4,
    DWPOS_SPAN = 5
} DESKTOP_WALLPAPER_POSITION;

interface IDesktopWallpaper : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetWallpaper(LPCWSTR monitorID, LPCWSTR wallpaper) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetWallpaper(LPCWSTR monitorID, LPWSTR *wallpaper) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMonitorDevicePathAt(UINT monitorIndex, LPWSTR *monitorID) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMonitorDevicePathCount(UINT *count) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMonitorRECT(LPCWSTR monitorID, RECT *displayRect) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBackgroundColor(COLORREF color) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBackgroundColor(COLORREF *color) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetPosition(DESKTOP_WALLPAPER_POSITION position) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPosition(DESKTOP_WALLPAPER_POSITION *position) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSlideshow(IShellItemArray *items) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSlideshow(IShellItemArray **items) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSlideshowOptions(DESKTOP_SLIDESHOW_OPTIONS options, UINT slideshowTick) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSlideshowOptions(DESKTOP_SLIDESHOW_OPTIONS *options, UINT *slideshowTick) = 0;
    virtual HRESULT STDMETHODCALLTYPE AdvanceSlideshow(LPCWSTR monitorID, DESKTOP_SLIDESHOW_DIRECTION direction) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetStatus(DESKTOP_SLIDESHOW_STATE *state) = 0;
    virtual HRESULT STDMETHODCALLTYPE Enable(BOOL enable) = 0;
};

// Énumérations supplémentaires pour le slideshow
typedef enum DESKTOP_SLIDESHOW_OPTIONS {
    DSO_SHUFFLEIMAGES = 0x01
} DESKTOP_SLIDESHOW_OPTIONS;

typedef enum DESKTOP_SLIDESHOW_STATE {
    DSS_ENABLED = 0x01,
    DSS_SLIDESHOW = 0x02,
    DSS_DISABLED_BY_REMOTE_SESSION = 0x04
} DESKTOP_SLIDESHOW_STATE;

typedef enum DESKTOP_SLIDESHOW_DIRECTION {
    DSD_FORWARD = 0,
    DSD_BACKWARD = 1
} DESKTOP_SLIDESHOW_DIRECTION;

static const IID IID_IDesktopWallpaper = {0xB92B56A9, 0x8B55, 0x4E14, {0x9A, 0x89, 0x01, 0x99, 0xBB, 0xB6, 0xF9, 0x3B}};

#endif // __IDesktopWallpaper_INTERFACE_DEFINED__

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
                    QPixmap starPixmap(getImagePath("star_inactive.png"));
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

    QPushButton *m_disableBtn;
    QString m_categoryId;
    QWidget *m_parent;
};

// Classe pour le sélecteur d'écran avec esthétique moderne
class ScreenSelector : public QWidget
{
    Q_OBJECT

public:
    ScreenSelector(QWidget *parent = nullptr) : QWidget(parent), m_screenCount(0)
    {
        setFixedHeight(35);
        setupScreens();
        setupUI();
        // Sélectionner tous les écrans par défaut
        for (int i = 0; i < m_screenCount; i++) {
            m_selectedScreens[i] = true;
        }
    }

    void setScreenSelected(int screenIndex, bool selected)
    {
        if (screenIndex >= 0 && screenIndex < m_screenCount) {
            m_selectedScreens[screenIndex] = selected;
            update();
            emit screenSelectionChanged(getSelectedScreens());
        }
    }

    bool isScreenSelected(int screenIndex) const
    {
        return m_selectedScreens.value(screenIndex, false);
    }

    QList<int> getSelectedScreens() const
    {
        QList<int> selected;
        for (auto it = m_selectedScreens.constBegin(); it != m_selectedScreens.constEnd(); ++it) {
            if (it.value()) {
                selected.append(it.key());
            }
        }
        return selected;
    }

    void setScreenCanBeDeselected(int screenIndex, bool canDeselect)
    {
        m_canBeDeselected[screenIndex] = canDeselect;
        update();
    }

    bool canScreenBeDeselected(int screenIndex) const
    {
        return m_canBeDeselected.value(screenIndex, false);
    }

    int screenCount() const { return m_screenCount; }

signals:
    void screenSelectionChanged(const QList<int> &selectedScreens);
    void screenDeselectionBlocked(int screenIndex);

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

        // Puis dessiner les onglets sélectionnés par-dessus avec clipping pour respecter les coins arrondis
        painter.save();

        // Créer un chemin de clipping avec les coins arrondis
        QPainterPath clipPath;
        clipPath.addRoundedRect(globalRect, borderRadius, borderRadius);
        painter.setClipPath(clipPath);

        for (int i = 0; i < m_screenCount; i++) {
            QRect tabRect(i * tabWidth, 0, tabWidth, height());

            // Dessiner les onglets sélectionnés
            if (isScreenSelected(i)) {
                painter.setBrush(QColor("#d14836")); // Rouge-orange pour sélectionnés
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
        painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
        for (int i = 0; i < m_screenCount; i++) {
            QRect tabRect(i * tabWidth, 0, tabWidth, height());
            QString text = QString("Écran %1").arg(i + 1);

            // Couleur du texte selon l'état
            if (isScreenSelected(i)) {
                painter.setPen(QColor("#ffffff")); // Blanc pour sélectionnés
            } else {
                painter.setPen(QColor("#cccccc")); // Gris pour non-sélectionnés
            }

            painter.drawText(tabRect, Qt::AlignCenter, text);

            // Dessiner un petit indicateur si l'écran ne peut pas être désélectionné
            if (!canScreenBeDeselected(i) && isScreenSelected(i)) {
                painter.setPen(QColor("#ffffff"));
                painter.setFont(QFont("Segoe UI", 7));
                QRect lockRect = tabRect.adjusted(2, tabRect.height() - 12, -2, -2);
                painter.drawText(lockRect, Qt::AlignRight, "🔒");
            }
        }
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            int tabWidth = width() / m_screenCount;
            int clickedScreen = static_cast<int>(event->position().x()) / tabWidth;
            if (clickedScreen >= 0 && clickedScreen < m_screenCount) {
                bool currentlySelected = isScreenSelected(clickedScreen);

                if (currentlySelected) {
                    // Tentative de désélection
                    if (canScreenBeDeselected(clickedScreen)) {
                        setScreenSelected(clickedScreen, false);
                    } else {
                        // Émission du signal de blocage pour afficher le message
                        emit screenDeselectionBlocked(clickedScreen);
                    }
                } else {
                    // Sélection
                    setScreenSelected(clickedScreen, true);
                }
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
    int m_screenCount;
    QMap<int, bool> m_selectedScreens;      // Écrans sélectionnés
    QMap<int, bool> m_canBeDeselected;      // Écrans qui peuvent être désélectionnés
};

// Classe pour afficher un compte à rebours avec camembert de progression
class CountdownWidget : public QWidget
{
    Q_OBJECT

signals:
    void countdownExpired(); // Signal émis quand le compte à rebours expire

public:
    CountdownWidget(QWidget *parent = nullptr) : QWidget(parent), m_totalSeconds(3600), m_remainingSeconds(3600), m_isStartupMode(false), m_isNeverMode(false)
    {
        setFixedSize(280, 200); // Élargi pour permettre des cadres plus larges

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
        m_isNeverMode = (seconds == -1); // Mode "Jamais"
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

        if (m_isNeverMode) {
            // Mode "Changement manuel uniquement" : affichage dans un encadré informatif
            QRect infoRect = rect().adjusted(0, 15, 0, -120); // Cadre élargi

            // Fond de l'encadré avec bordure arrondie
            painter.setBrush(QColor(33, 150, 243, 30)); // #2196F3 avec transparence
            painter.setPen(QPen(QColor("#2196F3"), 2));
            painter.drawRoundedRect(infoRect, 8, 8);

            // Icône info centrée verticalement à gauche
            QPixmap manualIcon(getImagePath("info.png")); // Même icône que le redémarrage
            int iconSize = 32;
            int iconY = infoRect.center().y() - iconSize/2; // Centré verticalement
            QRect iconRect(infoRect.left() + 15, iconY, iconSize, iconSize);

            if (!manualIcon.isNull()) {
                painter.drawPixmap(iconRect, manualIcon);
            } else {
                // Fallback si l'image ne se charge pas
                painter.setPen(QColor("#2196F3"));
                painter.setFont(QFont("Segoe UI", 16));
                painter.drawText(iconRect, Qt::AlignCenter, "🔄");
            }

            // Texte complet à droite de l'icône
            painter.setPen(QColor("#ffffff"));
            painter.setFont(QFont("Segoe UI", 10)); // Pas de gras
            QRect textRect(infoRect.left() + 60, infoRect.top() + 15, infoRect.width() - 65, infoRect.height() - 30);
            painter.drawText(textRect, Qt::AlignLeft | Qt::TextWordWrap,
                "Changement de fond d'écran manuel seulement");
            return;
        }

        if (m_isStartupMode) {
            // Mode "Au démarrage" : affichage dans un encadré d'information élargi
            QRect infoRect = rect().adjusted(0, 15, 0, -100); // Cadre élargi

            // Fond de l'encadré avec bordure arrondie
            painter.setBrush(QColor(33, 150, 243, 30)); // #2196F3 avec transparence
            painter.setPen(QPen(QColor("#2196F3"), 2));
            painter.drawRoundedRect(infoRect, 8, 8);

            // Icône reboot centrée verticalement à gauche
            QPixmap rebootIcon(getImagePath("info.png")); // Chemin depuis l'exécutable
            int iconSize = 32;
            int iconY = infoRect.center().y() - iconSize/2; // Centré verticalement
            QRect iconRect(infoRect.left() + 15, iconY, iconSize, iconSize);

            if (!rebootIcon.isNull()) {
                painter.drawPixmap(iconRect, rebootIcon);
            } else {
                // Fallback si l'image ne se charge pas
                painter.setPen(QColor("#2196F3"));
                painter.setFont(QFont("Segoe UI", 16));
                painter.drawText(iconRect, Qt::AlignCenter, "🔄");
            }

            // Texte complet à droite de l'icône
            painter.setPen(QColor("#ffffff"));
            painter.setFont(QFont("Segoe UI", 10)); // Pas de gras
            QRect textRect(infoRect.left() + 60, infoRect.top() + 15, infoRect.width() - 65, infoRect.height() - 30);
            painter.drawText(textRect, Qt::AlignLeft | Qt::TextWordWrap,
                "Changement de fond d'écran au prochain redémarrage de l'ordinateur");
            return;
        }

        // Centrer le cercle dans le widget élargi
        int circleSize = 160; // Taille du cercle
        int centerX = rect().center().x();
        int centerY = rect().center().y();
        QRect circleRect(centerX - circleSize/2, centerY - circleSize/2, circleSize, circleSize);

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
        if (m_isStartupMode || m_isNeverMode) {
            // Pas de countdown en mode démarrage ou jamais
            return;
        }

        if (m_remainingSeconds > 0) {
            m_remainingSeconds--;
            update();
        } else {
            // Temps écoulé, émettre le signal puis réinitialiser
            emit countdownExpired();
            m_remainingSeconds = m_totalSeconds;
            update();
        }
    }

private:
    int m_totalSeconds;
    int m_remainingSeconds;
    bool m_isStartupMode;
    bool m_isNeverMode;
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

// Fonctions utilitaires pour la gestion du démarrage Windows
#ifdef Q_OS_WIN
bool addToWindowsStartup(const QString &appName, const QString &appPath) {
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
                               L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                               0, KEY_SET_VALUE, &hKey);

    if (result != ERROR_SUCCESS) {
        return false;
    }

    std::wstring wAppName = appName.toStdWString();
    std::wstring wAppPath = appPath.toStdWString();

    result = RegSetValueExW(hKey, wAppName.c_str(), 0, REG_SZ,
                           reinterpret_cast<const BYTE*>(wAppPath.c_str()),
                           (wAppPath.length() + 1) * sizeof(wchar_t));

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool removeFromWindowsStartup(const QString &appName) {
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
                               L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                               0, KEY_SET_VALUE, &hKey);

    if (result != ERROR_SUCCESS) {
        return false;
    }

    std::wstring wAppName = appName.toStdWString();
    result = RegDeleteValueW(hKey, wAppName.c_str());

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool isInWindowsStartup(const QString &appName) {
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
                               L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                               0, KEY_QUERY_VALUE, &hKey);

    if (result != ERROR_SUCCESS) {
        return false;
    }

    std::wstring wAppName = appName.toStdWString();
    DWORD dataType;
    DWORD dataSize = 0;

    result = RegQueryValueExW(hKey, wAppName.c_str(), nullptr, &dataType, nullptr, &dataSize);

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}
#endif

class ModernWindow : public QWidget
{
    Q_OBJECT

public:

    ModernWindow(QWidget *parent = nullptr) : QWidget(parent), networkManager(new QNetworkAccessManager(this)), isLoadingSettings(false)
    {
        setWindowTitle("WallpaperIA - Gestionnaire de Fonds d'écran");
        setWindowIcon(QIcon(getImagePath("icon.png")));
        setFixedSize(725, 650);

        // Configuration des boutons de la fenêtre : minimiser et fermer (pas d'agrandissement)
        setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

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
        // on ne peut pas utiliser "Au démarrage de l'ordinateur"
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

        if (launchedAtStartup && frequencyCombo && frequencyCombo->currentIndex() == 7) {
            // Index 7 = "Au démarrage de l'ordinateur"
            // Déclencher le changement de fond d'écran
            QTimer::singleShot(2000, [this]() {
                currentTriggerMode = "Démarrage";
                onChangeNowClicked();
            }); // Délai de 2s pour laisser le système démarrer
        }
    }

    void applyStartupSetting(bool enabled) {
        #ifdef Q_OS_WIN
        QString appName = "WallpaperIA";
        QString appPath = QCoreApplication::applicationFilePath();

        if (enabled) {
            // Ajouter l'argument --startup pour démarrer dans le tray
            // Convertir le chemin en format Windows avec backslashes
            QString windowsPath = QDir::toNativeSeparators(appPath);
            QString startupCommand = QString("\"%1\" --startup").arg(windowsPath);
            if (!addToWindowsStartup(appName, startupCommand)) {
                // En cas d'erreur, désactiver le toggle
                startupToggle->setChecked(false);
            }
        } else {
            removeFromWindowsStartup(appName);
        }
        #endif
    }

    bool checkStartupStatus() {
        #ifdef Q_OS_WIN
        return isInWindowsStartup("WallpaperIA");
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
            if (index == 8) { // Index de "Autre"
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
        adjustmentLayout->setSpacing(10);
        adjustmentLayout->setContentsMargins(10, 15, 10, 15);

        // Layout horizontal principal pour diviser en deux parties
        QHBoxLayout *mainAdjustmentLayout = new QHBoxLayout();
        mainAdjustmentLayout->setSpacing(10);
        mainAdjustmentLayout->setAlignment(Qt::AlignTop);

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

        // Ajouter le sélecteur à la partie gauche
        mainAdjustmentLayout->addWidget(selectorWidget);

        // Encadré d'explication (partie droite)
        QWidget *explanationWidget = new QWidget();
        explanationWidget->setFixedHeight(167); // Hauteur exacte: 32px (combo) + 15px (spacing) + 120px (image)
        explanationWidget->setStyleSheet(
            "QWidget {"
            "background-color: rgba(33, 150, 243, 30);" // #2196F3 avec transparence
            "border: 2px solid #2196F3;"
            "border-radius: 8px;"
            "}"
        );

        QVBoxLayout *explanationLayout = new QVBoxLayout(explanationWidget);
        explanationLayout->setContentsMargins(10, 10, 10, 10);

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
            "padding: 5px;"
            "}"
        );

        // Texte par défaut pour "Remplir"
        adjustmentExplanationLabel->setText("L'image conserve son ratio mais dépasse de l'écran pour le remplir");

        explanationLayout->addWidget(adjustmentExplanationLabel);
        explanationLayout->addStretch();

        // Ajouter l'encadré d'explication à la partie droite
        mainAdjustmentLayout->addWidget(explanationWidget);

        // Ajouter le layout principal au groupe
        adjustmentLayout->addLayout(mainAdjustmentLayout);

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
        connect(startupToggle, &ToggleSwitch::toggled, this, &ModernWindow::onStartupToggled);
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
        trayIcon->setIcon(QIcon(getImagePath("icon.png")));
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
    void onScreenSelectionChanged(const QList<int> &selectedScreens)
    {
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
                statusLabel->setText("Aucun écran sélectionné");
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
        // Par défaut, mode manuel si pas défini
        if (currentTriggerMode.isEmpty()) {
            currentTriggerMode = "Manuel";
        }

        // Désactiver le bouton pendant le processus
        changeNowButton->setEnabled(false);
        changeNowButton->setText("🔄 Changement en cours...");

        // Déterminer combien d'images sont nécessaires
        QList<int> targetScreens;
        if (multiScreenToggle->isChecked() && screenSelector && screenSelector->screenCount() > 1) {
            targetScreens = screenSelector->getSelectedScreens();
        } else {
            targetScreens.append(-1); // Mode classique : une seule image pour tous
        }

        if (targetScreens.contains(-1) || targetScreens.size() == 1) {
            // Une seule image nécessaire
            statusLabel->setText("Récupération d'une image aléatoire...");
            getRandomWallpaper();
        } else {
            // Plusieurs images nécessaires
            statusLabel->setText(QString("Récupération de %1 images aléatoires...").arg(targetScreens.size()));
            getMultipleRandomWallpapers(targetScreens);
        }
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
        QNetworkRequest request(QUrl("http://localhost:8080/WallpaperIA/api/categories"));
        QNetworkReply *reply = networkManager->get(request);

        connect(reply, &QNetworkReply::finished, [this, reply, screenIndex]() {
            // Vérifier que le téléchargement multi-écrans est toujours actif
            if (!currentMultiDownload) {
                reply->deleteLater();
                return;
            }

            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonObject obj = doc.object();

                if (obj["success"].toBool()) {
                    QJsonArray categories = obj["data"].toArray();
                    if (!categories.isEmpty()) {
                        // Choisir une catégorie au hasard
                        int randomCategoryIndex = QRandomGenerator::global()->bounded(categories.size());
                        QString categoryId = categories[randomCategoryIndex].toObject()["id"].toString();

                        // Obtenir une image de cette catégorie pour cet écran
                        getRandomImageFromCategoryForScreen(categoryId, screenIndex);
                    } else {
                        handleMultiDownloadError(QString("Aucune catégorie disponible pour écran %1").arg(screenIndex + 1));
                    }
                } else {
                    handleMultiDownloadError(QString("Erreur API catégories pour écran %1: %2").arg(screenIndex + 1).arg(obj["message"].toString()));
                }
            } else {
                handleMultiDownloadError(QString("Erreur de connexion catégories pour écran %1: %2").arg(screenIndex + 1).arg(reply->errorString()));
            }
            reply->deleteLater();
        });
    }

    void getRandomImageFromCategoryForScreen(const QString &categoryId, int screenIndex)
    {
        QNetworkRequest request(QUrl(QString("http://localhost:8080/WallpaperIA/api/wallpapers?category=%1").arg(categoryId)));
        QNetworkReply *reply = networkManager->get(request);

        connect(reply, &QNetworkReply::finished, [this, reply, screenIndex]() {
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
                        // Choisir une image au hasard
                        int randomIndex = QRandomGenerator::global()->bounded(wallpapers.size());
                        QJsonObject wallpaper = wallpapers[randomIndex].toObject();
                        QString filename = wallpaper["filename"].toString();

                        // Vérifier que le filename n'est pas vide avant le téléchargement
                        if (filename.isEmpty()) {
                            handleMultiDownloadError(QString("Filename d'image vide pour écran %1").arg(screenIndex + 1));
                        } else {
                            // Construire l'URL complète pour le téléchargement
                            QString imageUrl = QString("http://localhost:8080/WallpaperIA/api/get/%1").arg(filename);

                            // Télécharger cette image pour cet écran
                            downloadImageForScreen(imageUrl, screenIndex);
                        }
                    } else {
                        handleMultiDownloadError(QString("Aucune image disponible pour écran %1").arg(screenIndex + 1));
                    }
                } else {
                    handleMultiDownloadError(QString("Erreur API wallpapers pour écran %1: %2").arg(screenIndex + 1).arg(obj["message"].toString()));
                }
            } else {
                handleMultiDownloadError(QString("Erreur de connexion wallpapers pour écran %1: %2").arg(screenIndex + 1).arg(reply->errorString()));
            }
            reply->deleteLater();
        });
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

        connect(reply, &QNetworkReply::finished, [this, reply, screenIndex, imageUrl]() {
            // Vérifier que le téléchargement multi-écrans est toujours actif
            if (!currentMultiDownload) {
                reply->deleteLater();
                return;
            }

            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();

                // Sauvegarder l'image temporairement
                QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperIA";
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

        // Créer une image composite avec les différentes images pour chaque écran
        if (setMultipleWallpapers(currentMultiDownload->downloadedImages)) {
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
        QString detailedError = QString("Erreur multi-écrans: %1").arg(error);
        if (currentMultiDownload) {
            detailedError += QString("\n\nProgression: %1/%2 écrans complétés")
                            .arg(currentMultiDownload->completedDownloads)
                            .arg(currentMultiDownload->pendingDownloads);

            // Lister les écrans ciblés
            QStringList screenNumbers;
            for (int screen : currentMultiDownload->targetScreens) {
                screenNumbers.append(QString::number(screen + 1));
            }
            detailedError += QString("\nÉcrans ciblés: %1").arg(screenNumbers.join(", "));

            delete currentMultiDownload;
            currentMultiDownload = nullptr;
        }

        // Afficher une boîte de dialogue avec le message complet
        QMessageBox errorBox(this);
        errorBox.setWindowTitle("Erreur de téléchargement");
        errorBox.setText("Une erreur s'est produite lors du téléchargement des fonds d'écran.");
        errorBox.setDetailedText(detailedError);
        errorBox.setIcon(QMessageBox::Warning);
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.setStyleSheet(
            "QMessageBox {"
            "background-color: #2b2b2b;"
            "color: #ffffff;"
            "}"
            "QMessageBox QPushButton {"
            "background-color: #d14836;"
            "color: white;"
            "border: none;"
            "border-radius: 4px;"
            "padding: 8px 16px;"
            "min-width: 80px;"
            "}"
            "QMessageBox QPushButton:hover {"
            "background-color: #b8392a;"
            "}"
            "QMessageBox QTextEdit {"
            "background-color: #3b3b3b;"
            "color: #ffffff;"
            "border: 1px solid #555555;"
            "}"
        );
        errorBox.exec();

        // Aussi mettre un message court dans le statut
        restoreButton("Erreur de téléchargement - Voir détails");
    }

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

                // Sauvegarder le fichier original
                QString originalFilePath = tempDir + "/" + filename;
                QFile originalFile(originalFilePath);
                if (!originalFile.exists() || originalFile.size() != imageData.size()) {
                    if (originalFile.open(QIODevice::WriteOnly)) {
                        originalFile.write(imageData);
                        originalFile.close();
                    }
                }

                // Convertir l'image en BMP comme Firefox (plus fiable pour Windows)
                QString tempFilePath = originalFilePath; // Par défaut, utiliser l'original
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    // Sauvegarder en BMP pour une compatibilité maximale avec Windows
                    QString bmpPath = tempDir + "/" + QFileInfo(filename).baseName() + ".bmp";
                    pixmap.save(bmpPath, "BMP");
                    tempFilePath = bmpPath; // Pour Windows, utiliser le BMP
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

                // Déterminer quels écrans utiliser
                QList<int> targetScreens;

                // Si le multi-écran est activé et qu'il y a plusieurs écrans, utiliser les écrans sélectionnés
                if (multiScreenToggle->isChecked() && screenSelector && screenSelector->screenCount() > 1) {
                    targetScreens = screenSelector->getSelectedScreens();
                } else {
                    // Mode classique : tous les écrans
                    targetScreens.append(-1); // -1 = tous les écrans
                }

                // Appliquer le fond d'écran
                if (setWindowsWallpaperMultiScreen(tempFilePath, targetScreens, filename)) {
                    // Mettre à jour l'historique pour chaque écran concerné (utiliser le fichier original)
                    if (targetScreens.contains(-1)) {
                        // Tous les écrans
                        for (int i = 0; i < screenSelector->screenCount(); i++) {
                            addToScreenHistory(i, originalFilePath, currentTriggerMode.isEmpty() ? "Manuel" : currentTriggerMode);
                        }
                        statusLabel->setText(QString("Fond d'écran: %1").arg(filename));
                    } else {
                        // Écrans spécifiques
                        for (int screen : targetScreens) {
                            addToScreenHistory(screen, originalFilePath, currentTriggerMode.isEmpty() ? "Manuel" : currentTriggerMode);
                        }
                        if (targetScreens.size() == 1) {
                            statusLabel->setText(QString("Fond d'écran écran %1: %2").arg(targetScreens.first() + 1).arg(filename));
                        } else {
                            QStringList screenNumbers;
                            for (int screen : targetScreens) {
                                screenNumbers.append(QString::number(screen + 1));
                            }
                            statusLabel->setText(QString("Fond d'écran écrans %1: %2").arg(screenNumbers.join(", ")).arg(filename));
                        }
                    }

                    // Nettoyer les fichiers temporaires après application réussie
                    cleanupTemporaryFiles();

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

    bool setWindowsWallpaper(const QString &imagePath, int screenIndex = -1)
    {
        #ifdef Q_OS_WIN
        QFileInfo fileInfo(imagePath);
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            return false;
        }

        // Si screenIndex est -1 ou qu'il n'y a qu'un écran, utiliser la méthode traditionnelle
        if (screenIndex == -1 || QApplication::screens().count() <= 1) {
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
        }

        // Pour un écran spécifique, utiliser IDesktopWallpaper (Windows 8+)
        return setWallpaperForSpecificScreen(imagePath, screenIndex);
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
        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperIA";
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
            SPIF_UPDATEINIFILE | SPIF_SENDCHANGE
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
        // Utiliser le nouveau système de mapping sophistiqué

        // 1. Calculer les dimensions du bureau virtuel
        QRect virtualDesktop = calculateVirtualDesktopBounds();

        // 2. Générer les mappings pour chaque écran
        QList<ScreenMapping> mappings = generateScreenMappings(imagePaths);

        if (mappings.isEmpty()) {
            return false;
        }

        // 3. Créer l'image composite avec mapping précis
        QPixmap composite = createCompositeImageFromMappings(mappings, virtualDesktop);

        // 4. Appliquer le wrapping selon la logique DMT (Windows < 8 uniquement)
        QPixmap finalImage = wrapCoordinatesForWindows(composite, virtualDesktop);

        // 5. Sauvegarder et appliquer
        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperIA";
        QDir().mkpath(tempDir);
        QString compositePath = tempDir + "/composite_wallpaper.bmp";

        if (finalImage.save(compositePath, "BMP")) {
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

    // Structure de mapping sophistiquée inspirée de DualMonitorTools
    struct ScreenMapping {
        QRect screenRect;           // Rectangle de l'écran dans le bureau virtuel
        QRect sourceRect;           // Rectangle dans l'image source à utiliser
        QRect destRect;             // Rectangle de destination dans l'image composite
        QString imagePath;          // Chemin vers l'image source
        bool isPrimary;             // Écran principal

        ScreenMapping() : isPrimary(false) {}
    };

    QRect calculateVirtualDesktopBounds()
    {
        QList<QScreen*> screens = QApplication::screens();
        if (screens.isEmpty()) return QRect();

        // Calculer avec résolutions natives comme dans screenmap.cpp
        int minX = 0, minY = 0, maxX = 0, maxY = 0;

        QScreen* qtPrimaryScreen = QApplication::primaryScreen();
        qDebug() << "=== DEBUG VIRTUAL DESKTOP (NATIVES) ===";
        qDebug() << "Nombre d'écrans:" << screens.size();
        qDebug() << "Écran principal détecté par Qt:" << (qtPrimaryScreen ? qtPrimaryScreen->name() : "AUCUN");

        for (int i = 0; i < screens.size(); i++) {
            QScreen* screen = screens[i];
            QRect geom = screen->geometry();
            QSize realSize = screen->size() * screen->devicePixelRatio();
            bool isQtPrimary = (screen == qtPrimaryScreen);

            // Coordonnées : garder telles quelles (pas de scaling)
            int realX = geom.x();
            int realY = geom.y();

            // Tailles : utiliser résolutions réelles
            int realW = realSize.width();
            int realH = realSize.height();

            if (realX < minX) minX = realX;
            if (realY < minY) minY = realY;
            if (realX + realW > maxX) maxX = realX + realW;
            if (realY + realH > maxY) maxY = realY + realH;

            qDebug() << QString("Écran %1: geom=%2x%3 à (%4,%5) real=%6x%7 - Qt Primary: %8 - Name: %9")
                        .arg(i)
                        .arg(geom.width()).arg(geom.height())
                        .arg(geom.x()).arg(geom.y())
                        .arg(realW).arg(realH)
                        .arg(isQtPrimary ? "YES" : "NO")
                        .arg(screen->name());
        }

        QRect bounds(minX, minY, maxX - minX, maxY - minY);
        qDebug() << QString("Bureau virtuel avec résolutions natives: %1x%2 à (%3,%4)")
                    .arg(bounds.width())
                    .arg(bounds.height())
                    .arg(bounds.x())
                    .arg(bounds.y());

        // Vérification si wrapping sera nécessaire
        bool needsWrap = (bounds.left() < 0 || bounds.top() < 0);
        qDebug() << QString("Wrapping nécessaire: %1 (left=%2, top=%3)")
                    .arg(needsWrap ? "OUI" : "NON")
                    .arg(bounds.left())
                    .arg(bounds.top());
        qDebug() << "=========================================";

        return bounds;
    }

    QList<ScreenMapping> generateScreenMappings(const QMap<int, QString> &imagePaths)
    {
        QList<ScreenMapping> mappings;
        QList<QScreen*> screens = QApplication::screens();

        if (screens.isEmpty()) {
            return mappings;
        }

        // Calculer les dimensions totales avec résolution réelle comme dans screenmap.cpp
        int minX = 0, minY = 0, maxX = 0, maxY = 0;

        for (QScreen* screen : screens) {
            QRect geom = screen->geometry();
            QSize realSize = screen->size() * screen->devicePixelRatio();

            // Coordonnées : garder telles quelles (pas de scaling)
            int realX = geom.x();
            int realY = geom.y();

            // Tailles : utiliser résolutions réelles
            int realW = realSize.width();
            int realH = realSize.height();

            if (realX < minX) minX = realX;
            if (realY < minY) minY = realY;
            if (realX + realW > maxX) maxX = realX + realW;
            if (realY + realH > maxY) maxY = realY + realH;
        }

        qDebug() << "=== GENERATESCREENMAPPINGS DEBUG ===";
        qDebug() << QString("Zone totale avec résolutions natives: %1x%2 offset=(%3,%4)")
                    .arg(maxX - minX).arg(maxY - minY).arg(-minX).arg(-minY);

        // Créer les mappings pour chaque écran avec ses images
        for (auto it = imagePaths.constBegin(); it != imagePaths.constEnd(); ++it) {
            int screenIndex = it.key();
            QString imagePath = it.value();

            if (screenIndex >= 0 && screenIndex < screens.size()) {
                QScreen* screen = screens[screenIndex];
                QRect geom = screen->geometry();
                QSize realSize = screen->size() * screen->devicePixelRatio();

                ScreenMapping mapping;
                mapping.screenRect = QRect(geom.x(), geom.y(), realSize.width(), realSize.height());
                mapping.imagePath = imagePath;
                mapping.isPrimary = (screen == QApplication::primaryScreen());
                mapping.destRect = QRect(geom.x() - minX, geom.y() - minY, realSize.width(), realSize.height());

                // Pour l'image source, utiliser toute l'image
                QPixmap sourceImage(imagePath);
                if (!sourceImage.isNull()) {
                    mapping.sourceRect = QRect(0, 0, sourceImage.width(), sourceImage.height());
                } else {
                    mapping.sourceRect = QRect(0, 0, realSize.width(), realSize.height());
                }

                mappings.append(mapping);

                qDebug() << QString("Écran %1: geom=%2x%3 à (%4,%5) real=%6x%7 dest=(%8,%9)")
                            .arg(screenIndex)
                            .arg(geom.width()).arg(geom.height())
                            .arg(geom.x()).arg(geom.y())
                            .arg(realSize.width()).arg(realSize.height())
                            .arg(mapping.destRect.x()).arg(mapping.destRect.y());
            }
        }

        qDebug() << QString("Mappings générés: %1").arg(mappings.size());
        qDebug() << "====================================";

        return mappings;
    }

    QPixmap applyAdjustmentMode(const QPixmap &sourceImage, const QSize &targetSize, int adjustmentMode)
    {
        if (sourceImage.isNull()) {
            return QPixmap();
        }

        QPixmap result(targetSize);
        result.fill(Qt::black);
        QPainter painter(&result);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.setRenderHint(QPainter::Antialiasing);

        switch (adjustmentMode) {
            case 0: // Remplir (Cover) - conserve le ratio, peut dépasser
            {
                QPixmap scaledImage = sourceImage.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                // Centrer l'image si elle dépasse
                int x = (targetSize.width() - scaledImage.width()) / 2;
                int y = (targetSize.height() - scaledImage.height()) / 2;
                painter.drawPixmap(x, y, scaledImage);
                break;
            }
            case 1: // Ajuster (Contain) - conserve le ratio, bandes noires
            {
                QPixmap scaledImage = sourceImage.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                // Centrer l'image dans l'espace disponible
                int x = (targetSize.width() - scaledImage.width()) / 2;
                int y = (targetSize.height() - scaledImage.height()) / 2;
                painter.drawPixmap(x, y, scaledImage);
                break;
            }
            case 2: // Étendre - ce mode ne devrait pas être utilisé en multi-écran
            case 3: // Étirer - déforme pour remplir exactement
            {
                QPixmap scaledImage = sourceImage.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                painter.drawPixmap(0, 0, scaledImage);
                break;
            }
            case 4: // Mosaïque - répète l'image
            {
                for (int x = 0; x < targetSize.width(); x += sourceImage.width()) {
                    for (int y = 0; y < targetSize.height(); y += sourceImage.height()) {
                        painter.drawPixmap(x, y, sourceImage);
                    }
                }
                break;
            }
            default: // Par défaut, utiliser le mode remplir
            {
                QPixmap scaledImage = sourceImage.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                int x = (targetSize.width() - scaledImage.width()) / 2;
                int y = (targetSize.height() - scaledImage.height()) / 2;
                painter.drawPixmap(x, y, scaledImage);
                break;
            }
        }

        painter.end();
        return result;
    }

    QPixmap createCompositeImageFromMappings(const QList<ScreenMapping> &mappings, const QRect &virtualDesktop)
    {
        QPixmap composite(virtualDesktop.size());
        composite.fill(Qt::black);

        QPainter painter(&composite);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.setRenderHint(QPainter::Antialiasing);

        qDebug() << QString("=== CRÉATION COMPOSITE ===");
        qDebug() << QString("Taille composite: %1x%2").arg(composite.width()).arg(composite.height());

        // Récupérer le mode d'ajustement sélectionné par l'utilisateur
        int adjustmentMode = adjustmentCombo ? adjustmentCombo->currentIndex() : 0;
        qDebug() << QString("Mode d'ajustement appliqué: %1").arg(adjustmentMode);

        for (const ScreenMapping &mapping : mappings) {
            QPixmap sourceImage(mapping.imagePath);
            if (!sourceImage.isNull()) {
                qDebug() << QString("Traitement image pour écran - destRect: (%1,%2,%3,%4)")
                            .arg(mapping.destRect.x()).arg(mapping.destRect.y())
                            .arg(mapping.destRect.width()).arg(mapping.destRect.height());

                // Appliquer le mode d'ajustement choisi par l'utilisateur
                QPixmap adjustedImage = applyAdjustmentMode(sourceImage, mapping.destRect.size(), adjustmentMode);

                painter.drawPixmap(mapping.destRect, adjustedImage);
            }
        }

        painter.end();
        qDebug() << "=== FIN CRÉATION COMPOSITE ===";
        return composite;
    }

    bool isWindows8OrLater()
    {
        #ifdef Q_OS_WIN
        // Utiliser QSysInfo pour détecter la version Windows
        QOperatingSystemVersion current = QOperatingSystemVersion::current();

        // Windows 8 = version 6.2, Windows 7 = version 6.1
        // Windows 8 et plus récent supportent les coordonnées virtuelles directement
        return (current.majorVersion() > 6) ||
               (current.majorVersion() == 6 && current.minorVersion() >= 2);
        #else
        return true; // Non-Windows, assumer le support moderne
        #endif
    }

    bool needsCoordinateWrapping(const QRect &virtualDesktop)
    {
        // Logique DMT : wrapping nécessaire seulement si Windows < 8 ET coordonnées négatives
        bool hasNegativeCoords = (virtualDesktop.left() < 0 || virtualDesktop.top() < 0);
        bool isOldWindows = !isWindows8OrLater();

        qDebug() << QString("Version Windows >= 8: %1, Coords négatives: %2")
                    .arg(!isOldWindows).arg(hasNegativeCoords);

        return hasNegativeCoords && isOldWindows;
    }

    QPixmap wrapCoordinatesForWindows(const QPixmap &sourceImage, const QRect &virtualDesktop)
    {
        if (!needsCoordinateWrapping(virtualDesktop)) {
            return sourceImage; // Pas besoin de wrapping
        }

        qDebug() << "=== WRAPPING NÉCESSAIRE (Windows < 8) ===";
        qDebug() << QString("Offset virtuel: (%1,%2)").arg(virtualDesktop.left()).arg(virtualDesktop.top());
        qDebug() << QString("Taille image source: %1x%2").arg(sourceImage.width()).arg(sourceImage.height());

        // Créer une nouvelle image avec les coordonnées wrappées
        QPixmap wrappedImage(sourceImage.size());
        wrappedImage.fill(Qt::black);

        QPainter painter(&wrappedImage);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        int xWrap = -virtualDesktop.left();
        int yWrap = -virtualDesktop.top();
        int xNotWrap = sourceImage.width() - xWrap;
        int yNotWrap = sourceImage.height() - yWrap;

        qDebug() << QString("Paramètres wrapping - xWrap:%1, yWrap:%2, xNotWrap:%3, yNotWrap:%4")
                    .arg(xWrap).arg(yWrap).arg(xNotWrap).arg(yNotWrap);

        // Quadrant en haut à droite (a)
        if (xWrap > 0 && yWrap > 0) {
            QRect srcRect(0, 0, xWrap, yWrap);
            QRect destRect(xNotWrap, yNotWrap, xWrap, yWrap);
            painter.drawPixmap(destRect, sourceImage, srcRect);
            qDebug() << QString("Quadrant A: src(%1,%2,%3,%4) -> dest(%5,%6,%7,%8)")
                        .arg(srcRect.x()).arg(srcRect.y()).arg(srcRect.width()).arg(srcRect.height())
                        .arg(destRect.x()).arg(destRect.y()).arg(destRect.width()).arg(destRect.height());
        }

        // Quadrant en haut à gauche (b)
        if (yWrap > 0 && xNotWrap > 0) {
            QRect srcRect(xWrap, 0, xNotWrap, yWrap);
            QRect destRect(0, yNotWrap, xNotWrap, yWrap);
            painter.drawPixmap(destRect, sourceImage, srcRect);
            qDebug() << QString("Quadrant B: src(%1,%2,%3,%4) -> dest(%5,%6,%7,%8)")
                        .arg(srcRect.x()).arg(srcRect.y()).arg(srcRect.width()).arg(srcRect.height())
                        .arg(destRect.x()).arg(destRect.y()).arg(destRect.width()).arg(destRect.height());
        }

        // Quadrant en bas à droite (c)
        if (xWrap > 0 && yNotWrap > 0) {
            QRect srcRect(0, yWrap, xWrap, yNotWrap);
            QRect destRect(xNotWrap, 0, xWrap, yNotWrap);
            painter.drawPixmap(destRect, sourceImage, srcRect);
            qDebug() << QString("Quadrant C: src(%1,%2,%3,%4) -> dest(%5,%6,%7,%8)")
                        .arg(srcRect.x()).arg(srcRect.y()).arg(srcRect.width()).arg(srcRect.height())
                        .arg(destRect.x()).arg(destRect.y()).arg(destRect.width()).arg(destRect.height());
        }

        // Quadrant en bas à gauche (d) - écran principal
        if (xNotWrap > 0 && yNotWrap > 0) {
            QRect srcRect(xWrap, yWrap, xNotWrap, yNotWrap);
            QRect destRect(0, 0, xNotWrap, yNotWrap);
            painter.drawPixmap(destRect, sourceImage, srcRect);
            qDebug() << QString("Quadrant D (principal): src(%1,%2,%3,%4) -> dest(%5,%6,%7,%8)")
                        .arg(srcRect.x()).arg(srcRect.y()).arg(srcRect.width()).arg(srcRect.height())
                        .arg(destRect.x()).arg(destRect.y()).arg(destRect.width()).arg(destRect.height());
        }

        painter.end();
        qDebug() << "=== FIN WRAPPING ===";
        return wrappedImage;
    }


    QPixmap createCompositeWallpaperFromMultipleImages(const QMap<int, QString> &screenImages,
                                                       const QList<QScreen*> &screens, const QRect &virtualDesktop)
    {
        // Créer une image de la taille du bureau virtuel
        QPixmap composite(virtualDesktop.size());
        composite.fill(Qt::black);

        QPainter painter(&composite);

        for (int i = 0; i < screens.count(); ++i) {
            QScreen* screen = screens[i];
            QRect screenRect = screen->geometry();

            // Calculer la position relative dans l'image composite
            QRect relativeRect = screenRect.translated(-virtualDesktop.topLeft());

            QString imagePath;
            if (screenImages.contains(i)) {
                // Nouvelle image pour cet écran
                imagePath = screenImages[i];
            } else {
                // Utiliser l'historique pour les autres écrans
                imagePath = getLastWallpaperFromHistory(i);
            }

            if (!imagePath.isEmpty()) {
                QPixmap screenImage(imagePath);
                if (!screenImage.isNull()) {
                    QPixmap scaledImage = screenImage.scaled(relativeRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

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
                    // Image corrompue ou inaccessible, utiliser du noir
                    painter.fillRect(relativeRect, Qt::black);
                }
            } else {
                // Pas d'image, utiliser du noir
                painter.fillRect(relativeRect, Qt::black);
            }
        }

        painter.end();
        return composite;
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
        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperIA";
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
            SPIF_UPDATEINIFILE | SPIF_SENDCHANGE
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

        // Supprimer tous les fichiers sauf le fichier actuel
        for (const QFileInfo &fileInfo : files) {
            QString filePath = fileInfo.absoluteFilePath();

            // Ne pas supprimer le fichier actuel
            if (filePath != currentFilePath) {
                QFile::remove(filePath);
            }
        }
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
        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperIA";
        QDir dir(tempDir);

        if (!dir.exists()) {
            return;
        }

        // Lister tous les fichiers BMP et PNG temporaires
        QStringList filters;
        filters << "*.bmp" << "*.png";
        QFileInfoList tempFiles = dir.entryInfoList(filters, QDir::Files);

        // Supprimer tous les fichiers temporaires
        for (const QFileInfo &fileInfo : tempFiles) {
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }

    QString getLastWallpaperFromHistory(int screenIndex) const
    {
        // Récupérer uniquement depuis les logs
        return getCurrentWallpaperFromLog(screenIndex);
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

        // Sauvegarder l'historique des fonds d'écran
        saveHistoryToSettings();
    }

    void saveHistoryToSettings()
    {
        QSettings settings("WallpaperIA", "WallpaperSettings");

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

        // Charger l'historique des fonds d'écran
        loadHistoryFromSettings();

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
        QSettings settings("WallpaperIA", "WallpaperSettings");

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
    QLabel *adjustmentExplanationLabel; // Label pour l'explication du mode d'ajustement
    QMap<QString, int> categoryRatings; // Stockage des notations des catégories
    ScreenSelector *screenSelector; // Sélecteur d'écran

    // Historique des fonds d'écran par écran (max 8 images)
    QMap<int, QStringList> screenWallpaperHistory;
    static const int MAX_HISTORY_SIZE = 8;
    static const qint64 MAX_HISTORY_DIR_SIZE = 3 * 1024 * 1024; // 3 Mo

    QString getHistoryDirectory() const
    {
        QString historyDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/history";
        QDir().mkpath(historyDir);
        return historyDir;
    }

    QString getWallpapersDirectory() const
    {
        QString wallpapersDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/wallpapers";
        QDir().mkpath(wallpapersDir);
        return wallpapersDir;
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
                    QString screenId = getScreenUniqueId(screenIndex);
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
            QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperIA";
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