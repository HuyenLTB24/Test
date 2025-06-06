#ifndef BOT_WORKER_H
#define BOT_WORKER_H

#include <QThread>
#include <QVariantMap>
#include <memory>

class TwitterBot;
class QWidget;

class BotWorker : public QThread
{
    Q_OBJECT

public:
    explicit BotWorker(const QVariantMap& credentials, QWidget* mainWindow = nullptr, QObject* parent = nullptr);
    ~BotWorker();

    void stop();
    TwitterBot* getBot() const { return m_bot.get(); }

signals:
    void logSignal(const QString& message);
    void tweetProcessedSignal(const QString& timeStr, const QString& username, 
                             const QString& tweetUrl, const QString& replyText,
                             const QString& status, bool likeSuccess, bool followSuccess, 
                             bool retweetSuccess, int replyCount, int viewCount,
                             const QString& tweetId, const QString& originalContent);
    void finishedSignal();

protected:
    void run() override;

private slots:
    void handleLog(const QString& message);
    void handleError(const QString& message);
    void handleTweetProcessed(const QString& timeStr, const QString& username, 
                             const QString& tweetUrl, const QString& replyText,
                             const QString& status, bool likeSuccess, bool followSuccess, 
                             bool retweetSuccess, int replyCount, int viewCount,
                             const QString& tweetId, const QString& originalContent);

private:
    QVariantMap m_credentials;
    QWidget* m_mainWindow;
    std::unique_ptr<TwitterBot> m_bot;
};

#endif // BOT_WORKER_H
