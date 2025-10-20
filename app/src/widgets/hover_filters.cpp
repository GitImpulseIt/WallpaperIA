#include "hover_filters.h"
#include "../utils/utils.h"

// CategoryHoverFilter
CategoryHoverFilter::CategoryHoverFilter(QWidget *categoryFrame, const QString &categoryId, QWidget *parent)
    : QObject(parent), m_categoryFrame(categoryFrame), m_categoryId(categoryId), m_parent(parent) {}

bool CategoryHoverFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_categoryFrame) {
        if (event->type() == QEvent::Enter) {
            // Vérifier si la catégorie n'est pas interdite
            QVariant ratingVariant = m_parent->property(("rating_" + m_categoryId).toLocal8Bit().constData());
            int rating = ratingVariant.isValid() ? ratingVariant.toInt() : 1;

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

            // Afficher le bouton "Appliquer" uniquement si la catégorie n'est pas interdite
            if (rating != -1) {
                QPushButton *applyBtn = m_categoryFrame->findChild<QPushButton*>("applyBtn_" + m_categoryId);
                if (applyBtn) {
                    applyBtn->show();
                }
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
                int currentRatingValue = ratingVariant.isValid() ? ratingVariant.toInt() : 1;
                if (currentRatingValue == -1) {
                    QLabel *disableIcon = m_categoryFrame->parentWidget()->findChild<QLabel*>("disableIcon_" + m_categoryId);
                    if (disableIcon) disableIcon->show();
                }
            }

            // Masquer le bouton "Appliquer"
            QPushButton *applyBtn = m_categoryFrame->findChild<QPushButton*>("applyBtn_" + m_categoryId);
            if (applyBtn) {
                applyBtn->hide();
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

// StarHoverFilter
StarHoverFilter::StarHoverFilter(QPushButton *starBtn, const QString &categoryId, int starIndex, QWidget *parent)
    : QObject(parent), m_starBtn(starBtn), m_categoryId(categoryId), m_starIndex(starIndex), m_parent(parent) {}

bool StarHoverFilter::eventFilter(QObject *obj, QEvent *event)
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

void StarHoverFilter::updateStarPreview(const QString &categoryId, int rating) {
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

// DisableHoverFilter
DisableHoverFilter::DisableHoverFilter(QPushButton *disableBtn, const QString &categoryId, QWidget *parent)
    : QObject(parent), m_disableBtn(disableBtn), m_categoryId(categoryId), m_parent(parent) {}

bool DisableHoverFilter::eventFilter(QObject *obj, QEvent *event)
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

void DisableHoverFilter::updateStarPreview(const QString &categoryId, int rating) {
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