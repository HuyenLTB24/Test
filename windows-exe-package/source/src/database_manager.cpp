#include "database_manager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QUuid>
#include <QMutexLocker>
#include <QApplication>
#include <QMessageBox>
#include <QWidget>

const QString DatabaseManager::DATABASE_NAME = "x2modern.db";
const QString DatabaseManager::ACCOUNTS_TABLE = "accounts";
const QString DatabaseManager::SETTINGS_TABLE = "settings";
const QString DatabaseManager::STATS_TABLE = "account_stats";
const QString DatabaseManager::REPLIED_TWEETS_TABLE = "replied_tweets";
const QString DatabaseManager::LOGS_TABLE = "logs";

DatabaseManager::DatabaseManager(QWidget* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_databasePath = QDir(appDataPath).absoluteFilePath(DATABASE_NAME);
    
    initializeDatabase();
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool DatabaseManager::initializeDatabase()
{
    QMutexLocker locker(&m_mutex);
    
    m_database = QSqlDatabase::addDatabase("QSQLITE", "main");
    m_database.setDatabaseName(m_databasePath);
    
    if (!m_database.open()) {
        qCritical() << "Failed to open database:" << m_database.lastError().text();
        return false;
    }
    
    return createTables();
}

bool DatabaseManager::createTables()
{
    QStringList createQueries = {
        QString(
            "CREATE TABLE IF NOT EXISTS %1 ("
            "profile_id TEXT PRIMARY KEY, "
            "name TEXT NOT NULL, "
            "username TEXT NOT NULL, "
            "password TEXT, "
            "email TEXT, "
            "phone TEXT, "
            "debug_port INTEGER DEFAULT 9222, "
            "profile_path TEXT, "
            "use_proxy BOOLEAN DEFAULT 0, "
            "proxy_url TEXT, "
            "use_gemini BOOLEAN DEFAULT 1, "
            "gemini_key TEXT, "
            "chatgpt_key TEXT, "
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
            "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
            ")"
        ).arg(ACCOUNTS_TABLE),
        
        QString(
            "CREATE TABLE IF NOT EXISTS %1 ("
            "profile_id TEXT PRIMARY KEY, "
            "max_replies INTEGER DEFAULT 50, "
            "min_views INTEGER DEFAULT 0, "
            "skip_replies BOOLEAN DEFAULT 1, "
            "skip_retweets BOOLEAN DEFAULT 1, "
            "skip_japanese BOOLEAN DEFAULT 0, "
            "auto_like BOOLEAN DEFAULT 1, "
            "auto_follow_verified BOOLEAN DEFAULT 0, "
            "auto_retweet BOOLEAN DEFAULT 0, "
            "japanese_only BOOLEAN DEFAULT 0, "
            "reply_first_only BOOLEAN DEFAULT 0, "
            "minimize_window BOOLEAN DEFAULT 0, "
            "time_limit_hours INTEGER DEFAULT 24, "
            "time_limit_minutes INTEGER DEFAULT 0, "
            "interval INTEGER DEFAULT 30, "
            "reply_interval INTEGER DEFAULT 0, "
            "schedule_enabled BOOLEAN DEFAULT 0, "
            "start_time TEXT DEFAULT '09:00', "
            "end_time TEXT DEFAULT '17:00', "
            "schedule_days TEXT DEFAULT '0,1,2,3,4,5,6', "
            "mode_id INTEGER DEFAULT 1, "
            "target_keywords TEXT, "
            "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
            "FOREIGN KEY (profile_id) REFERENCES %2 (profile_id) ON DELETE CASCADE"
            ")"
        ).arg(SETTINGS_TABLE, ACCOUNTS_TABLE),
        
        QString(
            "CREATE TABLE IF NOT EXISTS %1 ("
            "profile_id TEXT PRIMARY KEY, "
            "replies_sent INTEGER DEFAULT 0, "
            "likes_given INTEGER DEFAULT 0, "
            "follows_made INTEGER DEFAULT 0, "
            "retweets INTEGER DEFAULT 0, "
            "last_activity DATETIME, "
            "total_runtime_minutes INTEGER DEFAULT 0, "
            "success_rate REAL DEFAULT 0.0, "
            "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
            "FOREIGN KEY (profile_id) REFERENCES %2 (profile_id) ON DELETE CASCADE"
            ")"
        ).arg(STATS_TABLE, ACCOUNTS_TABLE),
        
        QString(
            "CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "profile_id TEXT NOT NULL, "
            "tweet_id TEXT NOT NULL, "
            "username TEXT NOT NULL, "
            "reply_text TEXT, "
            "replied_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
            "UNIQUE(profile_id, tweet_id), "
            "FOREIGN KEY (profile_id) REFERENCES %2 (profile_id) ON DELETE CASCADE"
            ")"
        ).arg(REPLIED_TWEETS_TABLE, ACCOUNTS_TABLE),
        
        QString(
            "CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
            "level TEXT NOT NULL, "
            "module TEXT NOT NULL, "
            "account TEXT, "
            "message TEXT NOT NULL, "
            "details TEXT"
            ")"
        ).arg(LOGS_TABLE)
    };
    
    for (const QString& query : createQueries) {
        if (!executeQuery(query)) {
            return false;
        }
    }
    
    // Create indexes for better performance
    QStringList indexQueries = {
        QString("CREATE INDEX IF NOT EXISTS idx_logs_timestamp ON %1 (timestamp)").arg(LOGS_TABLE),
        QString("CREATE INDEX IF NOT EXISTS idx_logs_level ON %1 (level)").arg(LOGS_TABLE),
        QString("CREATE INDEX IF NOT EXISTS idx_logs_account ON %1 (account)").arg(LOGS_TABLE),
        QString("CREATE INDEX IF NOT EXISTS idx_replied_tweets_profile ON %1 (profile_id)").arg(REPLIED_TWEETS_TABLE),
        QString("CREATE INDEX IF NOT EXISTS idx_replied_tweets_tweet ON %1 (tweet_id)").arg(REPLIED_TWEETS_TABLE)
    };
    
    for (const QString& query : indexQueries) {
        executeQuery(query);
    }
    
    return true;
}

bool DatabaseManager::executeQuery(const QString& query, const QVariantList& bindings)
{
    QSqlQuery sqlQuery(m_database);
    sqlQuery.prepare(query);
    
    for (int i = 0; i < bindings.size(); ++i) {
        sqlQuery.bindValue(i, bindings[i]);
    }
    
    if (!sqlQuery.exec()) {
        qCritical() << "Query failed:" << query;
        qCritical() << "Error:" << sqlQuery.lastError().text();
        handleDatabaseError(sqlQuery.lastError());
        return false;
    }
    
    return true;
}

QSqlQuery DatabaseManager::prepareQuery(const QString& query)
{
    QSqlQuery sqlQuery(m_database);
    sqlQuery.prepare(query);
    return sqlQuery;
}

QString DatabaseManager::generateProfileId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool DatabaseManager::createAccount(const QVariantMap& accountData)
{
    QMutexLocker locker(&m_mutex);
    
    QString profileId = accountData.value("profile_id", generateProfileId()).toString();
    
    QString query = QString(
        "INSERT INTO %1 (profile_id, name, username, password, email, phone, "
        "debug_port, profile_path, use_proxy, proxy_url, use_gemini, gemini_key, chatgpt_key) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    ).arg(ACCOUNTS_TABLE);
    
    QVariantList bindings = {
        profileId,
        accountData.value("name"),
        accountData.value("username"),
        accountData.value("password"),
        accountData.value("email"),
        accountData.value("phone"),
        accountData.value("debug_port", 9222),
        accountData.value("profile_path"),
        accountData.value("use_proxy", false),
        accountData.value("proxy_url"),
        accountData.value("use_gemini", true),
        accountData.value("gemini_key"),
        accountData.value("chatgpt_key")
    };
    
    if (!executeQuery(query, bindings)) {
        return false;
    }
    
    // Create default settings for the account
    QVariantMap defaultSettings;
    defaultSettings["profile_id"] = profileId;
    saveSettings(profileId, defaultSettings);
    
    // Create default stats for the account
    QString statsQuery = QString(
        "INSERT INTO %1 (profile_id) VALUES (?)"
    ).arg(STATS_TABLE);
    executeQuery(statsQuery, {profileId});
    
    return true;
}

bool DatabaseManager::updateAccount(const QString& profileId, const QVariantMap& accountData)
{
    QMutexLocker locker(&m_mutex);
    
    QString query = QString(
        "UPDATE %1 SET name = ?, username = ?, password = ?, email = ?, phone = ?, "
        "debug_port = ?, profile_path = ?, use_proxy = ?, proxy_url = ?, "
        "use_gemini = ?, gemini_key = ?, chatgpt_key = ?, updated_at = CURRENT_TIMESTAMP "
        "WHERE profile_id = ?"
    ).arg(ACCOUNTS_TABLE);
    
    QVariantList bindings = {
        accountData.value("name"),
        accountData.value("username"),
        accountData.value("password"),
        accountData.value("email"),
        accountData.value("phone"),
        accountData.value("debug_port", 9222),
        accountData.value("profile_path"),
        accountData.value("use_proxy", false),
        accountData.value("proxy_url"),
        accountData.value("use_gemini", true),
        accountData.value("gemini_key"),
        accountData.value("chatgpt_key"),
        profileId
    };
    
    return executeQuery(query, bindings);
}

bool DatabaseManager::deleteAccount(const QString& profileId)
{
    QMutexLocker locker(&m_mutex);
    
    QString query = QString("DELETE FROM %1 WHERE profile_id = ?").arg(ACCOUNTS_TABLE);
    return executeQuery(query, {profileId});
}

QList<QVariantMap> DatabaseManager::getAllAccounts()
{
    QMutexLocker locker(&m_mutex);
    
    QList<QVariantMap> accounts;
    QString query = QString("SELECT * FROM %1 ORDER BY name").arg(ACCOUNTS_TABLE);
    
    QSqlQuery sqlQuery = prepareQuery(query);
    if (!sqlQuery.exec()) {
        qCritical() << "Failed to get accounts:" << sqlQuery.lastError().text();
        return accounts;
    }
    
    while (sqlQuery.next()) {
        QVariantMap account;
        QSqlRecord record = sqlQuery.record();
        
        for (int i = 0; i < record.count(); ++i) {
            account[record.fieldName(i)] = record.value(i);
        }
        
        accounts.append(account);
    }
    
    return accounts;
}

QVariantMap DatabaseManager::getAccount(const QString& profileId)
{
    QMutexLocker locker(&m_mutex);
    
    QString query = QString("SELECT * FROM %1 WHERE profile_id = ?").arg(ACCOUNTS_TABLE);
    QSqlQuery sqlQuery = prepareQuery(query);
    sqlQuery.bindValue(0, profileId);
    
    if (!sqlQuery.exec() || !sqlQuery.next()) {
        return QVariantMap();
    }
    
    QVariantMap account;
    QSqlRecord record = sqlQuery.record();
    
    for (int i = 0; i < record.count(); ++i) {
        account[record.fieldName(i)] = record.value(i);
    }
    
    return account;
}

bool DatabaseManager::saveSettings(const QString& profileId, const QVariantMap& settings)
{
    QMutexLocker locker(&m_mutex);
    
    // Check if settings exist
    QString checkQuery = QString("SELECT profile_id FROM %1 WHERE profile_id = ?").arg(SETTINGS_TABLE);
    QSqlQuery checkSqlQuery = prepareQuery(checkQuery);
    checkSqlQuery.bindValue(0, profileId);
    
    bool exists = checkSqlQuery.exec() && checkSqlQuery.next();
    
    QString query;
    QVariantList bindings;
    
    if (exists) {
        query = QString(
            "UPDATE %1 SET max_replies = ?, min_views = ?, skip_replies = ?, skip_retweets = ?, "
            "skip_japanese = ?, auto_like = ?, auto_follow_verified = ?, auto_retweet = ?, "
            "japanese_only = ?, reply_first_only = ?, minimize_window = ?, time_limit_hours = ?, "
            "time_limit_minutes = ?, interval = ?, reply_interval = ?, schedule_enabled = ?, "
            "start_time = ?, end_time = ?, schedule_days = ?, mode_id = ?, target_keywords = ?, "
            "updated_at = CURRENT_TIMESTAMP WHERE profile_id = ?"
        ).arg(SETTINGS_TABLE);
        
        bindings = {
            settings.value("max_replies", TwitterConstants::DEFAULT_MAX_REPLIES),
            settings.value("min_views", TwitterConstants::DEFAULT_MIN_VIEWS),
            settings.value("skip_replies", true),
            settings.value("skip_retweets", true),
            settings.value("skip_japanese", false),
            settings.value("auto_like", true),
            settings.value("auto_follow_verified", false),
            settings.value("auto_retweet", false),
            settings.value("japanese_only", false),
            settings.value("reply_first_only", false),
            settings.value("minimize_window", false),
            settings.value("time_limit_hours", TwitterConstants::DEFAULT_TIME_LIMIT_HOURS),
            settings.value("time_limit_minutes", TwitterConstants::DEFAULT_TIME_LIMIT_MINUTES),
            settings.value("interval", TwitterConstants::DEFAULT_INTERVAL),
            settings.value("reply_interval", TwitterConstants::DEFAULT_REPLY_INTERVAL),
            settings.value("schedule_enabled", false),
            settings.value("start_time", "09:00"),
            settings.value("end_time", "17:00"),
            settings.value("schedule_days", "0,1,2,3,4,5,6"),
            settings.value("mode_id", static_cast<int>(TwitterConstants::BotMode::FEED_MODE)),
            settings.value("target_keywords", ""),
            profileId
        };
    } else {
        query = QString(
            "INSERT INTO %1 (profile_id, max_replies, min_views, skip_replies, skip_retweets, "
            "skip_japanese, auto_like, auto_follow_verified, auto_retweet, japanese_only, "
            "reply_first_only, minimize_window, time_limit_hours, time_limit_minutes, "
            "interval, reply_interval, schedule_enabled, start_time, end_time, schedule_days, "
            "mode_id, target_keywords) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
        ).arg(SETTINGS_TABLE);
        
        bindings = {
            profileId,
            settings.value("max_replies", TwitterConstants::DEFAULT_MAX_REPLIES),
            settings.value("min_views", TwitterConstants::DEFAULT_MIN_VIEWS),
            settings.value("skip_replies", true),
            settings.value("skip_retweets", true),
            settings.value("skip_japanese", false),
            settings.value("auto_like", true),
            settings.value("auto_follow_verified", false),
            settings.value("auto_retweet", false),
            settings.value("japanese_only", false),
            settings.value("reply_first_only", false),
            settings.value("minimize_window", false),
            settings.value("time_limit_hours", TwitterConstants::DEFAULT_TIME_LIMIT_HOURS),
            settings.value("time_limit_minutes", TwitterConstants::DEFAULT_TIME_LIMIT_MINUTES),
            settings.value("interval", TwitterConstants::DEFAULT_INTERVAL),
            settings.value("reply_interval", TwitterConstants::DEFAULT_REPLY_INTERVAL),
            settings.value("schedule_enabled", false),
            settings.value("start_time", "09:00"),
            settings.value("end_time", "17:00"),
            settings.value("schedule_days", "0,1,2,3,4,5,6"),
            settings.value("mode_id", static_cast<int>(TwitterConstants::BotMode::FEED_MODE)),
            settings.value("target_keywords", "")
        };
    }
    
    return executeQuery(query, bindings);
}

QVariantMap DatabaseManager::getSettings(const QString& profileId)
{
    QMutexLocker locker(&m_mutex);
    
    QString query = QString("SELECT * FROM %1 WHERE profile_id = ?").arg(SETTINGS_TABLE);
    QSqlQuery sqlQuery = prepareQuery(query);
    sqlQuery.bindValue(0, profileId);
    
    if (!sqlQuery.exec() || !sqlQuery.next()) {
        return QVariantMap();
    }
    
    QVariantMap settings;
    QSqlRecord record = sqlQuery.record();
    
    for (int i = 0; i < record.count(); ++i) {
        settings[record.fieldName(i)] = record.value(i);
    }
    
    return settings;
}

bool DatabaseManager::deleteSettings(const QString& profileId)
{
    QMutexLocker locker(&m_mutex);
    
    QString query = QString("DELETE FROM %1 WHERE profile_id = ?").arg(SETTINGS_TABLE);
    return executeQuery(query, {profileId});
}

QVariantMap DatabaseManager::getAccountStats(const QString& profileId)
{
    QMutexLocker locker(&m_mutex);
    
    QString query = QString("SELECT * FROM %1 WHERE profile_id = ?").arg(STATS_TABLE);
    QSqlQuery sqlQuery = prepareQuery(query);
    sqlQuery.bindValue(0, profileId);
    
    if (!sqlQuery.exec() || !sqlQuery.next()) {
        return QVariantMap();
    }
    
    QVariantMap stats;
    QSqlRecord record = sqlQuery.record();
    
    for (int i = 0; i < record.count(); ++i) {
        stats[record.fieldName(i)] = record.value(i);
    }
    
    return stats;
}

bool DatabaseManager::updateAccountStats(const QString& profileId, const QVariantMap& stats)
{
    QMutexLocker locker(&m_mutex);
    
    QString query = QString(
        "UPDATE %1 SET replies_sent = ?, likes_given = ?, follows_made = ?, retweets = ?, "
        "last_activity = CURRENT_TIMESTAMP, total_runtime_minutes = ?, success_rate = ?, "
        "updated_at = CURRENT_TIMESTAMP WHERE profile_id = ?"
    ).arg(STATS_TABLE);
    
    QVariantList bindings = {
        stats.value("replies_sent", 0),
        stats.value("likes_given", 0),
        stats.value("follows_made", 0),
        stats.value("retweets", 0),
        stats.value("total_runtime_minutes", 0),
        stats.value("success_rate", 0.0),
        profileId
    };
    
    return executeQuery(query, bindings);
}

bool DatabaseManager::addRepliedTweet(const QString& profileId, const QString& tweetId, 
                                     const QString& username, const QString& replyText)
{
    QMutexLocker locker(&m_mutex);
    
    QString query = QString(
        "INSERT OR REPLACE INTO %1 (profile_id, tweet_id, username, reply_text) VALUES (?, ?, ?, ?)"
    ).arg(REPLIED_TWEETS_TABLE);
    
    QVariantList bindings = {profileId, tweetId, username, replyText};
    return executeQuery(query, bindings);
}

QList<QVariantMap> DatabaseManager::getRepliedTweets(const QString& profileId, int limit)
{
    QMutexLocker locker(&m_mutex);
    
    QList<QVariantMap> tweets;
    QString query;
    QVariantList bindings;
    
    if (profileId.isEmpty()) {
        query = QString("SELECT * FROM %1 ORDER BY replied_at DESC LIMIT ?").arg(REPLIED_TWEETS_TABLE);
        bindings = {limit};
    } else {
        query = QString("SELECT * FROM %1 WHERE profile_id = ? ORDER BY replied_at DESC LIMIT ?").arg(REPLIED_TWEETS_TABLE);
        bindings = {profileId, limit};
    }
    
    QSqlQuery sqlQuery = prepareQuery(query);
    for (int i = 0; i < bindings.size(); ++i) {
        sqlQuery.bindValue(i, bindings[i]);
    }
    
    if (!sqlQuery.exec()) {
        return tweets;
    }
    
    while (sqlQuery.next()) {
        QVariantMap tweet;
        QSqlRecord record = sqlQuery.record();
        
        for (int i = 0; i < record.count(); ++i) {
            tweet[record.fieldName(i)] = record.value(i);
        }
        
        tweets.append(tweet);
    }
    
    return tweets;
}

bool DatabaseManager::hasRepliedToTweet(const QString& profileId, const QString& tweetId)
{
    QMutexLocker locker(&m_mutex);
    
    QString query = QString("SELECT 1 FROM %1 WHERE profile_id = ? AND tweet_id = ?").arg(REPLIED_TWEETS_TABLE);
    QSqlQuery sqlQuery = prepareQuery(query);
    sqlQuery.bindValue(0, profileId);
    sqlQuery.bindValue(1, tweetId);
    
    return sqlQuery.exec() && sqlQuery.next();
}

bool DatabaseManager::addLog(const QString& level, const QString& module, 
                            const QString& account, const QString& message, 
                            const QJsonObject& details)
{
    QMutexLocker locker(&m_mutex);
    
    QString query = QString(
        "INSERT INTO %1 (level, module, account, message, details) VALUES (?, ?, ?, ?, ?)"
    ).arg(LOGS_TABLE);
    
    QString detailsJson = details.isEmpty() ? QString() : QJsonDocument(details).toJson(QJsonDocument::Compact);
    
    QVariantList bindings = {level, module, account, message, detailsJson};
    return executeQuery(query, bindings);
}

QList<QVariantMap> DatabaseManager::getLogs(int limit)
{
    QMutexLocker locker(&m_mutex);
    
    QList<QVariantMap> logs;
    QString query = QString("SELECT * FROM %1 ORDER BY timestamp DESC LIMIT ?").arg(LOGS_TABLE);
    
    QSqlQuery sqlQuery = prepareQuery(query);
    sqlQuery.bindValue(0, limit);
    
    if (!sqlQuery.exec()) {
        return logs;
    }
    
    while (sqlQuery.next()) {
        QVariantMap log;
        QSqlRecord record = sqlQuery.record();
        
        for (int i = 0; i < record.count(); ++i) {
            log[record.fieldName(i)] = record.value(i);
        }
        
        logs.append(log);
    }
    
    return logs;
}

QList<QVariantMap> DatabaseManager::getFilteredLogs(const QString& module,
                                                    const QString& level,
                                                    const QString& account,
                                                    const QString& dateRange,
                                                    const QString& searchText)
{
    QMutexLocker locker(&m_mutex);
    
    QList<QVariantMap> logs;
    QString query = QString("SELECT * FROM %1 WHERE 1=1").arg(LOGS_TABLE);
    QVariantList bindings;
    
    if (!module.isEmpty() && module != "All Modules") {
        query += " AND module = ?";
        bindings.append(module);
    }
    
    if (!level.isEmpty() && level != "All Levels") {
        query += " AND level = ?";
        bindings.append(level);
    }
    
    if (!account.isEmpty() && account != "All Accounts") {
        QString cleanAccount = account.startsWith("@") ? account.mid(1) : account;
        query += " AND account = ?";
        bindings.append(cleanAccount);
    }
    
    if (!dateRange.isEmpty()) {
        QDateTime cutoff = QDateTime::currentDateTime();
        if (dateRange == "Last Hour") {
            cutoff = cutoff.addSecs(-3600);
        } else if (dateRange == "Last 24 Hours") {
            cutoff = cutoff.addDays(-1);
        } else if (dateRange == "Last 7 Days") {
            cutoff = cutoff.addDays(-7);
        } else if (dateRange == "Last 30 Days") {
            cutoff = cutoff.addDays(-30);
        }
        
        query += " AND timestamp >= ?";
        bindings.append(cutoff);
    }
    
    if (!searchText.isEmpty()) {
        query += " AND message LIKE ?";
        bindings.append(QString("%%1%").arg(searchText));
    }
    
    query += " ORDER BY timestamp DESC LIMIT 1000";
    
    QSqlQuery sqlQuery = prepareQuery(query);
    for (int i = 0; i < bindings.size(); ++i) {
        sqlQuery.bindValue(i, bindings[i]);
    }
    
    if (!sqlQuery.exec()) {
        return logs;
    }
    
    while (sqlQuery.next()) {
        QVariantMap log;
        QSqlRecord record = sqlQuery.record();
        
        for (int i = 0; i < record.count(); ++i) {
            log[record.fieldName(i)] = record.value(i);
        }
        
        logs.append(log);
    }
    
    return logs;
}

bool DatabaseManager::clearLogs()
{
    QMutexLocker locker(&m_mutex);
    
    QString query = QString("DELETE FROM %1").arg(LOGS_TABLE);
    return executeQuery(query);
}

bool DatabaseManager::exportLogs(const QString& filePath, const QString& module,
                                const QString& level, const QString& account,
                                const QString& dateRange)
{
    // Implementation would export logs to CSV/JSON format
    // This is a simplified version
    QList<QVariantMap> logs = getFilteredLogs(module, level, account, dateRange, QString());
    
    // For now, just return true - actual implementation would write to file
    Q_UNUSED(filePath)
    Q_UNUSED(logs)
    return true;
}

bool DatabaseManager::backupDatabase(const QString& backupPath)
{
    QMutexLocker locker(&m_mutex);
    
    return QFile::copy(m_databasePath, backupPath);
}

bool DatabaseManager::restoreDatabase(const QString& backupPath)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_database.isOpen()) {
        m_database.close();
    }
    
    QFile::remove(m_databasePath);
    bool success = QFile::copy(backupPath, m_databasePath);
    
    if (success) {
        initializeDatabase();
    }
    
    return success;
}

void DatabaseManager::handleDatabaseError(const QSqlError& error)
{
    QString errorMsg = QString("Database error: %1").arg(error.text());
    qCritical() << errorMsg;
    
    if (m_mainWindow) {
        QMessageBox::critical(m_mainWindow, "Database Error", errorMsg);
    }
}
