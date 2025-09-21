#pragma once

#include <QWidget>
#include <QPropertyAnimation>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QEasingCurve>
#include <QColor>
#include <QRect>

// Classe pour créer un bouton à bascule personnalisé avec animation
class ToggleSwitch : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal animationProgress READ animationProgress WRITE setAnimationProgress)

public:
    ToggleSwitch(QWidget *parent = nullptr);

    bool isChecked() const;
    void setChecked(bool checked);

    qreal animationProgress() const;
    void setAnimationProgress(qreal progress);

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

private:
    bool m_checked;
    qreal m_animationProgress;
    QPropertyAnimation *m_animation;
};