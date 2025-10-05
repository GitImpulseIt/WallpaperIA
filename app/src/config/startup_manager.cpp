#include "startup_manager.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <string>
#endif

StartupManager::StartupManager()
{
}

bool StartupManager::addToWindowsStartup(const QString &appName, const QString &appPath)
{
#ifdef Q_OS_WIN
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
                               L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                               0, KEY_SET_VALUE, &hKey);

    if (result != ERROR_SUCCESS) {
        return false;
    }

    std::wstring wAppName = appName.toStdWString();
    std::wstring wAppPath = appPath.toStdWString();

    result = RegSetValueExW(hKey, wAppName.c_str(), 0, REG_SZ,
                           reinterpret_cast<const BYTE*>(wAppPath.c_str()),
                           (wAppPath.length() + 1) * sizeof(wchar_t));

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
#else
    Q_UNUSED(appName);
    Q_UNUSED(appPath);
    return false;
#endif
}

bool StartupManager::removeFromWindowsStartup(const QString &appName)
{
#ifdef Q_OS_WIN
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
                               L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                               0, KEY_SET_VALUE, &hKey);

    if (result != ERROR_SUCCESS) {
        return false;
    }

    std::wstring wAppName = appName.toStdWString();
    result = RegDeleteValueW(hKey, wAppName.c_str());

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
#else
    Q_UNUSED(appName);
    return false;
#endif
}

bool StartupManager::isInWindowsStartup(const QString &appName)
{
#ifdef Q_OS_WIN
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
                               L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                               0, KEY_QUERY_VALUE, &hKey);

    if (result != ERROR_SUCCESS) {
        return false;
    }

    std::wstring wAppName = appName.toStdWString();
    DWORD dataType;
    DWORD dataSize = 0;

    result = RegQueryValueExW(hKey, wAppName.c_str(), nullptr, &dataType, nullptr, &dataSize);

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
#else
    Q_UNUSED(appName);
    return false;
#endif
}
