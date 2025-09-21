#include "utils.h"

QString getImagePath(const QString &imageName) {
    return QCoreApplication::applicationDirPath() + "/" + imageName;
}