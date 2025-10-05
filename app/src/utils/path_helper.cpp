#include "path_helper.h"

QString PathHelper::getWallpapersDirectory()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/wallpapers";
    QDir().mkpath(dir);
    return dir;
}

QString PathHelper::getHistoryDirectory()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/history";
    QDir().mkpath(dir);
    return dir;
}

QString PathHelper::getCategoriesCachePath()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir().mkpath(cacheDir);
    return cacheDir + "/categories_cache.json";
}

QString PathHelper::getThumbnailsCacheDirectory()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/thumbnails";
    QDir().mkpath(dir);
    return dir;
}
