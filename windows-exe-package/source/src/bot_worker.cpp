#include "bot_worker.h"
#include "twitter_bot.h"
#include <QDebug>
#include <QEventLoop>

BotWorker::BotWorker(const QVariantMap& credentials, QWidget* mainWindow, QObject* parent)
    : QThread(parent)
    , m_credentials(credentials)
    , m_mainWindow(mainWindow)
    , m_bot(nullptr)
{
}

BotWorker::~BotWorker()
{
    stop();
    wait(); // Wait for thread to finish
}

void BotWorker::stop()
{
    if (m_bot) {
        m_bot->stop();
    }
    
    requestInterruption();
}

void BotWorker::run()
{
    // Create bot instance in the worker thread
    m_bot = std::make_unique<TwitterBot>(m_credentials, m_mainWindow);
    
    // Connect bot signals to worker slots
    connect(m_bot.get(), &TwitterBot::logSignal, this, &BotWorker::handleLog);
    connect(m_bot.get(), &TwitterBot::errorSignal, this, &BotWorker::handleError);
    connect(m_bot.get(), &TwitterBot::tweetProcessedSignal, this, &BotWorker::handleTweetProcessed);
    connect(m_bot.get(), &TwitterBot::stopSignal, this, &QThread::quit);
    
    // Start the bot
    m_bot->start();
    
    // Run the event loop
    exec();
    
    // Cleanup
    if (m_bot) {
        m_bot->stop();
        m_bot.reset();
    }
    
    emit finishedSignal();
}

void BotWorker::handleLog(const QString& message)
{
    emit logSignal(message);
}

void BotWorker::handleError(const QString& message)
{
    emit logSignal(QString("ERROR: %1").arg(message));
}

void BotWorker::handleTweetProcessed(const QString& timeStr, const QString& username,
                                   const QString& tweetUrl, const QString& replyText,
                                   const QString& status, bool likeSuccess, bool followSuccess,
                                   bool retweetSuccess, int replyCount, int viewCount,
                                   const QString& tweetId, const QString& originalContent)
{
    emit tweetProcessedSignal(timeStr, username, tweetUrl, replyText, status,
                            likeSuccess, followSuccess, retweetSuccess,
                            replyCount, viewCount, tweetId, originalContent);
}
