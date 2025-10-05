#ifndef STARTUP_MANAGER_H
#define STARTUP_MANAGER_H

#include <QString>

class StartupManager
{
public:
    StartupManager();

    static bool addToWindowsStartup(const QString &appName, const QString &appPath);
    static bool removeFromWindowsStartup(const QString &appName);
    static bool isInWindowsStartup(const QString &appName);
};

#endif // STARTUP_MANAGER_H
