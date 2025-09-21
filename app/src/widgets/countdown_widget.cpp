#include "countdown_widget.h"
#include "../utils/utils.h"

CountdownWidget::CountdownWidget(QWidget *parent) : QWidget(parent), m_totalSeconds(3600), m_remainingSeconds(3600), m_isStartupMode(false), m_isNeverMode(false), m_isPaused(false)
{
    setFixedSize(280, 200); // Élargi pour permettre des cadres plus larges

    // Timer pour mise à jour chaque seconde
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CountdownWidget::updateCountdown);
    m_timer->start(1000); // Mise à jour chaque seconde
}

void CountdownWidget::setDuration(int seconds)
{
    m_totalSeconds = seconds;
    m_remainingSeconds = seconds;
    m_isStartupMode = (seconds == 0);
    m_isNeverMode = (seconds == -1); // Mode "Jamais"
    update();
}

void CountdownWidget::setRemainingTime(int seconds)
{
    m_remainingSeconds = seconds;
    update();
}

void CountdownWidget::pauseCountdown()
{
    m_isPaused = true;
    update();
}

void CountdownWidget::resumeCountdown()
{
    m_isPaused = false;
    update();
}

bool CountdownWidget::isPaused() const
{
    return m_isPaused;
}

void CountdownWidget::paintEvent(QPaintEvent *)
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

    QString timeText;
    if (m_isPaused) {
        timeText = "PAUSE";
        painter.setPen(QColor("#ff9800")); // Orange pour indiquer la pause
    } else {
        // Formatage du temps restant
        int hours = m_remainingSeconds / 3600;
        int minutes = (m_remainingSeconds % 3600) / 60;
        int seconds = m_remainingSeconds % 60;

        if (hours > 0) {
            timeText = QString("%1h %2m").arg(hours).arg(minutes);
        } else if (minutes > 0) {
            timeText = QString("%1m %2s").arg(minutes).arg(seconds);
        } else {
            timeText = QString("%1s").arg(seconds);
        }
    }

    painter.drawText(circleRect, Qt::AlignCenter, timeText);
}

void CountdownWidget::updateCountdown()
{
    if (m_isStartupMode || m_isNeverMode || m_isPaused) {
        // Pas de countdown en mode démarrage, jamais, ou si en pause
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