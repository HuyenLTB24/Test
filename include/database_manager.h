#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantMap>
#include <QStringList>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMutex>
#include "constants.h"

class QWidget;

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QWidget* mainWindow = nullptr, QObject* parent = nullptr);
    ~DatabaseManager();

    // Account management
    bool createAccount(const QVariantMap& accountData);
    bool updateAccount(const QString& profileId, const QVariantMap& accountData);
    bool deleteAccount(const QString& profileId);
    QList<QVariantMap> getAllAccounts();
    QVariantMap getAccount(const QString& profileId);
    
    // Settings management
    bool saveSettings(const QString& profileId, const QVariantMap& settings);
    QVariantMap getSettings(const QString& profileId);
    bool deleteSettings(const QString& profileId);
    
    // Statistics
    QVariantMap getAccountStats(const QString& profileId);
    bool updateAccountStats(const QString& profileId, const QVariantMap& stats);
    
    // Replied tweets tracking
    bool addRepliedTweet(const QString& profileId, const QString& tweetId, 
                        const QString& username, const QString& replyText);
    QList<QVariantMap> getRepliedTweets(const QString& profileId = QString(), int limit = 100);
    bool hasRepliedToTweet(const QString& profileId, const QString& tweetId);
    
    // Logging
    bool addLog(const QString& level, const QString& module, 
               const QString& account, const QString& message, 
               const QJsonObject& details = QJsonObject());
    QList<QVariantMap> getLogs(int limit = 1000);
    QList<QVariantMap> getFilteredLogs(const QString& module = QString(),
                                      const QString& level = QString(),
                                      const QString& account = QString(),
                                      const QString& dateRange = QString(),
                                      const QString& searchText = QString());
    bool clearLogs();
    bool exportLogs(const QString& filePath, const QString& module = QString(),
                   const QString& level = QString(), const QString& account = QString(),
                   const QString& dateRange = QString());
    
    // Database maintenance
    bool initializeDatabase();
    bool backupDatabase(const QString& backupPath);
    bool restoreDatabase(const QString& backupPath);
    
private slots:
    void handleDatabaseError(const QSqlError& error);

private:
    bool createTables();
    bool executeQuery(const QString& query, const QVariantList& bindings = QVariantList());
    QSqlQuery prepareQuery(const QString& query);
    QString generateProfileId();
    
    QSqlDatabase m_database;
    QWidget* m_mainWindow;
    QMutex m_mutex;
    QString m_databasePath;
    
    static const QString DATABASE_NAME;
    static const QString ACCOUNTS_TABLE;
    static const QString SETTINGS_TABLE;
    static const QString STATS_TABLE;
    static const QString REPLIED_TWEETS_TABLE;
    static const QString LOGS_TABLE;
};

#endif // DATABASE_MANAGER_H
