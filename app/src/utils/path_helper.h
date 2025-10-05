#ifndef PATH_HELPER_H
#define PATH_HELPER_H

#include <QString>
#include <QStandardPaths>
#include <QDir>

class PathHelper
{
public:
    static QString getWallpapersDirectory();
    static QString getHistoryDirectory();
    static QString getCategoriesCachePath();
    static QString getThumbnailsCacheDirectory();
};

#endif // PATH_HELPER_H
