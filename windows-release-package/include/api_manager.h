#ifndef API_MANAGER_H
#define API_MANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QQueue>
#include <QMutex>

struct ApiRequest {
    QString id;
    QString endpoint;
    QJsonObject data;
    int retryCount = 0;
    qint64 timestamp;
};

class ApiManager : public QObject
{
    Q_OBJECT

public:
    explicit ApiManager(QObject* parent = nullptr);
    ~ApiManager();

    // API key management
    void setGeminiKey(const QString& key) { m_geminiKey = key; }
    void setChatGPTKey(const QString& key) { m_chatgptKey = key; }
    
    // Rate limiting
    void setRateLimit(int requestsPerMinute) { m_rateLimitPerMinute = requestsPerMinute; }
    
    // Request methods
    QString generateGeminiResponse(const QString& prompt, const QString& context = QString());
    QString generateChatGPTResponse(const QString& prompt, const QString& context = QString());
    void generateGeminiResponseAsync(const QString& requestId, const QString& prompt, const QString& context = QString());
    void generateChatGPTResponseAsync(const QString& requestId, const QString& prompt, const QString& context = QString());

signals:
    void responseReady(const QString& requestId, const QString& response);
    void errorOccurred(const QString& requestId, const QString& error);

private slots:
    void handleNetworkReply();
    void processRequestQueue();

private:
    void sendGeminiRequest(const ApiRequest& request);
    void sendChatGPTRequest(const ApiRequest& request);
    bool canMakeRequest() const;
    void addRequestToQueue(const ApiRequest& request);

private:
    QNetworkAccessManager* m_networkManager;
    QTimer* m_queueTimer;
    
    QString m_geminiKey;
    QString m_chatgptKey;
    
    // Rate limiting
    int m_rateLimitPerMinute;
    QQueue<qint64> m_requestTimes;
    QQueue<ApiRequest> m_requestQueue;
    
    // Thread safety
    QMutex m_mutex;
    
    // Request tracking
    QMap<QNetworkReply*, QString> m_pendingRequests;
};

#endif // API_MANAGER_H
