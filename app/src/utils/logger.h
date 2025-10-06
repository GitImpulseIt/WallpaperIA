#pragma once

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QStandardPaths>
#include <QDir>

class Logger
{
public:
    static Logger& instance()
    {
        static Logger instance;
        return instance;
    }

    void log(const QString &message, const QString &level = "INFO")
    {
        QMutexLocker locker(&mutex);

        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        QString logLine = QString("[%1] [%2] %3\n").arg(timestamp).arg(level).arg(message);

        // Écrire dans le fichier
        if (logFile.isOpen()) {
            QTextStream stream(&logFile);
            stream << logLine;
            stream.flush(); // Forcer l'écriture immédiate
        }
    }

    void debug(const QString &message) { log(message, "DEBUG"); }
    void info(const QString &message) { log(message, "INFO"); }
    void warning(const QString &message) { log(message, "WARNING"); }
    void error(const QString &message) { log(message, "ERROR"); }
    void critical(const QString &message) { log(message, "CRITICAL"); }

private:
    Logger()
    {
        // Créer le fichier de log dans AppData
        QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/logs";
        QDir().mkpath(logDir);

        QString logPath = logDir + "/wallpaper_debug.log";
        logFile.setFileName(logPath);

        // Ouvrir en mode append
        if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            log("=== Logger initialized ===", "INFO");
        }
    }

    ~Logger()
    {
        if (logFile.isOpen()) {
            log("=== Logger shutdown ===", "INFO");
            logFile.close();
        }
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    QFile logFile;
    QMutex mutex;
};

// Macros pour simplifier l'utilisation
#define LOG_DEBUG(msg) Logger::instance().debug(QString("%1:%2 - %3").arg(__FILE__).arg(__LINE__).arg(msg))
#define LOG_INFO(msg) Logger::instance().info(msg)
#define LOG_WARNING(msg) Logger::instance().warning(msg)
#define LOG_ERROR(msg) Logger::instance().error(msg)
#define LOG_CRITICAL(msg) Logger::instance().critical(msg)
