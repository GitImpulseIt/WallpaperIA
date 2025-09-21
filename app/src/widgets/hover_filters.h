#pragma once

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QEvent>
#include <QLabel>
#include <QPixmap>
#include <QIcon>
#include <QVariant>
#include <QString>

// Classe pour gérer les événements de survol des catégories
class CategoryHoverFilter : public QObject
{
    Q_OBJECT

public:
    CategoryHoverFilter(QWidget *categoryFrame, const QString &categoryId, QWidget *parent);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

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
    StarHoverFilter(QPushButton *starBtn, const QString &categoryId, int starIndex, QWidget *parent);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void updateStarPreview(const QString &categoryId, int rating);

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
    DisableHoverFilter(QPushButton *disableBtn, const QString &categoryId, QWidget *parent);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void updateStarPreview(const QString &categoryId, int rating);

    QPushButton *m_disableBtn;
    QString m_categoryId;
    QWidget *m_parent;
};