#include "wallpaper_builder.h"
#include "../utils/logger.h"
#include <QOperatingSystemVersion>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

WallpaperBuilder::WallpaperBuilder()
{
    LOG_INFO("WallpaperBuilder instance created");
}

bool WallpaperBuilder::createMultiScreenWallpaper(const QMap<int, QString> &imagePaths, const QString &outputPath)
{
    LOG_INFO(">>> WallpaperBuilder::createMultiScreenWallpaper() STARTED");
    LOG_INFO(QString("Image paths count: %1, Output: %2").arg(imagePaths.size()).arg(outputPath));

    try {
        // 1. Calculer les dimensions du bureau virtuel
        LOG_INFO("Step 1: Calculating virtual desktop bounds...");
        QRect virtualDesktop = calculateVirtualDesktopBounds();
        LOG_INFO(QString("Virtual desktop: x=%1, y=%2, w=%3, h=%4")
            .arg(virtualDesktop.x()).arg(virtualDesktop.y())
            .arg(virtualDesktop.width()).arg(virtualDesktop.height()));

        // 2. Générer les mappings pour chaque écran
        LOG_INFO("Step 2: Generating screen mappings...");
        QList<ScreenMapping> mappings = generateScreenMappings(imagePaths);
        LOG_INFO(QString("Mappings generated: %1").arg(mappings.size()));

        if (mappings.isEmpty()) {
            LOG_ERROR("No mappings generated, aborting");
            return false;
        }

        // 3. Créer l'image composite avec mapping précis
        LOG_INFO("Step 3: Creating composite image...");
        QPixmap composite = createCompositeImageFromMappings(mappings, virtualDesktop);
        LOG_INFO(QString("Composite created: %1x%2").arg(composite.width()).arg(composite.height()));

        // 4. Appliquer le wrapping selon la logique DMT (Windows < 8 uniquement)
        LOG_INFO("Step 4: Applying coordinate wrapping...");
        QPixmap finalImage = wrapCoordinatesForWindows(composite, virtualDesktop);
        LOG_INFO(QString("Final image: %1x%2").arg(finalImage.width()).arg(finalImage.height()));

        // 5. Sauvegarder l'image finale
        LOG_INFO("Step 5: Saving final image...");
        if (!finalImage.save(outputPath, "BMP")) {
            LOG_ERROR("Failed to save final image");
            return false;
        }

        LOG_INFO("Image saved successfully");
        LOG_INFO("<<< WallpaperBuilder::createMultiScreenWallpaper() ENDED (success)");
        return true;
    } catch (const std::exception &e) {
        LOG_CRITICAL(QString("Exception in createMultiScreenWallpaper: %1").arg(e.what()));
        return false;
    } catch (...) {
        LOG_CRITICAL("Unknown exception in createMultiScreenWallpaper");
        return false;
    }
}

QRect WallpaperBuilder::calculateVirtualDesktopBounds()
{
    QList<QScreen*> screens = QApplication::screens();
    if (screens.isEmpty()) return QRect();

    // Calculer avec résolutions natives comme dans screenmap.cpp
    int minX = 0, minY = 0, maxX = 0, maxY = 0;

    QScreen* qtPrimaryScreen = QApplication::primaryScreen();
    if (!qtPrimaryScreen) return QRect(); // Protection contre primaryScreen null

    for (int i = 0; i < screens.size(); i++) {
        QScreen* screen = screens[i];
        if (!screen) continue; // Protection contre écran null
        QRect geom = screen->geometry();
        QSize realSize = screen->size() * screen->devicePixelRatio();
        bool isQtPrimary = (screen == qtPrimaryScreen);

        // Coordonnées : garder telles quelles (pas de scaling)
        int realX = geom.x();
        int realY = geom.y();

        // Tailles : utiliser résolutions réelles
        int realW = realSize.width();
        int realH = realSize.height();

        if (realX < minX) minX = realX;
        if (realY < minY) minY = realY;
        if (realX + realW > maxX) maxX = realX + realW;
        if (realY + realH > maxY) maxY = realY + realH;
    }

    QRect bounds(minX, minY, maxX - minX, maxY - minY);
    return bounds;
}

QList<ScreenMapping> WallpaperBuilder::generateScreenMappings(const QMap<int, QString> &imagePaths)
{
    QList<ScreenMapping> mappings;
    QList<QScreen*> screens = QApplication::screens();

    if (screens.isEmpty()) {
        return mappings;
    }

    // Calculer les dimensions totales avec résolution réelle comme dans screenmap.cpp
    int minX = 0, minY = 0, maxX = 0, maxY = 0;

    for (QScreen* screen : screens) {
        QRect geom = screen->geometry();
        QSize realSize = screen->size() * screen->devicePixelRatio();

        // Coordonnées : garder telles quelles (pas de scaling)
        int realX = geom.x();
        int realY = geom.y();

        // Tailles : utiliser résolutions réelles
        int realW = realSize.width();
        int realH = realSize.height();

        if (realX < minX) minX = realX;
        if (realY < minY) minY = realY;
        if (realX + realW > maxX) maxX = realX + realW;
        if (realY + realH > maxY) maxY = realY + realH;
    }

    // Créer les mappings pour chaque écran avec ses images
    for (auto it = imagePaths.constBegin(); it != imagePaths.constEnd(); ++it) {
        int screenIndex = it.key();
        QString imagePath = it.value();

        if (screenIndex >= 0 && screenIndex < screens.size()) {
            QScreen* screen = screens[screenIndex];
            if (!screen) continue; // Protection contre écran null

            QRect geom = screen->geometry();
            QSize realSize = screen->size() * screen->devicePixelRatio();

            ScreenMapping mapping;
            mapping.screenRect = QRect(geom.x(), geom.y(), realSize.width(), realSize.height());
            mapping.imagePath = imagePath;
            mapping.isPrimary = (screen == QApplication::primaryScreen());
            mapping.destRect = QRect(geom.x() - minX, geom.y() - minY, realSize.width(), realSize.height());

            // Pour l'image source, utiliser toute l'image
            QPixmap sourceImage(imagePath);
            if (!sourceImage.isNull()) {
                mapping.sourceRect = QRect(0, 0, sourceImage.width(), sourceImage.height());
            } else {
                mapping.sourceRect = QRect(0, 0, realSize.width(), realSize.height());
            }

            mappings.append(mapping);
        }
    }

    return mappings;
}

QPixmap WallpaperBuilder::createCompositeImageFromMappings(const QList<ScreenMapping> &mappings, const QRect &virtualDesktop, int adjustmentMode)
{
    QPixmap composite(virtualDesktop.size());
    composite.fill(Qt::black);

    QPainter painter(&composite);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const ScreenMapping &mapping : mappings) {
        QPixmap sourceImage(mapping.imagePath);
        if (!sourceImage.isNull()) {
            // Appliquer le mode d'ajustement choisi par l'utilisateur
            QPixmap adjustedImage = applyAdjustmentMode(sourceImage, mapping.destRect.size(), adjustmentMode);
            painter.drawPixmap(mapping.destRect, adjustedImage);
        }
    }

    painter.end();
    return composite;
}

QPixmap WallpaperBuilder::applyAdjustmentMode(const QPixmap &sourceImage, const QSize &targetSize, int adjustmentMode)
{
    if (sourceImage.isNull()) {
        return QPixmap();
    }

    QPixmap result(targetSize);
    result.fill(Qt::black);
    QPainter painter(&result);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);

    switch (adjustmentMode) {
        case 0: // Remplir (Cover) - conserve le ratio, peut dépasser
        {
            QPixmap scaledImage = sourceImage.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            // Centrer l'image si elle dépasse
            int x = (targetSize.width() - scaledImage.width()) / 2;
            int y = (targetSize.height() - scaledImage.height()) / 2;
            painter.drawPixmap(x, y, scaledImage);
            break;
        }
        case 1: // Ajuster (Contain) - conserve le ratio, bandes noires
        {
            QPixmap scaledImage = sourceImage.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            // Centrer l'image dans l'espace disponible
            int x = (targetSize.width() - scaledImage.width()) / 2;
            int y = (targetSize.height() - scaledImage.height()) / 2;
            painter.drawPixmap(x, y, scaledImage);
            break;
        }
        case 2: // Étendre - ce mode ne devrait pas être utilisé en multi-écran
        case 3: // Étirer - déforme pour remplir exactement
        {
            QPixmap scaledImage = sourceImage.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            painter.drawPixmap(0, 0, scaledImage);
            break;
        }
        case 4: // Mosaïque - répète l'image
        {
            for (int x = 0; x < targetSize.width(); x += sourceImage.width()) {
                for (int y = 0; y < targetSize.height(); y += sourceImage.height()) {
                    painter.drawPixmap(x, y, sourceImage);
                }
            }
            break;
        }
        default: // Par défaut, utiliser le mode remplir
        {
            QPixmap scaledImage = sourceImage.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            int x = (targetSize.width() - scaledImage.width()) / 2;
            int y = (targetSize.height() - scaledImage.height()) / 2;
            painter.drawPixmap(x, y, scaledImage);
            break;
        }
    }

    painter.end();
    return result;
}

bool WallpaperBuilder::isWindows8OrLater()
{
    #ifdef Q_OS_WIN
    // Utiliser QSysInfo pour détecter la version Windows
    QOperatingSystemVersion current = QOperatingSystemVersion::current();

    // Windows 8 = version 6.2, Windows 7 = version 6.1
    // Windows 8 et plus récent supportent les coordonnées virtuelles directement
    return (current.majorVersion() > 6) ||
           (current.majorVersion() == 6 && current.minorVersion() >= 2);
    #else
    return true; // Non-Windows, assumer le support moderne
    #endif
}

bool WallpaperBuilder::needsCoordinateWrapping(const QRect &virtualDesktop)
{
    // Logique DMT : wrapping nécessaire seulement si Windows < 8 ET coordonnées négatives
    bool hasNegativeCoords = (virtualDesktop.left() < 0 || virtualDesktop.top() < 0);
    bool isOldWindows = !isWindows8OrLater();

    return hasNegativeCoords && isOldWindows;
}

QPixmap WallpaperBuilder::wrapCoordinatesForWindows(const QPixmap &sourceImage, const QRect &virtualDesktop)
{
    if (!needsCoordinateWrapping(virtualDesktop)) {
        return sourceImage; // Pas besoin de wrapping
    }

    // Créer une nouvelle image avec les coordonnées wrappées
    QPixmap wrappedImage(sourceImage.size());
    wrappedImage.fill(Qt::black);

    QPainter painter(&wrappedImage);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    int xWrap = -virtualDesktop.left();
    int yWrap = -virtualDesktop.top();
    int xNotWrap = sourceImage.width() - xWrap;
    int yNotWrap = sourceImage.height() - yWrap;

    // Quadrant en haut à droite (a)
    if (xWrap > 0 && yWrap > 0) {
        QRect srcRect(0, 0, xWrap, yWrap);
        QRect destRect(xNotWrap, yNotWrap, xWrap, yWrap);
        painter.drawPixmap(destRect, sourceImage, srcRect);
    }

    // Quadrant en haut à gauche (b)
    if (yWrap > 0 && xNotWrap > 0) {
        QRect srcRect(xWrap, 0, xNotWrap, yWrap);
        QRect destRect(0, yNotWrap, xNotWrap, yWrap);
        painter.drawPixmap(destRect, sourceImage, srcRect);
    }

    // Quadrant en bas à droite (c)
    if (xWrap > 0 && yNotWrap > 0) {
        QRect srcRect(0, yWrap, xWrap, yNotWrap);
        QRect destRect(xNotWrap, 0, xWrap, yNotWrap);
        painter.drawPixmap(destRect, sourceImage, srcRect);
    }

    // Quadrant en bas à gauche (d) - écran principal
    if (xNotWrap > 0 && yNotWrap > 0) {
        QRect srcRect(xWrap, yWrap, xNotWrap, yNotWrap);
        QRect destRect(0, 0, xNotWrap, yNotWrap);
        painter.drawPixmap(destRect, sourceImage, srcRect);
    }

    painter.end();
    return wrappedImage;
}

QString WallpaperBuilder::getTemporaryWallpaperPath()
{
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/WallpaperAI";
    QDir().mkpath(tempDir);
    return tempDir + "/composite_wallpaper.bmp";
}

QString WallpaperBuilder::getScreenName(QScreen* screen, int index)
{
    QString screenName = screen->name();
    if (screenName.isEmpty()) {
        screenName = QString("Écran %1").arg(index + 1);
    }
    return screenName;
}

bool WallpaperBuilder::isRealPrimaryScreen(QScreen* screen)
{
    return screen == QApplication::primaryScreen();
}