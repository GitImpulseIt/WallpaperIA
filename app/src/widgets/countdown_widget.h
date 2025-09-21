#pragma once

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QFont>
#include <QRect>
#include <QPixmap>
#include <QColor>
#include <QPen>
#include <QString>

// Classe pour afficher un compte à rebours avec camembert de progression
class CountdownWidget : public QWidget
{
    Q_OBJECT

signals:
    void countdownExpired(); // Signal émis quand le compte à rebours expire

public:
    CountdownWidget(QWidget *parent = nullptr);

    void setDuration(int seconds);
    void setRemainingTime(int seconds);

protected:
    void paintEvent(QPaintEvent *) override;

private slots:
    void updateCountdown();

private:
    int m_totalSeconds;
    int m_remainingSeconds;
    bool m_isStartupMode;
    bool m_isNeverMode;
    QTimer *m_timer;
};