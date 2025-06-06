#ifndef TWITTER_BOT_GUI_H
#define TWITTER_BOT_GUI_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QGroupBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QSplitter>
#include <QTimer>
#include <QHash>
#include <QStringList>
#include <QJsonObject>
#include <memory>

class DatabaseManager;
class TwitterBot;
class BotWorker;
class AccountManager;

class TwitterBotGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit TwitterBotGUI(QWidget* parent = nullptr);
    ~TwitterBotGUI();

public slots:
    void updateAccountWidget(const QString& profileId, const QVariantMap& newSettings);

private slots:
    // Menu navigation
    void showAccountsPage();
    void showSettingsPage();
    void showStatsPage();
    void showLogPage();
    void showAccountManager();
    
    // Account management
    void addNewAccount();
    void editAccount(const QString& profileId);
    void onAccountChanged(const QString& profileId);
    void showAccountSettings(const QString& profileId);
    
    // Bot control
    void startBot(const QString& profileId);
    void stopBot(const QString& profileId);
    void pauseBot(const QString& profileId);
    void toggleBot(const QString& profileId, bool checked);
    void configBot(const QString& profileId);
    void startAllBots();
    void stopAllBots();
    
    // Mode changes
    void onAccountModeChanged(const QString& profileId, int modeIndex);
    void onGlobalModeChanged(int modeIndex);
    void onModeChanged(int index);
    
    // Bot events
    void onTweetProcessed(const QString& timeStr, const QString& username, 
                         const QString& tweetLink, const QString& replyText,
                         const QString& status, bool likeSuccess, bool followSuccess,
                         bool retweetSuccess, int responseTime, int charCount,
                         const QString& additionalStatsJson, const QString& currentUrl);
    void onBotLog(const QString& message);
    void onBotError(const QString& message);
    void onBotFinished(const QString& profileId);
    
    // Statistics
    void refreshTweetStats();
    void loadRecentTweetData();
    
    // Logging
    void loadLogData();
    void clearLogFilters();
    void applyLogFilters();
    void clearLogs();
    void exportLogs();
    void startLogAutoRefresh();
    void onAutoRefreshChanged(const QString& value);

private:
    void initUI();
    void setupStyles();
    
    // UI creation
    QFrame* createSidebar();
    void createHeader();
    QFrame* createAccountCard(const QVariantMap& accountData);
    
    // Account management
    void loadAccounts();
    void updateAccountStatus(const QString& profileId, const QString& status);
    
    // Utility
    void clearContentArea();
    void setActiveMenuItem(const QString& menuName);
    void showMessage(const QString& title, const QString& message);
    void log(const QString& message);

private:
    // Core components
    DatabaseManager* m_database;
    
    // UI components
    QWidget* m_centralWidget;
    QHBoxLayout* m_mainLayout;
    QFrame* m_sidebar;
    QWidget* m_contentArea;
    QVBoxLayout* m_contentLayout;
    QWidget* m_accountsContainer;
    QGridLayout* m_accountsLayout;
    QScrollArea* m_scrollArea;
    
    // Controls
    QComboBox* m_modeCombo;
    QComboBox* m_modeGroup;
    QTableWidget* m_tweetsTable;
    QTableWidget* m_logTable;
    
    // Bot management
    QHash<QString, TwitterBot*> m_botInstances;
    QHash<QString, BotWorker*> m_botWorkers;
    QHash<QString, QPushButton*> m_sidebarMenuButtons;
    
    // Timers
    QTimer* m_refreshTimer;
    QTimer* m_logRefreshTimer;
    
    // Account manager
    AccountManager* m_accountManager;
    
    // Current state
    QString m_currentPage;
    
    // Style constants
    static const QString MAIN_STYLE;
    static const QString SIDEBAR_STYLE;
    static const QString CARD_STYLE;
    static const QString BUTTON_STYLE;
    static const QString TABLE_STYLE;
};

#endif // TWITTER_BOT_GUI_H
