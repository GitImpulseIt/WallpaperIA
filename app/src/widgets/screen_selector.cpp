#include "screen_selector.h"

ScreenSelector::ScreenSelector(QWidget *parent) : QWidget(parent), m_screenCount(0)
{
    setFixedHeight(35);
    setupScreens();
    setupUI();
    // Sélectionner tous les écrans par défaut
    for (int i = 0; i < m_screenCount; i++) {
        m_selectedScreens[i] = true;
    }
}

void ScreenSelector::setScreenSelected(int screenIndex, bool selected)
{
    if (screenIndex >= 0 && screenIndex < m_screenCount) {
        m_selectedScreens[screenIndex] = selected;
        update();
        emit screenSelectionChanged(getSelectedScreens());
    }
}

bool ScreenSelector::isScreenSelected(int screenIndex) const
{
    return m_selectedScreens.value(screenIndex, false);
}

QList<int> ScreenSelector::getSelectedScreens() const
{
    QList<int> selected;
    for (auto it = m_selectedScreens.constBegin(); it != m_selectedScreens.constEnd(); ++it) {
        if (it.value()) {
            selected.append(it.key());
        }
    }
    return selected;
}

void ScreenSelector::setScreenCanBeDeselected(int screenIndex, bool canDeselect)
{
    m_canBeDeselected[screenIndex] = canDeselect;
    update();
}

bool ScreenSelector::canScreenBeDeselected(int screenIndex) const
{
    return m_canBeDeselected.value(screenIndex, false);
}

int ScreenSelector::screenCount() const
{
    return m_screenCount;
}

void ScreenSelector::refresh()
{
    // Protection contre les appels simultanés
    if (m_isRefreshing) {
        return; // Ignorer si un refresh est déjà en cours
    }

    m_isRefreshing = true;

    // Sauvegarder l'état de sélection actuel
    QMap<int, bool> oldSelection = m_selectedScreens;
    QMap<int, bool> oldCanDeselect = m_canBeDeselected;

    // Reconfigurer les écrans
    setupScreens();
    setupUI();

    // Restaurer l'état de sélection pour les écrans qui existent encore
    m_selectedScreens.clear();
    m_canBeDeselected.clear();

    for (int i = 0; i < m_screenCount; i++) {
        // Restaurer la sélection si l'écran existait avant, sinon sélectionner par défaut
        m_selectedScreens[i] = oldSelection.value(i, true);
        // Restaurer l'état de désélection
        m_canBeDeselected[i] = oldCanDeselect.value(i, true);
    }

    // Redessiner le widget
    update();

    // Émettre le signal de changement de sélection
    emit screenSelectionChanged(getSelectedScreens());

    m_isRefreshing = false;
}

void ScreenSelector::paintEvent(QPaintEvent *)
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

    // Dessiner les numéros et l'icône de verrou
    painter.setPen(QColor("#ffffff"));
    QFont font = painter.font();
    font.setWeight(QFont::Bold);
    painter.setFont(font);

    for (int i = 0; i < m_screenCount; i++) {
        QRect tabRect(i * tabWidth, 0, tabWidth, height());

        // Numéro de l'écran
        QString screenText = QString::number(i + 1);
        painter.drawText(tabRect, Qt::AlignCenter, screenText);

        // Icône de verrou si l'écran ne peut pas être désélectionné
        if (!canScreenBeDeselected(i)) {
            QRect lockRect = tabRect.adjusted(0, 0, -5, 0); // Petit décalage à droite
            painter.drawText(lockRect, Qt::AlignRight, "🔒");
        }
    }
}

void ScreenSelector::mousePressEvent(QMouseEvent *event)
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

void ScreenSelector::setupScreens()
{
    // Détecter le nombre d'écrans
    m_screenCount = QApplication::screens().count();
    if (m_screenCount < 1) {
        m_screenCount = 1; // Au moins un écran par défaut
    }
}

void ScreenSelector::setupUI()
{
    // Calculer la largeur optimale basée sur le nombre d'écrans
    int optimalWidth = m_screenCount * 80; // 80px par onglet
    setMinimumSize(optimalWidth, 35);
    setMaximumHeight(35);
    // Permettre l'expansion horizontale pour l'alignement justifié
}