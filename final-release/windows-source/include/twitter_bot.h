#ifndef TWITTER_BOT_H
#define TWITTER_BOT_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVariantMap>
#include <QStringList>
#include <QMutex>
#include <QThread>
#include <QQueue>
#include <QCache>
#include <memory>
#include "constants.h"

class DatabaseManager;
class QWidget;

struct TweetStats {
    QString tweetId;
    QString username;
    QString content;
    int responseTimeMs = 0;
    int characterCount = 0;
    bool isVerified = false;
    bool hasMedia = false;
    int interactionCount = 0;
};

class TwitterBot : public QObject
{
    Q_OBJECT

public:
    explicit TwitterBot(const QVariantMap& credentials, QWidget* mainWindow = nullptr, QObject* parent = nullptr);
    ~TwitterBot();

    // Bot control
    void start();
    void stop();
    void pause();
    void resume();
    
    // Settings
    void updateSettings(const QVariantMap& settings);
    QVariantMap getSettings() const;
    
    // Status
    bool isRunning() const { return m_isRunning; }
    bool isPaused() const { return m_isPaused; }
    QString getUsername() const { return m_username; }
    QString getProfileId() const { return m_profileId; }
    TwitterConstants::BotMode getMode() const { return m_mode; }
    void setMode(TwitterConstants::BotMode mode) { m_mode = mode; }

public slots:
    void onTweetProcessed(const QString& timeStr, const QString& username, 
                         const QString& tweetUrl, const QString& replyText,
                         const QString& status, bool likeSuccess, bool followSuccess, 
                         bool retweetSuccess, int replyCount, int viewCount,
                         const QString& tweetId, const QString& originalContent);

signals:
    void tweetProcessedSignal(const QString& timeStr, const QString& username, 
                             const QString& tweetUrl, const QString& replyText,
                             const QString& status, bool likeSuccess, bool followSuccess, 
                             bool retweetSuccess, int replyCount, int viewCount,
                             const QString& tweetId, const QString& originalContent);
    void logSignal(const QString& message);
    void statusSignal(const QString& status);
    void errorSignal(const QString& error);
    void stopSignal();

private slots:
    void processNextTweet();
    void handleNetworkReply();
    void handlePageLoadFinished(bool ok);
    void onTimerTimeout();

private:
    // Core functionality
    bool setupWebEngine();
    bool checkLoggedIn();
    bool login();
    void cleanup();
    
    // Tweet processing
    bool processTweet(const QJsonObject& tweet);
    bool replyToTweet(const QJsonObject& tweet);
    bool likeTweet(const QString& tweetId);
    bool followUser(const QString& username);
    bool retweetTweet(const QString& tweetId);
    
    // Feed monitoring
    void monitorFeed();
    QList<QJsonObject> getRecentTweets(int maxTweets = 50);
    bool processTrendingTweet(const QJsonObject& tweet);
    
    // AI integration
    QString generateAIResponse(const QString& tweetText);
    QString generateGeminiResponse(const QString& tweetText);
    QString generateChatGPTResponse(const QString& tweetText);
    
    // Tweet analysis
    bool isOwnTweet(const QJsonObject& tweet);
    bool isReplyTweet(const QJsonObject& tweet);
    bool shouldSkipTweet(const QJsonObject& tweet);
    QString getTweetId(const QJsonObject& tweet);
    QString getTweetTimestamp(const QJsonObject& tweet);
    
    // Text processing
    QString cleanText(const QString& text);
    QString cleanHashtags(const QString& text);
    
    // Utilities
    void log(const QString& message);
    void errorLog(const QString& message);
    bool waitForPageLoad(int timeoutMs = 10000);
    void minimizeWindow();
    bool ensureConnection();
    
    // Settings management
    void loadInitialSettings();
    bool updateSettingsFromDB();
    
    // Cache management
    void clearApiCache();
    bool isApiResponseCached(const QString& key);
    QString getCachedApiResponse(const QString& key);
    void cacheApiResponse(const QString& key, const QString& response);

private:
    DatabaseManager* m_database;
    QWidget* m_mainWindow;
    QWebEngineView* m_webView;
    QWebEnginePage* m_webPage;
    QNetworkAccessManager* m_networkManager;
    QTimer* m_processTimer;
    QTimer* m_feedTimer;
    
    // Bot state
    bool m_isRunning;
    bool m_isPaused;
    bool m_shouldStop;
    bool m_forceStop;
    QString m_username;
    QString m_profileId;
    TwitterConstants::BotMode m_mode;
    
    // API settings
    bool m_useGemini;
    QString m_geminiKey;
    QString m_chatgptKey;
    
    // Filter settings
    QVariantMap m_filterSettings;
    int m_timeLimitHours;
    int m_timeLimitMinutes;
    int m_interval;
    int m_replyInterval;
    bool m_shouldMinimize;
    bool m_scheduleEnabled;
    QString m_startTime;
    QString m_endTime;
    QStringList m_scheduleDays;
    
    // Target users for user mode
    QStringList m_targetUsers;
    QSet<QString> m_repliedUsers;
    
    // Caches
    QCache<QString, QString> m_apiCache;
    QSet<QString> m_repliedTweets;
    QQueue<QJsonObject> m_tweetQueue;
    
    // Thread safety
    QMutex m_mutex;
    
    // Constants
    static const int API_CACHE_TTL_SECONDS = 300;
    static const int MAX_RETRIES = 3;
    static const int DEFAULT_TIMEOUT_MS = 10000;
};

#endif // TWITTER_BOT_H
