#include "toggle_switch.h"

ToggleSwitch::ToggleSwitch(QWidget *parent) : QWidget(parent), m_checked(false), m_animationProgress(0.0)
{
    setFixedSize(54, 30);
    setCursor(Qt::PointingHandCursor);

    // Configuration de l'animation
    m_animation = new QPropertyAnimation(this, "animationProgress");
    m_animation->setDuration(200); // 200ms pour une animation fluide
    m_animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(m_animation, &QPropertyAnimation::valueChanged, this, QOverload<>::of(&QWidget::update));
}

bool ToggleSwitch::isChecked() const
{
    return m_checked;
}

void ToggleSwitch::setChecked(bool checked)
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

qreal ToggleSwitch::animationProgress() const
{
    return m_animationProgress;
}

void ToggleSwitch::setAnimationProgress(qreal progress)
{
    m_animationProgress = progress;
    update();
}

void ToggleSwitch::paintEvent(QPaintEvent *)
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

void ToggleSwitch::mousePressEvent(QMouseEvent *)
{
    setChecked(!m_checked);
}