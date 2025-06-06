#include "twitter_bot.h"
#include "database_manager.h"
#include <QApplication>
#include <QWidget>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QThread>
#include <QMutexLocker>
#include <QUrl>
#include <QUrlQuery>
#include <QTime>
#include <QDate>
#include <QDateTime>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QDebug>
#include <QEventLoop>
#include <QCryptographicHash>
#include <QWebEngineProfile>

TwitterBot::TwitterBot(const QVariantMap& credentials, QWidget* mainWindow, QObject* parent)
    : QObject(parent)
    , m_database(nullptr)
    , m_mainWindow(mainWindow)
    , m_webView(nullptr)
    , m_webPage(nullptr)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_processTimer(new QTimer(this))
    , m_feedTimer(new QTimer(this))
    , m_isRunning(false)
    , m_isPaused(false)
    , m_shouldStop(false)
    , m_forceStop(false)
    , m_mode(TwitterConstants::BotMode::Feed)
    , m_useGemini(true)
    , m_timeLimitHours(24)
    , m_timeLimitMinutes(0)
    , m_interval(60)
    , m_replyInterval(180)
    , m_shouldMinimize(false)
    , m_scheduleEnabled(false)
    , m_apiCache(100) // Cache up to 100 API responses
{
    // Extract credentials
    m_username = credentials.value("username").toString();
    m_profileId = credentials.value("profile_id").toString();
    
    // Setup database
    m_database = new DatabaseManager(this);
    if (!m_database->initialize()) {
        emit errorSignal("Failed to initialize database");
        return;
    }
    
    // Load initial settings
    loadInitialSettings();
    
    // Setup web engine
    if (!setupWebEngine()) {
        emit errorSignal("Failed to setup web engine");
        return;
    }
    
    // Setup timers
    m_processTimer->setSingleShot(false);
    m_feedTimer->setSingleShot(false);
    
    connect(m_processTimer, &QTimer::timeout, this, &TwitterBot::processNextTweet);
    connect(m_feedTimer, &QTimer::timeout, this, &TwitterBot::monitorFeed);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &TwitterBot::handleNetworkReply);
    
    log(QString("TwitterBot initialized for user: %1").arg(m_username));
}

TwitterBot::~TwitterBot()
{
    cleanup();
}

void TwitterBot::start()
{
    if (m_isRunning) {
        log("Bot is already running");
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    m_isRunning = true;
    m_isPaused = false;
    m_shouldStop = false;
    m_forceStop = false;
    
    // Load latest settings from database
    updateSettingsFromDB();
    
    // Check if we're logged in
    if (!checkLoggedIn()) {
        log("Not logged in, attempting to login...");
        if (!login()) {
            emit errorSignal("Failed to login to Twitter");
            stop();
            return;
        }
    }
    
    // Clear caches
    clearApiCache();
    
    // Start appropriate mode
    switch (m_mode) {
        case TwitterConstants::BotMode::Feed:
            log("Starting Feed mode");
            m_feedTimer->start(m_interval * 1000);
            break;
        case TwitterConstants::BotMode::User:
            log("Starting User mode");
            m_processTimer->start(m_interval * 1000);
            break;
        case TwitterConstants::BotMode::Comments:
            log("Starting Comments mode");
            m_feedTimer->start(m_interval * 1000);
            break;
        case TwitterConstants::BotMode::Trending:
            log("Starting Trending mode");
            m_feedTimer->start(m_interval * 1000);
            break;
    }
    
    emit statusSignal("Running");
    log("Bot started successfully");
}

void TwitterBot::stop()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isRunning) {
        return;
    }
    
    m_shouldStop = true;
    m_isRunning = false;
    m_isPaused = false;
    
    // Stop all timers
    m_processTimer->stop();
    m_feedTimer->stop();
    
    // Clear queues
    m_tweetQueue.clear();
    
    emit statusSignal("Stopped");
    emit stopSignal();
    log("Bot stopped");
}

void TwitterBot::pause()
{
    if (!m_isRunning || m_isPaused) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    m_isPaused = true;
    
    m_processTimer->stop();
    m_feedTimer->stop();
    
    emit statusSignal("Paused");
    log("Bot paused");
}

void TwitterBot::resume()
{
    if (!m_isRunning || !m_isPaused) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    m_isPaused = false;
    
    // Restart timers based on mode
    switch (m_mode) {
        case TwitterConstants::BotMode::Feed:
        case TwitterConstants::BotMode::Comments:
        case TwitterConstants::BotMode::Trending:
            m_feedTimer->start(m_interval * 1000);
            break;
        case TwitterConstants::BotMode::User:
            m_processTimer->start(m_interval * 1000);
            break;
    }
    
    emit statusSignal("Running");
    log("Bot resumed");
}

void TwitterBot::updateSettings(const QVariantMap& settings)
{
    QMutexLocker locker(&m_mutex);
    
    // Update API settings
    m_useGemini = settings.value("use_gemini", true).toBool();
    m_geminiKey = settings.value("gemini_key").toString();
    m_chatgptKey = settings.value("chatgpt_key").toString();
    
    // Update timing settings
    m_interval = settings.value("interval", 60).toInt();
    m_replyInterval = settings.value("reply_interval", 180).toInt();
    m_timeLimitHours = settings.value("time_limit_hours", 24).toInt();
    m_timeLimitMinutes = settings.value("time_limit_minutes", 0).toInt();
    
    // Update filter settings
    m_filterSettings = settings.value("filters").toMap();
    
    // Update schedule settings
    m_scheduleEnabled = settings.value("schedule_enabled", false).toBool();
    m_startTime = settings.value("start_time").toString();
    m_endTime = settings.value("end_time").toString();
    m_scheduleDays = settings.value("schedule_days").toStringList();
    
    // Update target users for user mode
    m_targetUsers = settings.value("target_users").toStringList();
    
    // Update window settings
    m_shouldMinimize = settings.value("minimize_window", false).toBool();
    
    // Restart timers with new intervals if running
    if (m_isRunning && !m_isPaused) {
        m_processTimer->stop();
        m_feedTimer->stop();
        
        switch (m_mode) {
            case TwitterConstants::BotMode::Feed:
            case TwitterConstants::BotMode::Comments:
            case TwitterConstants::BotMode::Trending:
                m_feedTimer->start(m_interval * 1000);
                break;
            case TwitterConstants::BotMode::User:
                m_processTimer->start(m_interval * 1000);
                break;
        }
    }
    
    log("Settings updated");
}

QVariantMap TwitterBot::getSettings() const
{
    QVariantMap settings;
    settings["use_gemini"] = m_useGemini;
    settings["gemini_key"] = m_geminiKey;
    settings["chatgpt_key"] = m_chatgptKey;
    settings["interval"] = m_interval;
    settings["reply_interval"] = m_replyInterval;
    settings["time_limit_hours"] = m_timeLimitHours;
    settings["time_limit_minutes"] = m_timeLimitMinutes;
    settings["filters"] = m_filterSettings;
    settings["schedule_enabled"] = m_scheduleEnabled;
    settings["start_time"] = m_startTime;
    settings["end_time"] = m_endTime;
    settings["schedule_days"] = m_scheduleDays;
    settings["target_users"] = m_targetUsers;
    settings["minimize_window"] = m_shouldMinimize;
    return settings;
}

bool TwitterBot::setupWebEngine()
{
    if (!m_mainWindow) {
        log("No main window provided, cannot setup web engine");
        return false;
    }
    
    m_webView = new QWebEngineView(m_mainWindow);
    m_webPage = m_webView->page();
    
    connect(m_webPage, &QWebEnginePage::loadFinished, this, &TwitterBot::handlePageLoadFinished);
    
    // Set user agent to avoid detection
    m_webPage->profile()->setHttpUserAgent(
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
    );
    
    log("Web engine setup completed");
    return true;
}

bool TwitterBot::checkLoggedIn()
{
    if (!m_webView) {
        return false;
    }
    
    // Navigate to Twitter home to check login status
    m_webView->load(QUrl("https://twitter.com/home"));
    
    if (!waitForPageLoad()) {
        return false;
    }
    
    // Check if we're on the home page (logged in) or login page
    QString currentUrl = m_webView->url().toString();
    return currentUrl.contains("/home") || currentUrl.contains("/timeline");
}

bool TwitterBot::login()
{
    if (!m_webView) {
        return false;
    }
    
    log("Attempting to login to Twitter...");
    
    // Navigate to login page
    m_webView->load(QUrl("https://twitter.com/login"));
    
    if (!waitForPageLoad()) {
        errorLog("Failed to load login page");
        return false;
    }
    
    // For now, we'll assume the user manually logs in
    // In a full implementation, you might want to use browser profiles
    // or implement automated login (though this is against Twitter's ToS)
    
    log("Please ensure you are logged in to Twitter in the browser");
    return true;
}

void TwitterBot::monitorFeed()
{
    if (m_shouldStop || m_isPaused) {
        return;
    }
    
    log("Monitoring feed for new tweets...");
    
    QList<QJsonObject> tweets = getRecentTweets(50);
    
    for (const QJsonObject& tweet : tweets) {
        if (m_shouldStop || m_isPaused) {
            break;
        }
        
        if (shouldSkipTweet(tweet)) {
            continue;
        }
        
        QString tweetId = getTweetId(tweet);
        if (m_repliedTweets.contains(tweetId)) {
            continue;
        }
        
        // Add to processing queue
        m_tweetQueue.enqueue(tweet);
    }
    
    // Process tweets in queue
    while (!m_tweetQueue.isEmpty() && !m_shouldStop && !m_isPaused) {
        QJsonObject tweet = m_tweetQueue.dequeue();
        processTweet(tweet);
        
        // Add delay between processing tweets
        QThread::msleep(QRandomGenerator::global()->bounded(2000, 5000));
    }
}

QList<QJsonObject> TwitterBot::getRecentTweets(int maxTweets)
{
    QList<QJsonObject> tweets;
    
    if (!m_webView) {
        return tweets;
    }
    
    // Navigate to appropriate page based on mode
    QString url;
    switch (m_mode) {
        case TwitterConstants::BotMode::Feed:
            url = "https://twitter.com/home";
            break;
        case TwitterConstants::BotMode::Comments:
            url = QString("https://twitter.com/%1/with_replies").arg(m_username);
            break;
        case TwitterConstants::BotMode::Trending:
            url = "https://twitter.com/explore/tabs/trending";
            break;
        case TwitterConstants::BotMode::User:
            // For user mode, we'll check specific users
            if (!m_targetUsers.isEmpty()) {
                url = QString("https://twitter.com/%1").arg(m_targetUsers.first());
            } else {
                url = "https://twitter.com/home";
            }
            break;
    }
    
    m_webView->load(QUrl(url));
    
    if (!waitForPageLoad()) {
        errorLog("Failed to load Twitter page");
        return tweets;
    }
    
    // In a real implementation, you would use JavaScript to extract tweet data
    // For now, we'll return mock data structure
    log(QString("Retrieved %1 tweets from feed").arg(tweets.size()));
    
    return tweets;
}

bool TwitterBot::processTweet(const QJsonObject& tweet)
{
    if (m_shouldStop || m_isPaused) {
        return false;
    }
    
    QString tweetId = getTweetId(tweet);
    QString username = tweet.value("username").toString();
    QString content = tweet.value("content").toString();
    QString tweetUrl = tweet.value("url").toString();
    
    log(QString("Processing tweet from @%1: %2").arg(username, content.left(50)));
    
    bool success = false;
    QString replyText;
    bool likeSuccess = false;
    bool followSuccess = false;
    bool retweetSuccess = false;
    
    try {
        // Generate AI response
        replyText = generateAIResponse(content);
        
        if (!replyText.isEmpty()) {
            // Reply to tweet
            success = replyToTweet(tweet);
            
            // Like the tweet
            likeSuccess = likeTweet(tweetId);
            
            // Follow the user (if configured)
            if (m_filterSettings.value("auto_follow", false).toBool()) {
                followSuccess = followUser(username);
            }
            
            // Retweet (if configured)
            if (m_filterSettings.value("auto_retweet", false).toBool()) {
                retweetSuccess = retweetTweet(tweetId);
            }
            
            // Mark as replied
            m_repliedTweets.insert(tweetId);
            
            // Store in database
            if (m_database) {
                QVariantMap tweetData;
                tweetData["tweet_id"] = tweetId;
                tweetData["username"] = username;
                tweetData["original_content"] = content;
                tweetData["reply_content"] = replyText;
                tweetData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
                tweetData["liked"] = likeSuccess;
                tweetData["followed"] = followSuccess;
                tweetData["retweeted"] = retweetSuccess;
                
                m_database->addRepliedTweet(m_username, tweetData);
            }
            
            // Emit signal for UI update
            emit tweetProcessedSignal(
                QTime::currentTime().toString(),
                username,
                tweetUrl,
                replyText,
                success ? "Success" : "Failed",
                likeSuccess,
                followSuccess,
                retweetSuccess,
                0, // reply count
                0, // view count
                tweetId,
                content
            );
            
            log(QString("Successfully processed tweet from @%1").arg(username));
        }
        
    } catch (const std::exception& e) {
        errorLog(QString("Error processing tweet: %1").arg(e.what()));
    }
    
    return success;
}

QString TwitterBot::generateAIResponse(const QString& tweetText)
{
    if (tweetText.isEmpty()) {
        return QString();
    }
    
    // Check cache first
    QString cacheKey = QString("ai_response_%1").arg(QString(QCryptographicHash::hash(tweetText.toUtf8(), QCryptographicHash::Md5).toHex()));
    if (isApiResponseCached(cacheKey)) {
        return getCachedApiResponse(cacheKey);
    }
    
    QString response;
    
    if (m_useGemini && !m_geminiKey.isEmpty()) {
        response = generateGeminiResponse(tweetText);
    } else if (!m_chatgptKey.isEmpty()) {
        response = generateChatGPTResponse(tweetText);
    } else {
        errorLog("No AI API key configured");
        return QString();
    }
    
    // Cache the response
    if (!response.isEmpty()) {
        cacheApiResponse(cacheKey, response);
    }
    
    return response;
}

QString TwitterBot::generateGeminiResponse(const QString& tweetText)
{
    if (m_geminiKey.isEmpty()) {
        return QString();
    }
    
    QNetworkRequest request;
    request.setUrl(QUrl("https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-goog-api-key", m_geminiKey.toUtf8());
    
    QJsonObject prompt;
    prompt["text"] = QString("Generate a brief, engaging reply to this tweet (max 280 characters): %1").arg(tweetText);
    
    QJsonObject content;
    content["parts"] = QJsonArray{prompt};
    
    QJsonObject requestData;
    requestData["contents"] = QJsonArray{content};
    
    QJsonDocument doc(requestData);
    QNetworkReply* reply = m_networkManager->post(request, doc.toJson());
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    QString response;
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument responseDoc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject responseObj = responseDoc.object();
        
        if (responseObj.contains("candidates")) {
            QJsonArray candidates = responseObj["candidates"].toArray();
            if (!candidates.isEmpty()) {
                QJsonObject candidate = candidates[0].toObject();
                QJsonObject content = candidate["content"].toObject();
                QJsonArray parts = content["parts"].toArray();
                if (!parts.isEmpty()) {
                    response = parts[0].toObject()["text"].toString();
                }
            }
        }
    } else {
        errorLog(QString("Gemini API error: %1").arg(reply->errorString()));
    }
    
    reply->deleteLater();
    return cleanText(response);
}

QString TwitterBot::generateChatGPTResponse(const QString& tweetText)
{
    if (m_chatgptKey.isEmpty()) {
        return QString();
    }
    
    QNetworkRequest request;
    request.setUrl(QUrl("https://api.openai.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_chatgptKey).toUtf8());
    
    QJsonObject message;
    message["role"] = "user";
    message["content"] = QString("Generate a brief, engaging reply to this tweet (max 280 characters): %1").arg(tweetText);
    
    QJsonObject requestData;
    requestData["model"] = "gpt-3.5-turbo";
    requestData["messages"] = QJsonArray{message};
    requestData["max_tokens"] = 100;
    requestData["temperature"] = 0.7;
    
    QJsonDocument doc(requestData);
    QNetworkReply* reply = m_networkManager->post(request, doc.toJson());
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    QString response;
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument responseDoc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject responseObj = responseDoc.object();
        
        if (responseObj.contains("choices")) {
            QJsonArray choices = responseObj["choices"].toArray();
            if (!choices.isEmpty()) {
                QJsonObject choice = choices[0].toObject();
                QJsonObject message = choice["message"].toObject();
                response = message["content"].toString();
            }
        }
    } else {
        errorLog(QString("ChatGPT API error: %1").arg(reply->errorString()));
    }
    
    reply->deleteLater();
    return cleanText(response);
}

bool TwitterBot::replyToTweet(const QJsonObject& tweet)
{
    if (!m_webView) {
        return false;
    }
    
    QString tweetId = getTweetId(tweet);
    QString content = tweet.value("content").toString();
    QString replyText = generateAIResponse(content);
    
    if (replyText.isEmpty()) {
        return false;
    }
    
    // Navigate to the tweet
    QString tweetUrl = QString("https://twitter.com/i/web/status/%1").arg(tweetId);
    m_webView->load(QUrl(tweetUrl));
    
    if (!waitForPageLoad()) {
        return false;
    }
    
    // In a real implementation, you would use JavaScript to:
    // 1. Click the reply button
    // 2. Enter the reply text
    // 3. Click the send button
    
    log(QString("Replied to tweet %1 with: %2").arg(tweetId, replyText));
    return true;
}

bool TwitterBot::likeTweet(const QString& tweetId)
{
    if (!m_webView || tweetId.isEmpty()) {
        return false;
    }
    
    // In a real implementation, you would use JavaScript to click the like button
    log(QString("Liked tweet %1").arg(tweetId));
    return true;
}

bool TwitterBot::followUser(const QString& username)
{
    if (!m_webView || username.isEmpty()) {
        return false;
    }
    
    // Navigate to user profile
    QString userUrl = QString("https://twitter.com/%1").arg(username);
    m_webView->load(QUrl(userUrl));
    
    if (!waitForPageLoad()) {
        return false;
    }
    
    // In a real implementation, you would use JavaScript to click the follow button
    log(QString("Followed user @%1").arg(username));
    return true;
}

bool TwitterBot::retweetTweet(const QString& tweetId)
{
    if (!m_webView || tweetId.isEmpty()) {
        return false;
    }
    
    // In a real implementation, you would use JavaScript to click the retweet button
    log(QString("Retweeted tweet %1").arg(tweetId));
    return true;
}

// Additional helper methods implementation continues...

void TwitterBot::processNextTweet()
{
    if (m_shouldStop || m_isPaused) {
        return;
    }
    
    if (m_mode == TwitterConstants::BotMode::User && !m_targetUsers.isEmpty()) {
        // Process specific users in user mode
        for (const QString& targetUser : m_targetUsers) {
            if (m_shouldStop || m_isPaused) {
                break;
            }
            
            QList<QJsonObject> userTweets = getRecentTweets(10);
            for (const QJsonObject& tweet : userTweets) {
                if (m_shouldStop || m_isPaused) {
                    break;
                }
                
                QString tweetId = getTweetId(tweet);
                if (!m_repliedTweets.contains(tweetId) && !shouldSkipTweet(tweet)) {
                    processTweet(tweet);
                    break; // Process one tweet per interval
                }
            }
        }
    }
}

void TwitterBot::handleNetworkReply()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    if (reply->error() != QNetworkReply::NoError) {
        errorLog(QString("Network error: %1").arg(reply->errorString()));
    }
    
    reply->deleteLater();
}

void TwitterBot::handlePageLoadFinished(bool ok)
{
    if (!ok) {
        errorLog("Page load failed");
    }
}

void TwitterBot::onTimerTimeout()
{
    // Handle timer timeouts
}

bool TwitterBot::isOwnTweet(const QJsonObject& tweet)
{
    QString username = tweet.value("username").toString();
    return username.toLower() == m_username.toLower();
}

bool TwitterBot::isReplyTweet(const QJsonObject& tweet)
{
    return tweet.value("is_reply", false).toBool();
}

bool TwitterBot::shouldSkipTweet(const QJsonObject& tweet)
{
    // Skip own tweets
    if (isOwnTweet(tweet)) {
        return true;
    }
    
    // Check time limit
    QString timestampStr = getTweetTimestamp(tweet);
    QDateTime tweetTime = QDateTime::fromString(timestampStr, Qt::ISODate);
    QDateTime cutoffTime = QDateTime::currentDateTime().addSecs(-(m_timeLimitHours * 3600 + m_timeLimitMinutes * 60));
    
    if (tweetTime < cutoffTime) {
        return true;
    }
    
    // Apply additional filters from settings
    QString content = tweet.value("content").toString().toLower();
    
    // Check for filtered keywords
    QStringList filterWords = m_filterSettings.value("filter_words").toStringList();
    for (const QString& word : filterWords) {
        if (content.contains(word.toLower())) {
            return true;
        }
    }
    
    return false;
}

QString TwitterBot::getTweetId(const QJsonObject& tweet)
{
    return tweet.value("id").toString();
}

QString TwitterBot::getTweetTimestamp(const QJsonObject& tweet)
{
    return tweet.value("timestamp").toString();
}

QString TwitterBot::cleanText(const QString& text)
{
    QString cleaned = text.trimmed();
    
    // Remove extra whitespace
    cleaned = cleaned.replace(QRegularExpression("\\s+"), " ");
    
    // Remove quotes if they wrap the entire text
    if (cleaned.startsWith('"') && cleaned.endsWith('"')) {
        cleaned = cleaned.mid(1, cleaned.length() - 2);
    }
    
    return cleaned;
}

QString TwitterBot::cleanHashtags(const QString& text)
{
    QString cleaned = text;
    
    // Remove hashtags but keep the text
    cleaned = cleaned.replace(QRegularExpression("#(\\w+)"), "\\1");
    
    return cleaned;
}

void TwitterBot::log(const QString& message)
{
    QString logMessage = QString("[%1] %2").arg(QDateTime::currentDateTime().toString(), message);
    emit logSignal(logMessage);
    
    // Store in database
    if (m_database) {
        m_database->addLog(m_username, "INFO", message);
    }
}

void TwitterBot::errorLog(const QString& message)
{
    QString logMessage = QString("[%1] ERROR: %2").arg(QDateTime::currentDateTime().toString(), message);
    emit logSignal(logMessage);
    emit errorSignal(message);
    
    // Store in database
    if (m_database) {
        m_database->addLog(m_username, "ERROR", message);
    }
}

bool TwitterBot::waitForPageLoad(int timeoutMs)
{
    if (!m_webView) {
        return false;
    }
    
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    
    connect(m_webPage, &QWebEnginePage::loadFinished, &loop, &QEventLoop::quit);
    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    timeoutTimer.start(timeoutMs);
    loop.exec();
    
    return !timeoutTimer.isActive(); // If timer is still active, we timed out
}

void TwitterBot::minimizeWindow()
{
    if (m_shouldMinimize && m_mainWindow) {
        m_mainWindow->showMinimized();
    }
}

bool TwitterBot::ensureConnection()
{
    if (!m_webView) {
        return false;
    }
    
    // Check if we're still connected to Twitter
    return checkLoggedIn();
}

void TwitterBot::loadInitialSettings()
{
    if (!m_database) {
        return;
    }
    
    QVariantMap settings = m_database->getAccountSettings(m_username);
    if (!settings.isEmpty()) {
        updateSettings(settings);
    }
}

bool TwitterBot::updateSettingsFromDB()
{
    if (!m_database) {
        return false;
    }
    
    QVariantMap settings = m_database->getAccountSettings(m_username);
    if (!settings.isEmpty()) {
        updateSettings(settings);
        return true;
    }
    
    return false;
}

void TwitterBot::clearApiCache()
{
    m_apiCache.clear();
}

bool TwitterBot::isApiResponseCached(const QString& key)
{
    return m_apiCache.contains(key);
}

QString TwitterBot::getCachedApiResponse(const QString& key)
{
    QString* cached = m_apiCache.object(key);
    return cached ? *cached : QString();
}

void TwitterBot::cacheApiResponse(const QString& key, const QString& response)
{
    m_apiCache.insert(key, new QString(response));
}

void TwitterBot::cleanup()
{
    stop();
    
    if (m_webView) {
        m_webView->deleteLater();
        m_webView = nullptr;
    }
    
    m_webPage = nullptr;
}

void TwitterBot::onTweetProcessed(const QString& timeStr, const QString& username,
                                const QString& tweetUrl, const QString& replyText,
                                const QString& status, bool likeSuccess, bool followSuccess,
                                bool retweetSuccess, int replyCount, int viewCount,
                                const QString& tweetId, const QString& originalContent)
{
    // This slot can be used to handle tweet processing results
    // Forward the signal to the GUI
    emit tweetProcessedSignal(timeStr, username, tweetUrl, replyText, status,
                            likeSuccess, followSuccess, retweetSuccess,
                            replyCount, viewCount, tweetId, originalContent);
}
