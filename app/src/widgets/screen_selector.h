#pragma once

#include <QWidget>
#include <QApplication>
#include <QMap>
#include <QList>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainterPath>
#include <QColor>
#include <QPen>

// Classe pour le sélecteur d'écran avec esthétique moderne
class ScreenSelector : public QWidget
{
    Q_OBJECT

public:
    ScreenSelector(QWidget *parent = nullptr);

    void setScreenSelected(int screenIndex, bool selected);
    bool isScreenSelected(int screenIndex) const;
    QList<int> getSelectedScreens() const;
    void setScreenCanBeDeselected(int screenIndex, bool canDeselect);
    bool canScreenBeDeselected(int screenIndex) const;
    int screenCount() const;
    void refresh(); // Rafraîchir le widget après un changement de configuration

signals:
    void screenSelectionChanged(const QList<int> &selectedScreens);
    void screenDeselectionBlocked(int screenIndex);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void setupScreens();
    void setupUI();

    int m_screenCount;
    QMap<int, bool> m_selectedScreens;      // Écrans sélectionnés
    QMap<int, bool> m_canBeDeselected;      // Écrans qui peuvent être désélectionnés
};