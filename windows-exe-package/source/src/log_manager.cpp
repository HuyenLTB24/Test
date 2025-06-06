#include "log_manager.h"
#include <QDir>
#include <QDebug>

LogManager::LogManager(QObject* parent)
    : QObject(parent)
    , m_logLevel(LogLevel::Info)
    , m_maxFileSize(10 * 1024 * 1024) // 10MB
    , m_maxFiles(10)
    , m_flushTimer(new QTimer(this))
    , m_logFile(nullptr)
    , m_logStream(nullptr)
    , m_totalLogCount(0)
    , m_errorCount(0)
    , m_warningCount(0)
{
    connect(m_flushTimer, &QTimer::timeout, this, &LogManager::flushPendingLogs);
    m_flushTimer->setInterval(5000); // Flush every 5 seconds
    m_flushTimer->start();
    
    // Set default log directory
    setLogDirectory("logs");
}

LogManager::~LogManager()
{
    flush();
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
    }
    if (m_logStream) {
        delete m_logStream;
    }
}

void LogManager::setLogDirectory(const QString& directory)
{
    QMutexLocker locker(&m_mutex);
    m_logDirectory = directory;
    
    // Create directory if it doesn't exist
    QDir().mkpath(directory);
    
    // Initialize log file
    openLogFile();
}

void LogManager::setMaxFileSize(qint64 bytes)
{
    QMutexLocker locker(&m_mutex);
    m_maxFileSize = bytes;
}

void LogManager::setMaxFiles(int count)
{
    QMutexLocker locker(&m_mutex);
    m_maxFiles = count;
}

void LogManager::setLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_logLevel = level;
}

void LogManager::setFlushInterval(int milliseconds)
{
    m_flushTimer->setInterval(milliseconds);
}

void LogManager::logDebug(const QString& message, const QString& category, const QString& username)
{
    log(LogLevel::Debug, message, category, username);
}

void LogManager::logInfo(const QString& message, const QString& category, const QString& username)
{
    log(LogLevel::Info, message, category, username);
}

void LogManager::logWarning(const QString& message, const QString& category, const QString& username)
{
    log(LogLevel::Warning, message, category, username);
}

void LogManager::logError(const QString& message, const QString& category, const QString& username)
{
    log(LogLevel::Error, message, category, username);
}

void LogManager::logCritical(const QString& message, const QString& category, const QString& username)
{
    log(LogLevel::Critical, message, category, username);
}

void LogManager::log(LogLevel level, const QString& message, const QString& category, const QString& username)
{
    if (level < m_logLevel) {
        return;
    }
    
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = level;
    entry.category = category;
    entry.message = message;
    entry.username = username;
    
    QMutexLocker locker(&m_mutex);
    m_pendingLogs.enqueue(entry);
    
    // Also output to console for debugging
    QString levelStr;
    switch (level) {
        case LogLevel::Debug: levelStr = "DEBUG"; break;
        case LogLevel::Info: levelStr = "INFO"; break;
        case LogLevel::Warning: levelStr = "WARNING"; break;
        case LogLevel::Error: levelStr = "ERROR"; break;
        case LogLevel::Critical: levelStr = "CRITICAL"; break;
    }
    
    qDebug() << QString("[%1] %2: %3").arg(levelStr, category.isEmpty() ? "General" : category, message);
}

void LogManager::flush()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_logStream || m_pendingLogs.isEmpty()) {
        return;
    }
    
    while (!m_pendingLogs.isEmpty()) {
        LogEntry entry = m_pendingLogs.dequeue();
        writeLogEntry(entry);
    }
    
    m_logStream->flush();
}

void LogManager::flushPendingLogs()
{
    flush();
}

void LogManager::openLogFile()
{
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
    
    if (m_logStream) {
        delete m_logStream;
        m_logStream = nullptr;
    }
    
    QString fileName = QString("%1/app_%2.log")
                      .arg(m_logDirectory)
                      .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    
    m_logFile = new QFile(fileName);
    if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        m_logStream = new QTextStream(m_logFile);
    }
}

void LogManager::writeLogEntry(const LogEntry& entry)
{
    if (!m_logStream) {
        return;
    }
    
    QString levelStr;
    switch (entry.level) {
        case LogLevel::Debug: levelStr = "DEBUG"; break;
        case LogLevel::Info: levelStr = "INFO"; break;
        case LogLevel::Warning: levelStr = "WARNING"; break;
        case LogLevel::Error: levelStr = "ERROR"; break;
        case LogLevel::Critical: levelStr = "CRITICAL"; break;
    }
    
    QString logLine = QString("[%1] [%2] [%3] [%4] %5")
                     .arg(entry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(levelStr)
                     .arg(entry.category.isEmpty() ? "General" : entry.category)
                     .arg(entry.username.isEmpty() ? "System" : entry.username)
                     .arg(entry.message);
    
    *m_logStream << logLine << endl;
    
    // Update statistics
    m_totalLogCount++;
    if (entry.level == LogLevel::Error || entry.level == LogLevel::Critical) {
        m_errorCount++;
    } else if (entry.level == LogLevel::Warning) {
        m_warningCount++;
    }
}

// Add missing method implementations
void LogManager::rotateLogFile() {
    // Stub implementation
}

void LogManager::clearLogs() {
    // Stub implementation
}

QStringList LogManager::getLogFiles() const {
    // Stub implementation
    return QStringList();
}

QList<LogEntry> LogManager::getRecentLogs(int count) const {
    Q_UNUSED(count)
    // Stub implementation
    return QList<LogEntry>();
}

QList<LogEntry> LogManager::getLogsForUser(const QString& username, int count) const {
    Q_UNUSED(username)
    Q_UNUSED(count)
    // Stub implementation
    return QList<LogEntry>();
}

QList<LogEntry> LogManager::getLogsForCategory(const QString& category, int count) const {
    Q_UNUSED(category)
    Q_UNUSED(count)
    // Stub implementation
    return QList<LogEntry>();
}

QList<LogEntry> LogManager::getLogsForLevel(LogLevel level, int count) const {
    Q_UNUSED(level)
    Q_UNUSED(count)
    // Stub implementation
    return QList<LogEntry>();
}

int LogManager::getTotalLogCount() const {
    return m_totalLogCount;
}

int LogManager::getErrorCount() const {
    return m_errorCount;
}

int LogManager::getWarningCount() const {
    return m_warningCount;
}

void LogManager::closeLogFile() {
    // Stub implementation
}

QString LogManager::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

LogLevel LogManager::stringToLevel(const QString& level) const {
    if (level == "DEBUG") return LogLevel::Debug;
    if (level == "INFO") return LogLevel::Info;
    if (level == "WARNING") return LogLevel::Warning;
    if (level == "ERROR") return LogLevel::Error;
    if (level == "CRITICAL") return LogLevel::Critical;
    return LogLevel::Info;
}

QString LogManager::getLogFileName() const {
    return QString("%1/app_%2.log")
          .arg(m_logDirectory)
          .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
}

void LogManager::cleanupOldFiles() {
    // Stub implementation
}


