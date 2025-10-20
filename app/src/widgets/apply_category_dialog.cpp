#include "apply_category_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ApplyCategoryDialog::ApplyCategoryDialog(const QString &categoryId, const QString &categoryName,
                                         const QString &thumbnailFilename, bool multiScreenMode,
                                         int screenCount, QWidget *parent)
    : QDialog(parent), m_categoryId(categoryId), m_categoryName(categoryName),
      m_thumbnailFilename(thumbnailFilename), m_multiScreenMode(multiScreenMode)
{
    setWindowTitle("Appliquer un fond d'écran");
    setModal(true);
    setFixedWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Titre
    QLabel *titleLabel = new QLabel("Catégorie : " + categoryName);
    titleLabel->setStyleSheet("font-size: 14pt; font-weight: bold; color: #2196F3;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Sélecteur d'écran (uniquement si multi-écran activé)
    if (multiScreenMode && screenCount > 1) {
        QLabel *screenLabel = new QLabel("Écran cible :");
        screenLabel->setStyleSheet("color: white; font-size: 11pt;");
        mainLayout->addWidget(screenLabel);

        screenCombo = new QComboBox();
        screenCombo->setStyleSheet(
            "QComboBox {"
            "   background-color: #2d2d2d;"
            "   color: white;"
            "   border: 1px solid #2196F3;"
            "   border-radius: 5px;"
            "   padding: 8px;"
            "   font-size: 11pt;"
            "}"
            "QComboBox::drop-down {"
            "   border: none;"
            "}"
            "QComboBox::down-arrow {"
            "   image: none;"
            "   border-left: 5px solid transparent;"
            "   border-right: 5px solid transparent;"
            "   border-top: 5px solid white;"
            "   margin-right: 10px;"
            "}"
        );

        for (int i = 0; i < screenCount; i++) {
            screenCombo->addItem(QString("Écran %1").arg(i + 1), i);
        }
        mainLayout->addWidget(screenCombo);
    } else {
        screenCombo = nullptr;
    }

    mainLayout->addSpacing(10);

    // Bouton : Appliquer cette image (miniature)
    applyThumbnailBtn = new QPushButton("Appliquer cette image");
    applyThumbnailBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #2196F3;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 12px;"
        "   font-size: 11pt;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #0D47A1;"
        "}"
    );
    connect(applyThumbnailBtn, &QPushButton::clicked, this, &ApplyCategoryDialog::onApplyThumbnail);
    mainLayout->addWidget(applyThumbnailBtn);

    // Bouton : Appliquer une image de cette catégorie (aléatoire)
    applyRandomBtn = new QPushButton("Appliquer une image de cette catégorie");
    applyRandomBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #8b4513;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 12px;"
        "   font-size: 11pt;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #6d3410;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #4d240b;"
        "}"
    );
    connect(applyRandomBtn, &QPushButton::clicked, this, &ApplyCategoryDialog::onApplyRandom);
    mainLayout->addWidget(applyRandomBtn);

    mainLayout->addSpacing(10);

    // Bouton : Fermer
    closeBtn = new QPushButton("Fermer");
    closeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #555555;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 10px;"
        "   font-size: 10pt;"
        "}"
        "QPushButton:hover {"
        "   background-color: #666666;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #444444;"
        "}"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    mainLayout->addWidget(closeBtn);

    // Style du dialogue
    setStyleSheet("QDialog { background-color: #1e1e1e; }");
}

void ApplyCategoryDialog::onApplyThumbnail()
{
    int screenIndex = 0;
    if (screenCombo) {
        screenIndex = screenCombo->currentData().toInt();
    }
    emit applyThumbnail(screenIndex);
    accept();
}

void ApplyCategoryDialog::onApplyRandom()
{
    int screenIndex = 0;
    if (screenCombo) {
        screenIndex = screenCombo->currentData().toInt();
    }
    emit applyRandomFromCategory(screenIndex);
    accept();
}
