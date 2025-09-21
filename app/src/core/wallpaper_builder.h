#pragma once

#include <QRect>
#include <QPixmap>
#include <QList>
#include <QMap>
#include <QString>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

// Structure pour mapper un écran avec son image source
struct ScreenMapping {
    QRect screenRect;           // Rectangle de l'écran dans le bureau virtuel
    QRect sourceRect;           // Rectangle dans l'image source à utiliser
    QRect destRect;             // Rectangle de destination dans l'image composite
    QString imagePath;          // Chemin vers l'image source
    bool isPrimary;             // Écran principal

    ScreenMapping() : isPrimary(false) {}
};

// Classe pour construire les fonds d'écran multi-écrans
class WallpaperBuilder
{
public:
    WallpaperBuilder();

    // Fonction principale pour créer un fond d'écran multi-écrans
    bool createMultiScreenWallpaper(const QMap<int, QString> &imagePaths, const QString &outputPath);

    // Fonctions de calcul du bureau virtuel
    QRect calculateVirtualDesktopBounds();
    QList<ScreenMapping> generateScreenMappings(const QMap<int, QString> &imagePaths);

    // Génération d'image composite
    QPixmap createCompositeImageFromMappings(const QList<ScreenMapping> &mappings, const QRect &virtualDesktop, int adjustmentMode = 0);

    // Wrapping pour compatibilité Windows
    QPixmap wrapCoordinatesForWindows(const QPixmap &sourceImage, const QRect &virtualDesktop);
    bool needsCoordinateWrapping(const QRect &virtualDesktop);

    // Fonction pour obtenir le chemin de sauvegarde temporaire
    QString getTemporaryWallpaperPath();

private:
    // Helpers privés
    QString getScreenName(QScreen* screen, int index);
    bool isRealPrimaryScreen(QScreen* screen);

    // Fonctions d'ajustement d'image
    QPixmap applyAdjustmentMode(const QPixmap &sourceImage, const QSize &targetSize, int adjustmentMode);

    // Détection de version Windows
    bool isWindows8OrLater();
};