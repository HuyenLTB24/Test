#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QTimer>
#include <QQueue>

enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
    Critical = 4
};

struct LogEntry {
    QDateTime timestamp;
    LogLevel level;
    QString category;
    QString message;
    QString username;
};

class LogManager : public QObject
{
    Q_OBJECT

public:
    explicit LogManager(QObject* parent = nullptr);
    ~LogManager();

    // Configuration
    void setLogDirectory(const QString& directory);
    void setMaxFileSize(qint64 bytes);
    void setMaxFiles(int count);
    void setLogLevel(LogLevel level);
    void setFlushInterval(int milliseconds);
    
    // Logging methods
    void logDebug(const QString& message, const QString& category = QString(), const QString& username = QString());
    void logInfo(const QString& message, const QString& category = QString(), const QString& username = QString());
    void logWarning(const QString& message, const QString& category = QString(), const QString& username = QString());
    void logError(const QString& message, const QString& category = QString(), const QString& username = QString());
    void logCritical(const QString& message, const QString& category = QString(), const QString& username = QString());
    
    // Generic logging
    void log(LogLevel level, const QString& message, const QString& category = QString(), const QString& username = QString());
    
    // File management
    void rotateLogFile();
    void clearLogs();
    QStringList getLogFiles() const;
    
    // Reading logs
    QList<LogEntry> getRecentLogs(int count = 1000) const;
    QList<LogEntry> getLogsForUser(const QString& username, int count = 1000) const;
    QList<LogEntry> getLogsForCategory(const QString& category, int count = 1000) const;
    QList<LogEntry> getLogsForLevel(LogLevel level, int count = 1000) const;
    
    // Statistics
    int getTotalLogCount() const;
    int getErrorCount() const;
    int getWarningCount() const;

signals:
    void logEntryAdded(const LogEntry& entry);
    void logFileRotated(const QString& newFileName);

public slots:
    void flush();

private slots:
    void flushPendingLogs();

private:
    void writeLogEntry(const LogEntry& entry);
    void openLogFile();
    void closeLogFile();
    QString levelToString(LogLevel level) const;
    LogLevel stringToLevel(const QString& level) const;
    QString getLogFileName() const;
    void cleanupOldFiles();

private:
    QString m_logDirectory;
    QFile* m_logFile;
    QTextStream* m_logStream;
    
    qint64 m_maxFileSize;
    int m_maxFiles;
    LogLevel m_logLevel;
    
    QQueue<LogEntry> m_pendingLogs;
    QTimer* m_flushTimer;
    
    // Thread safety
    mutable QMutex m_mutex;
    
    // Statistics
    int m_totalLogCount;
    int m_errorCount;
    int m_warningCount;
    
    // Default values
    static const qint64 DEFAULT_MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    static const int DEFAULT_MAX_FILES = 10;
    static const int DEFAULT_FLUSH_INTERVAL = 5000; // 5 seconds
};

#endif // LOG_MANAGER_H
