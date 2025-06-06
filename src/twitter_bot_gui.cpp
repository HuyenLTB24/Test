#include "twitter_bot_gui.h"
#include "twitter_bot.h"
#include "database_manager.h"
#include "bot_worker.h"
#include "account_manager.h"
#include <QApplication>
#include <QMessageBox>
#include <QHeaderView>
#include <QScrollArea>
#include <QSplitter>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QTimer>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QBrush>
#include <QColor>
#include <QDebug>

// Style constants
const QString TwitterBotGUI::MAIN_STYLE = R"(
    QMainWindow {
        background-color: #f2f9f1;
        color: #333;
    }
    QLabel {
        color: #333;
    }
    QPushButton {
        border: none;
        padding: 10px;
        border-radius: 5px;
    }
    QPushButton:hover {
        background-color: #e0f0e0;
    }
)";

const QString TwitterBotGUI::SIDEBAR_STYLE = R"(
    QFrame#sidebar_frame {
        background-color: #ffffff;
        border-right: 1px solid #e0e0e0;
    }
)";

const QString TwitterBotGUI::CARD_STYLE = R"(
    QFrame#account_card {
        background-color: white;
        border-radius: 10px;
        padding: 15px;
    }
    QFrame#account_card_content {
        background-color: white;
    }
)";

const QString TwitterBotGUI::BUTTON_STYLE = R"(
    QPushButton#start_button {
        background-color: #4CD964;
        color: white;
    }
    QPushButton#stop_button {
        background-color: #FF3B30;
        color: white;
    }
    QPushButton#pause_button {
        background-color: #FF9500;
        color: white;
    }
    QPushButton#edit_button {
        background-color: #1DA1F2;
        color: white;
    }
)";

const QString TwitterBotGUI::TABLE_STYLE = R"(
    QTableWidget {
        gridline-color: #E1E8ED;
        border: 1px solid #E1E8ED;
        border-radius: 8px;
        background-color: white;
        alternate-background-color: #F5F8FA;
    }
    QHeaderView::section {
        background-color: #F5F8FA;
        padding: 6px;
        font-weight: bold;
        border: none;
        border-bottom: 1px solid #E1E8ED;
    }
    QTableWidget::item {
        padding: 6px;
    }
)";

TwitterBotGUI::TwitterBotGUI(QWidget* parent)
    : QMainWindow(parent)
    , m_database(new DatabaseManager(this, this))
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_sidebar(nullptr)
    , m_contentArea(nullptr)
    , m_contentLayout(nullptr)
    , m_accountsContainer(nullptr)
    , m_accountsLayout(nullptr)
    , m_scrollArea(nullptr)
    , m_modeCombo(nullptr)
    , m_modeGroup(nullptr)
    , m_tweetsTable(nullptr)
    , m_logTable(nullptr)
    , m_refreshTimer(new QTimer(this))
    , m_logRefreshTimer(new QTimer(this))
    , m_accountManager(nullptr)
    , m_currentPage("Accounts")
{
    initUI();
    showAccountsPage();
    
    // Setup refresh timers
    connect(m_refreshTimer, &QTimer::timeout, this, &TwitterBotGUI::refreshTweetStats);
    connect(m_logRefreshTimer, &QTimer::timeout, this, &TwitterBotGUI::loadLogData);
}

TwitterBotGUI::~TwitterBotGUI()
{
    // Stop all bots before destroying
    stopAllBots();
    
    // Cleanup workers
    for (auto* worker : m_botWorkers) {
        if (worker && worker->isRunning()) {
            worker->stop();
            worker->wait(5000); // Wait up to 5 seconds
        }
    }
}

void TwitterBotGUI::initUI()
{
    setWindowTitle("X2 Modern - Twitter Bot Management");
    setMinimumSize(1000, 600);
    resize(1200, 800);
    
    // Central widget
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Create sidebar
    m_sidebar = createSidebar();
    m_mainLayout->addWidget(m_sidebar, 1);
    
    // Create content area
    m_contentArea = new QWidget;
    m_contentLayout = new QVBoxLayout(m_contentArea);
    m_contentLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->addWidget(m_contentArea, 4);
    
    // Create header
    createHeader();
    
    // Create accounts container
    m_accountsContainer = new QWidget;
    m_accountsLayout = new QGridLayout(m_accountsContainer);
    m_accountsLayout->setContentsMargins(0, 0, 0, 0);
    m_accountsLayout->setSpacing(20);
    
    // Scroll area for accounts
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setWidget(m_accountsContainer);
    
    m_contentLayout->addWidget(m_scrollArea);
    
    // Mode selection
    m_modeGroup = new QComboBox;
    m_modeGroup->addItems({"Feed Mode", "User Mode", "Comments Mode", "Trending Mode"});
    connect(m_modeGroup, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TwitterBotGUI::onModeChanged);
    
    setupStyles();
}

void TwitterBotGUI::setupStyles()
{
    setStyleSheet(MAIN_STYLE + SIDEBAR_STYLE + CARD_STYLE + BUTTON_STYLE + TABLE_STYLE + R"(
        QTabWidget::pane {
            border: 1px solid #e0e0e0;
            border-radius: 5px;
        }
        QTabBar::tab {
            background-color: #f0f0f0;
            padding: 8px 15px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background-color: #4CAF50;
            color: white;
        }
        QComboBox {
            padding: 5px;
            border: 1px solid #e0e0e0;
            border-radius: 3px;
        }
        QLineEdit {
            padding: 5px;
            border: 1px solid #e0e0e0;
            border-radius: 3px;
        }
    )");
}

QFrame* TwitterBotGUI::createSidebar()
{
    QFrame* sidebarFrame = new QFrame;
    sidebarFrame->setObjectName("sidebar_frame");
    
    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebarFrame);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);
    
    // Logo and label
    QWidget* logoContainer = new QWidget;
    QHBoxLayout* logoLayout = new QHBoxLayout(logoContainer);
    
    QLabel* logoLabel = new QLabel("X");
    logoLabel->setStyleSheet("background-color: #1DA1F2; color: white; font-size: 24px; "
                            "padding: 10px; border-radius: 8px;");
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setFixedSize(60, 60);
    
    QLabel* groupLabel = new QLabel("Twitter Bot");
    groupLabel->setFont(QFont("Arial", 18, QFont::Bold));
    
    logoLayout->addWidget(logoLabel);
    logoLayout->addWidget(groupLabel);
    logoLayout->setContentsMargins(20, 20, 20, 20);
    
    sidebarLayout->addWidget(logoContainer);
    sidebarLayout->addSpacing(20);
    
    // Menu items
    QStringList menuItems = {"Accounts", "Settings", "Statistics", "Log", "Account Manager"};
    QStringList menuIcons = {"👤", "⚙️", "📊", "📝", "👥"};
    
    for (int i = 0; i < menuItems.size(); ++i) {
        const QString& name = menuItems[i];
        const QString& icon = menuIcons[i];
        
        QPushButton* menuButton = new QPushButton(QString("%1 %2").arg(icon, name));
        menuButton->setFont(QFont("Arial", 12));
        menuButton->setStyleSheet(QString(
            "QPushButton {"
            "    text-align: left;"
            "    padding: 15px 20px;"
            "    background-color: %1;"
            "    border: none;"
            "    color: %2;"
            "}"
            "QPushButton:hover {"
            "    background-color: #f0f8ff;"
            "}"
        ).arg(name == "Accounts" ? "#e3f2fd" : "transparent",
              name == "Accounts" ? "#1976d2" : "#666"));
        
        m_sidebarMenuButtons[name] = menuButton;
        
        // Connect menu buttons
        if (name == "Accounts") {
            connect(menuButton, &QPushButton::clicked, this, &TwitterBotGUI::showAccountsPage);
        } else if (name == "Settings") {
            connect(menuButton, &QPushButton::clicked, this, &TwitterBotGUI::showSettingsPage);
        } else if (name == "Statistics") {
            connect(menuButton, &QPushButton::clicked, this, &TwitterBotGUI::showStatsPage);
        } else if (name == "Log") {
            connect(menuButton, &QPushButton::clicked, this, &TwitterBotGUI::showLogPage);
        } else if (name == "Account Manager") {
            connect(menuButton, &QPushButton::clicked, this, &TwitterBotGUI::showAccountManager);
        }
        
        sidebarLayout->addWidget(menuButton);
    }
    
    sidebarLayout->addStretch(1);
    
    // Bot controls at bottom
    QWidget* controlsContainer = new QWidget;
    QVBoxLayout* controlsLayout = new QVBoxLayout(controlsContainer);
    
    // Mode selector
    QLabel* modeLabel = new QLabel("Bot Mode:");
    modeLabel->setFont(QFont("Arial", 12, QFont::Bold));
    
    m_modeCombo = new QComboBox;
    m_modeCombo->addItems({"Feed Mode", "User Mode", "Comments Mode", "Trending Mode"});
    m_modeCombo->setCurrentIndex(0);
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TwitterBotGUI::onGlobalModeChanged);
    
    controlsLayout->addWidget(modeLabel);
    controlsLayout->addWidget(m_modeCombo);
    controlsLayout->addSpacing(10);
    
    // Global control buttons
    QWidget* globalButtonsContainer = new QWidget;
    QHBoxLayout* globalButtonsLayout = new QHBoxLayout(globalButtonsContainer);
    
    QPushButton* startAllButton = new QPushButton("Start All");
    startAllButton->setStyleSheet("background-color: #4CD964; color: white;");
    connect(startAllButton, &QPushButton::clicked, this, &TwitterBotGUI::startAllBots);
    
    QPushButton* stopAllButton = new QPushButton("Stop All");
    stopAllButton->setStyleSheet("background-color: #FF3B30; color: white;");
    connect(stopAllButton, &QPushButton::clicked, this, &TwitterBotGUI::stopAllBots);
    
    globalButtonsLayout->addWidget(startAllButton);
    globalButtonsLayout->addWidget(stopAllButton);
    
    controlsLayout->addWidget(globalButtonsContainer);
    
    sidebarLayout->addWidget(controlsContainer);
    sidebarLayout->addSpacing(20);
    
    return sidebarFrame;
}

void TwitterBotGUI::createHeader()
{
    QWidget* headerWidget = new QWidget;
    QVBoxLayout* headerLayout = new QVBoxLayout(headerWidget);
    
    QLabel* welcomeLabel = new QLabel("Accounts Dashboard");
    welcomeLabel->setFont(QFont("Arial", 24, QFont::Bold));
    
    QLabel* subtitleLabel = new QLabel("Manage your Twitter bot accounts and activities");
    subtitleLabel->setFont(QFont("Arial", 14));
    subtitleLabel->setStyleSheet("color: #757575");
    
    headerLayout->addWidget(welcomeLabel);
    headerLayout->addWidget(subtitleLabel);
    
    QPushButton* addAccountButton = new QPushButton("Add Account");
    addAccountButton->setStyleSheet(R"(
        background-color: #1DA1F2;
        color: white;
        padding: 8px 15px;
        border-radius: 5px;
        font-weight: bold;
    )");
    addAccountButton->setFixedWidth(150);
    connect(addAccountButton, &QPushButton::clicked, this, &TwitterBotGUI::addNewAccount);
    
    QHBoxLayout* headerTopLayout = new QHBoxLayout;
    headerTopLayout->addLayout(headerLayout);
    headerTopLayout->addStretch();
    headerTopLayout->addWidget(addAccountButton, 0, Qt::AlignRight);
    
    QWidget* headerContainer = new QWidget;
    headerContainer->setLayout(headerTopLayout);
    
    m_contentLayout->addWidget(headerContainer);
}

void TwitterBotGUI::loadAccounts()
{
    try {
        // Clear existing account widgets
        while (m_accountsLayout->count() > 0) {
            QLayoutItem* item = m_accountsLayout->takeAt(0);
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        
        // Get accounts from database
        QList<QVariantMap> accounts = m_database->getAllAccounts();
        
        if (accounts.isEmpty()) {
            showMessage("No Accounts", "Please add accounts in the Account Manager");
            return;
        }
        
        // Add each account to grid
        for (int i = 0; i < accounts.size(); ++i) {
            const QVariantMap& account = accounts[i];
            QString profileId = account.value("profile_id").toString();
            
            // Create account data
            QVariantMap accountData;
            accountData["name"] = account.value("name", QString("Account %1").arg(i + 1));
            accountData["username"] = account.value("username");
            accountData["profile_id"] = profileId;
            accountData["status"] = "Stopped";
            accountData["replies"] = 0;
            accountData["likes"] = 0;
            accountData["follows"] = 0;
            accountData["gemini_key"] = account.value("gemini_key");
            accountData["chatgpt_key"] = account.value("chatgpt_key");
            accountData["use_gemini"] = account.value("use_gemini", true);
            
            // Get statistics from database
            QVariantMap stats = m_database->getAccountStats(profileId);
            if (!stats.isEmpty()) {
                accountData["replies"] = stats.value("replies_sent", 0);
                accountData["likes"] = stats.value("likes_given", 0);
                accountData["follows"] = stats.value("follows_made", 0);
            }
            
            // Add to grid (4 columns)
            int row = i / 4;
            int col = i % 4;
            QFrame* accountCard = createAccountCard(accountData);
            accountCard->setProperty("profile_id", profileId);
            m_accountsLayout->addWidget(accountCard, row, col);
            
            // Update status if bot is already running
            if (m_botInstances.contains(profileId)) {
                TwitterBot* bot = m_botInstances[profileId];
                if (bot->isPaused()) {
                    updateAccountStatus(profileId, "Paused");
                } else if (bot->isRunning()) {
                    updateAccountStatus(profileId, "Running");
                }
            }
        }
        
    } catch (const std::exception& e) {
        showMessage("Error", QString("Could not load accounts: %1").arg(e.what()));
    }
}

QFrame* TwitterBotGUI::createAccountCard(const QVariantMap& accountData)
{
    QString profileId = accountData.value("profile_id").toString();
    
    QFrame* cardFrame = new QFrame;
    cardFrame->setObjectName("account_card");
    cardFrame->setFrameStyle(QFrame::Box);
    cardFrame->setMinimumSize(280, 300);
    
    QVBoxLayout* cardLayout = new QVBoxLayout(cardFrame);
    
    // Header with name and toggle switch
    QWidget* headerWidget = new QWidget;
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    
    QLabel* iconLabel = new QLabel("🐦");
    iconLabel->setFont(QFont("Arial", 20));
    
    QLabel* nameLabel = new QLabel(accountData.value("name").toString());
    nameLabel->setFont(QFont("Arial", 16, QFont::Bold));
    
    QString switchColor = accountData.value("status").toString() == "Running" ? "#4CD964" : "#8E8E93";
    QPushButton* switchButton = new QPushButton;
    switchButton->setObjectName(QString("switch_%1").arg(profileId));
    switchButton->setCheckable(true);
    switchButton->setChecked(accountData.value("status").toString() == "Running");
    switchButton->setFixedSize(50, 30);
    switchButton->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: %1;"
        "    border-radius: 15px;"
        "    border: none;"
        "}"
        "QPushButton:checked {"
        "    background-color: #4CD964;"
        "}"
        "QPushButton:!checked {"
        "    background-color: #8E8E93;"
        "}"
    ).arg(switchColor));
    
    connect(switchButton, &QPushButton::clicked,
            this, [this, profileId](bool checked) { toggleBot(profileId, checked); });
    
    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(nameLabel, 1);
    headerLayout->addWidget(switchButton);
    
    cardLayout->addWidget(headerWidget);
    
    // Username and status
    QLabel* usernameLabel = new QLabel(QString("@%1").arg(accountData.value("username").toString()));
    usernameLabel->setStyleSheet("color: #1DA1F2;");
    usernameLabel->setFont(QFont("Arial", 12));
    cardLayout->addWidget(usernameLabel);
    
    QLabel* statusLabel = new QLabel(accountData.value("status").toString());
    statusLabel->setObjectName(QString("status_%1").arg(profileId));
    statusLabel->setStyleSheet(accountData.value("status").toString() == "Running" 
                              ? "color: #4CD964;" : "color: #8E8E93;");
    cardLayout->addWidget(statusLabel);
    
    // Tabs for stats and settings
    QTabWidget* tabs = new QTabWidget;
    
    // Stats tab
    QWidget* statsTab = new QWidget;
    QVBoxLayout* statsLayout = new QVBoxLayout(statsTab);
    
    QWidget* statsWidget = new QWidget;
    QGridLayout* statsGrid = new QGridLayout(statsWidget);
    statsGrid->setContentsMargins(0, 0, 0, 0);
    
    QLabel* repliesLabel = new QLabel(QString("Replies: %1").arg(accountData.value("replies").toString()));
    repliesLabel->setObjectName(QString("replies_%1").arg(profileId));
    
    QLabel* likesLabel = new QLabel(QString("Likes: %1").arg(accountData.value("likes").toString()));
    likesLabel->setObjectName(QString("likes_%1").arg(profileId));
    
    QLabel* followsLabel = new QLabel(QString("Follows: %1").arg(accountData.value("follows").toString()));
    followsLabel->setObjectName(QString("follows_%1").arg(profileId));
    
    statsGrid->addWidget(repliesLabel, 0, 0);
    statsGrid->addWidget(likesLabel, 1, 0);
    statsGrid->addWidget(followsLabel, 2, 0);
    
    // Performance stats
    QWidget* performanceWidget = new QWidget;
    QVBoxLayout* performanceLayout = new QVBoxLayout(performanceWidget);
    performanceLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* performanceValue = new QLabel(accountData.value("status").toString() == "Running" ? "Active" : "Inactive");
    performanceValue->setObjectName(QString("performance_%1").arg(profileId));
    performanceValue->setFont(QFont("Arial", 18, QFont::Bold));
    performanceValue->setStyleSheet(accountData.value("status").toString() == "Running" 
                                   ? "color: #4CD964;" : "color: #FF3B30;");
    
    performanceLayout->addWidget(performanceValue, 0, Qt::AlignRight);
    
    statsGrid->addWidget(performanceWidget, 0, 1, 3, 1);
    statsLayout->addWidget(statsWidget);
    
    // Settings tab
    QWidget* settingsTab = new QWidget;
    QVBoxLayout* settingsLayout = new QVBoxLayout(settingsTab);
    
    // Mode settings
    QWidget* modeContainer = new QWidget;
    QHBoxLayout* modeLayout = new QHBoxLayout(modeContainer);
    
    QLabel* modeLabel = new QLabel("Mode:");
    QComboBox* modeCombo = new QComboBox;
    modeCombo->setObjectName(QString("mode_combo_%1").arg(profileId));
    modeCombo->addItems({"Feed Mode", "User Mode", "Comments Mode", "Trending Mode"});
    
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, profileId](int idx) { onAccountModeChanged(profileId, idx); });
    
    modeLayout->addWidget(modeLabel);
    modeLayout->addWidget(modeCombo, 1);
    
    // Search keyword for trending mode
    QWidget* keywordContainer = new QWidget;
    QHBoxLayout* keywordLayout = new QHBoxLayout(keywordContainer);
    
    QLabel* keywordLabel = new QLabel("Keyword:");
    QLineEdit* keywordInput = new QLineEdit;
    keywordInput->setObjectName(QString("keyword_input_%1").arg(profileId));
    keywordInput->setPlaceholderText("Enter search keyword");
    
    keywordLayout->addWidget(keywordLabel);
    keywordLayout->addWidget(keywordInput, 1);
    
    settingsLayout->addWidget(modeContainer);
    settingsLayout->addWidget(keywordContainer);
    
    // Advanced settings button
    QPushButton* configButton = new QPushButton("Advanced Settings");
    configButton->setStyleSheet("background-color: #1DA1F2; color: white;");
    connect(configButton, &QPushButton::clicked,
            this, [this, profileId]() { showAccountSettings(profileId); });
    settingsLayout->addWidget(configButton);
    
    settingsLayout->addStretch(1);
    
    // Add tabs
    tabs->addTab(statsTab, "Stats");
    tabs->addTab(settingsTab, "Settings");
    
    cardLayout->addWidget(tabs);
    
    // Action buttons
    QWidget* buttonsWidget = new QWidget;
    QHBoxLayout* buttonsLayout = new QHBoxLayout(buttonsWidget);
    buttonsLayout->setContentsMargins(0, 10, 0, 0);
    
    QPushButton* startButton = new QPushButton("Start");
    startButton->setObjectName("start_button");
    connect(startButton, &QPushButton::clicked,
            this, [this, profileId]() { startBot(profileId); });
    
    QPushButton* stopButton = new QPushButton("Stop");
    stopButton->setObjectName("stop_button");
    connect(stopButton, &QPushButton::clicked,
            this, [this, profileId]() { stopBot(profileId); });
    
    QPushButton* pauseButton = new QPushButton("Pause");
    pauseButton->setObjectName("pause_button");
    connect(pauseButton, &QPushButton::clicked,
            this, [this, profileId]() { pauseBot(profileId); });
    
    QPushButton* editButton = new QPushButton("Edit");
    editButton->setObjectName("edit_button");
    connect(editButton, &QPushButton::clicked,
            this, [this, profileId]() { editAccount(profileId); });
    
    buttonsLayout->addWidget(startButton);
    buttonsLayout->addWidget(stopButton);
    buttonsLayout->addWidget(pauseButton);
    buttonsLayout->addWidget(editButton);
    
    cardLayout->addWidget(buttonsWidget);
    
    return cardFrame;
}

void TwitterBotGUI::showAccountManager()
{
    if (!m_accountManager) {
        m_accountManager = new AccountManager(m_database, this);
        connect(m_accountManager, &AccountManager::accountChanged,
                this, &TwitterBotGUI::onAccountChanged);
    }
    
    clearContentArea();
    setActiveMenuItem("Account Manager");
    
    QLabel* titleLabel = new QLabel("Account Manager");
    titleLabel->setFont(QFont("Arial", 24, QFont::Bold));
    m_contentLayout->addWidget(titleLabel);
    
    QLabel* subtitleLabel = new QLabel("Add, edit, and manage your Twitter accounts");
    subtitleLabel->setStyleSheet("color: #757575");
    subtitleLabel->setFont(QFont("Arial", 14));
    m_contentLayout->addWidget(subtitleLabel);
    m_contentLayout->addSpacing(20);
    
    m_contentLayout->addWidget(m_accountManager);
}

void TwitterBotGUI::updateAccountWidget(const QString& profileId, const QVariantMap& newSettings)
{
    try {
        log(QString("Updating settings for account %1").arg(profileId));
        
        // Update bot settings if it's currently active
        if (m_botInstances.contains(profileId)) {
            TwitterBot* bot = m_botInstances[profileId];
            bot->updateSettings(newSettings);
        }
        
        // Reload the account display to show updated settings
        loadAccounts();
        
    } catch (const std::exception& e) {
        log(QString("Error updating account widget: %1").arg(e.what()));
    }
}

void TwitterBotGUI::onAccountChanged(const QString& profileId)
{
    Q_UNUSED(profileId)
    loadAccounts();
}

void TwitterBotGUI::addNewAccount()
{
    AccountDialog dialog(m_database, QString(), this);
    if (dialog.exec() == QDialog::Accepted) {
        QVariantMap accountData = dialog.getAccountData();
        if (m_database->createAccount(accountData)) {
            loadAccounts();
            showMessage("Success", "Account added successfully!");
        } else {
            showMessage("Error", "Failed to add account.");
        }
    }
}

void TwitterBotGUI::editAccount(const QString& profileId)
{
    AccountDialog dialog(m_database, profileId, this);
    if (dialog.exec() == QDialog::Accepted) {
        QVariantMap accountData = dialog.getAccountData();
        if (m_database->updateAccount(profileId, accountData)) {
            loadAccounts();
            showMessage("Success", "Account updated successfully!");
        } else {
            showMessage("Error", "Failed to update account.");
        }
    }
}

void TwitterBotGUI::showAccountSettings(const QString& profileId)
{
    editAccount(profileId);
}

void TwitterBotGUI::startBot(const QString& profileId)
{
    try {
        if (m_botInstances.contains(profileId)) {
            showMessage("Info", "Bot is already running for this account.");
            return;
        }
        
        QVariantMap account = m_database->getAccount(profileId);
        if (account.isEmpty()) {
            showMessage("Error", "Account not found.");
            return;
        }
        
        // Create bot worker
        BotWorker* worker = new BotWorker(account, this, this);
        m_botWorkers[profileId] = worker;
        
        if (TwitterBot* bot = worker->getBot()) {
            m_botInstances[profileId] = bot;
            
            // Connect signals
            connect(worker, &BotWorker::logSignal, this, &TwitterBotGUI::onBotLog);
            connect(worker, &BotWorker::tweetProcessedSignal, this, &TwitterBotGUI::onTweetProcessed);
            connect(worker, &BotWorker::finishedSignal, 
                    this, [this, profileId]() { onBotFinished(profileId); });
        }
        
        worker->start();
        updateAccountStatus(profileId, "Running");
        
        log(QString("Started bot for account: %1").arg(account.value("username").toString()));
        
    } catch (const std::exception& e) {
        showMessage("Error", QString("Failed to start bot: %1").arg(e.what()));
    }
}

void TwitterBotGUI::stopBot(const QString& profileId)
{
    try {
        if (m_botWorkers.contains(profileId)) {
            BotWorker* worker = m_botWorkers[profileId];
            worker->stop();
            worker->wait(5000);
            
            m_botWorkers.remove(profileId);
            delete worker;
        }
        
        if (m_botInstances.contains(profileId)) {
            m_botInstances.remove(profileId);
        }
        
        updateAccountStatus(profileId, "Stopped");
        log(QString("Stopped bot for account: %1").arg(profileId));
        
    } catch (const std::exception& e) {
        showMessage("Error", QString("Failed to stop bot: %1").arg(e.what()));
    }
}

void TwitterBotGUI::pauseBot(const QString& profileId)
{
    try {
        if (m_botInstances.contains(profileId)) {
            TwitterBot* bot = m_botInstances[profileId];
            bot->pause();
            updateAccountStatus(profileId, "Paused");
            log(QString("Paused bot for account: %1").arg(profileId));
        }
    } catch (const std::exception& e) {
        showMessage("Error", QString("Failed to pause bot: %1").arg(e.what()));
    }
}

void TwitterBotGUI::toggleBot(const QString& profileId, bool checked)
{
    if (checked) {
        startBot(profileId);
    } else {
        stopBot(profileId);
    }
}

void TwitterBotGUI::configBot(const QString& profileId)
{
    showMessage("Info", "Advanced configuration feature is under development");
}

void TwitterBotGUI::onAccountModeChanged(const QString& profileId, int modeIndex)
{
    Q_UNUSED(profileId)
    Q_UNUSED(modeIndex)
    // Implementation for account-specific mode change
}

void TwitterBotGUI::onGlobalModeChanged(int modeIndex)
{
    Q_UNUSED(modeIndex)
    // Implementation for global mode change
}

void TwitterBotGUI::startAllBots()
{
    QList<QVariantMap> accounts = m_database->getAllAccounts();
    for (const QVariantMap& account : accounts) {
        QString profileId = account.value("profile_id").toString();
        if (!m_botInstances.contains(profileId)) {
            startBot(profileId);
        }
    }
}

void TwitterBotGUI::stopAllBots()
{
    QStringList profileIds = m_botInstances.keys();
    for (const QString& profileId : profileIds) {
        stopBot(profileId);
    }
}

void TwitterBotGUI::updateAccountStatus(const QString& profileId, const QString& status)
{
    // Find and update the status label for this account
    QLabel* statusLabel = findChild<QLabel*>(QString("status_%1").arg(profileId));
    if (statusLabel) {
        statusLabel->setText(status);
        statusLabel->setStyleSheet(status == "Running" ? "color: #4CD964;" : 
                                  status == "Paused" ? "color: #FF9500;" : "color: #8E8E93;");
    }
    
    // Update switch button
    QPushButton* switchButton = findChild<QPushButton*>(QString("switch_%1").arg(profileId));
    if (switchButton) {
        switchButton->setChecked(status == "Running");
    }
    
    // Update performance label
    QLabel* performanceLabel = findChild<QLabel*>(QString("performance_%1").arg(profileId));
    if (performanceLabel) {
        performanceLabel->setText(status == "Running" ? "Active" : "Inactive");
        performanceLabel->setStyleSheet(status == "Running" ? "color: #4CD964;" : "color: #FF3B30;");
    }
}

void TwitterBotGUI::onTweetProcessed(const QString& timeStr, const QString& username, 
                                    const QString& tweetLink, const QString& replyText,
                                    const QString& status, bool likeSuccess, bool followSuccess,
                                    bool retweetSuccess, int responseTime, int charCount,
                                    const QString& additionalStatsJson, const QString& currentUrl)
{
    Q_UNUSED(timeStr)
    Q_UNUSED(username)
    Q_UNUSED(tweetLink)
    Q_UNUSED(replyText)
    Q_UNUSED(status)
    Q_UNUSED(likeSuccess)
    Q_UNUSED(followSuccess)
    Q_UNUSED(retweetSuccess)
    Q_UNUSED(responseTime)
    Q_UNUSED(charCount)
    Q_UNUSED(additionalStatsJson)
    Q_UNUSED(currentUrl)
    
    // Handle tweet processing updates
    // This could update statistics displays, etc.
}

void TwitterBotGUI::onBotLog(const QString& message)
{
    log(message);
}

void TwitterBotGUI::onBotError(const QString& message)
{
    log(QString("ERROR: %1").arg(message));
}

void TwitterBotGUI::onBotFinished(const QString& profileId)
{
    updateAccountStatus(profileId, "Stopped");
    
    if (m_botWorkers.contains(profileId)) {
        m_botWorkers.remove(profileId);
    }
    if (m_botInstances.contains(profileId)) {
        m_botInstances.remove(profileId);
    }
}

void TwitterBotGUI::onModeChanged(int index)
{
    Q_UNUSED(index)
    // Handle mode change
}

void TwitterBotGUI::showAccountsPage()
{
    clearContentArea();
    setActiveMenuItem("Accounts");
    
    createHeader();
    
    m_contentLayout->addWidget(m_scrollArea);
    loadAccounts();
    
    m_currentPage = "Accounts";
}

void TwitterBotGUI::setActiveMenuItem(const QString& menuName)
{
    for (auto it = m_sidebarMenuButtons.begin(); it != m_sidebarMenuButtons.end(); ++it) {
        QPushButton* button = it.value();
        bool isActive = (it.key() == menuName);
        
        button->setStyleSheet(QString(
            "QPushButton {"
            "    text-align: left;"
            "    padding: 15px 20px;"
            "    background-color: %1;"
            "    border: none;"
            "    color: %2;"
            "}"
            "QPushButton:hover {"
            "    background-color: #f0f8ff;"
            "}"
        ).arg(isActive ? "#e3f2fd" : "transparent",
              isActive ? "#1976d2" : "#666"));
    }
}

void TwitterBotGUI::showSettingsPage()
{
    clearContentArea();
    setActiveMenuItem("Settings");
    
    QLabel* titleLabel = new QLabel("Settings");
    titleLabel->setFont(QFont("Arial", 24, QFont::Bold));
    m_contentLayout->addWidget(titleLabel);
    
    QLabel* subtitleLabel = new QLabel("Configure your Twitter Bot");
    subtitleLabel->setStyleSheet("color: #757575");
    subtitleLabel->setFont(QFont("Arial", 14));
    m_contentLayout->addWidget(subtitleLabel);
    m_contentLayout->addSpacing(20);
    
    // Implementation for settings page
    QLabel* placeholderLabel = new QLabel("Settings page is under development");
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setStyleSheet("color: #999; font-size: 16px;");
    m_contentLayout->addWidget(placeholderLabel);
    
    m_currentPage = "Settings";
}

void TwitterBotGUI::showStatsPage()
{
    clearContentArea();
    setActiveMenuItem("Statistics");
    
    QLabel* titleLabel = new QLabel("Processed Tweets Analytics");
    titleLabel->setFont(QFont("Arial", 24, QFont::Bold));
    m_contentLayout->addWidget(titleLabel);
    
    QLabel* subtitleLabel = new QLabel("View statistics of tweets processed by your bots");
    subtitleLabel->setStyleSheet("color: #757575");
    subtitleLabel->setFont(QFont("Arial", 14));
    m_contentLayout->addWidget(subtitleLabel);
    m_contentLayout->addSpacing(20);
    
    // Add refresh button
    QPushButton* refreshButton = new QPushButton("Refresh Data");
    refreshButton->setStyleSheet(R"(
        QPushButton {
            background-color: #1DA1F2;
            color: white;
            border-radius: 4px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: #0d8ecf;
        }
    )");
    connect(refreshButton, &QPushButton::clicked, this, &TwitterBotGUI::refreshTweetStats);
    
    QHBoxLayout* topLayout = new QHBoxLayout;
    topLayout->addStretch();
    topLayout->addWidget(refreshButton);
    m_contentLayout->addLayout(topLayout);
    
    // Create tweets table
    m_tweetsTable = new QTableWidget;
    m_tweetsTable->setColumnCount(7);
    m_tweetsTable->setHorizontalHeaderLabels({"Time", "Username", "Tweet Link", "Reply", "Status", "Actions", "Views"});
    
    // Set table properties
    m_tweetsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tweetsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tweetsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tweetsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_tweetsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_tweetsTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_tweetsTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    
    m_tweetsTable->setAlternatingRowColors(true);
    
    loadRecentTweetData();
    
    m_contentLayout->addWidget(m_tweetsTable);
    
    m_currentPage = "Statistics";
}

void TwitterBotGUI::refreshTweetStats()
{
    if (m_currentPage == "Statistics") {
        showStatsPage();
    }
}

void TwitterBotGUI::loadRecentTweetData()
{
    try {
        if (!m_tweetsTable) return;
        
        m_tweetsTable->setRowCount(0);
        
        QList<QVariantMap> tweets = m_database->getRepliedTweets(QString(), 100);
        
        if (tweets.isEmpty()) return;
        
        for (int i = 0; i < tweets.size(); ++i) {
            const QVariantMap& tweet = tweets[i];
            m_tweetsTable->insertRow(i);
            
            QString timeStr = tweet.value("replied_at").toString();
            QString username = tweet.value("username").toString();
            QString tweetUrl = QString("https://twitter.com/i/web/status/%1").arg(tweet.value("tweet_id").toString());
            QString replyText = tweet.value("reply_text").toString();
            QString status = "Success";
            
            m_tweetsTable->setItem(i, 0, new QTableWidgetItem(timeStr));
            m_tweetsTable->setItem(i, 1, new QTableWidgetItem(username));
            
            QTableWidgetItem* urlItem = new QTableWidgetItem(tweetUrl);
            urlItem->setForeground(QBrush(QColor("#1DA1F2")));
            m_tweetsTable->setItem(i, 2, urlItem);
            
            QString truncatedReply = replyText.length() > 100 ? replyText.left(100) + "..." : replyText;
            m_tweetsTable->setItem(i, 3, new QTableWidgetItem(truncatedReply));
            
            QTableWidgetItem* statusItem = new QTableWidgetItem(status);
            statusItem->setForeground(QBrush(QColor("#34C759")));
            m_tweetsTable->setItem(i, 4, statusItem);
            
            m_tweetsTable->setItem(i, 5, new QTableWidgetItem("❤️ 👤"));
            m_tweetsTable->setItem(i, 6, new QTableWidgetItem("0"));
        }
        
    } catch (const std::exception& e) {
        showMessage("Error", QString("Could not load tweet data: %1").arg(e.what()));
    }
}

void TwitterBotGUI::showLogPage()
{
    clearContentArea();
    setActiveMenuItem("Log");
    
    QLabel* titleLabel = new QLabel("Program Logs");
    titleLabel->setFont(QFont("Arial", 24, QFont::Bold));
    m_contentLayout->addWidget(titleLabel);
    
    QLabel* subtitleLabel = new QLabel("View and monitor all program activities, bot operations, and system events");
    subtitleLabel->setStyleSheet("color: #757575");
    subtitleLabel->setFont(QFont("Arial", 14));
    m_contentLayout->addWidget(subtitleLabel);
    m_contentLayout->addSpacing(20);
    
    // Log table
    m_logTable = new QTableWidget;
    m_logTable->setColumnCount(6);
    m_logTable->setHorizontalHeaderLabels({"Timestamp", "Level", "Module", "Account", "Message", "Details"});
    
    m_logTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_logTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_logTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_logTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_logTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_logTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    
    m_logTable->setAlternatingRowColors(true);
    
    loadLogData();
    
    m_contentLayout->addWidget(m_logTable);
    
    // Auto-refresh controls
    QHBoxLayout* bottomControls = new QHBoxLayout;
    bottomControls->addStretch();
    
    QLabel* autoRefreshLabel = new QLabel("Auto-refresh:");
    QComboBox* autoRefreshCombo = new QComboBox;
    autoRefreshCombo->addItems({"Off", "5 seconds", "10 seconds", "30 seconds", "1 minute"});
    autoRefreshCombo->setCurrentText("10 seconds");
    connect(autoRefreshCombo, &QComboBox::currentTextChanged,
            this, &TwitterBotGUI::onAutoRefreshChanged);
    
    QPushButton* clearLogsButton = new QPushButton("Clear Logs");
    connect(clearLogsButton, &QPushButton::clicked, this, &TwitterBotGUI::clearLogs);
    
    QPushButton* exportLogsButton = new QPushButton("Export Logs");
    exportLogsButton->setStyleSheet("background-color: #1DA1F2; color: white;");
    connect(exportLogsButton, &QPushButton::clicked, this, &TwitterBotGUI::exportLogs);
    
    bottomControls->addWidget(autoRefreshLabel);
    bottomControls->addWidget(autoRefreshCombo);
    bottomControls->addSpacing(20);
    bottomControls->addWidget(clearLogsButton);
    bottomControls->addWidget(exportLogsButton);
    
    m_contentLayout->addLayout(bottomControls);
    
    startLogAutoRefresh();
    
    m_currentPage = "Log";
}

void TwitterBotGUI::loadLogData()
{
    try {
        if (!m_logTable) return;
        
        m_logTable->setRowCount(0);
        
        QList<QVariantMap> logs = m_database->getLogs(1000);
        
        if (logs.isEmpty()) return;
        
        for (int i = 0; i < logs.size(); ++i) {
            const QVariantMap& log = logs[i];
            m_logTable->insertRow(i);
            
            QString timestamp = log.value("timestamp").toString();
            if (!timestamp.isEmpty()) {
                QDateTime dt = QDateTime::fromString(timestamp, Qt::ISODate);
                if (dt.isValid()) {
                    timestamp = dt.toString("yyyy-MM-dd hh:mm:ss");
                }
            }
            
            m_logTable->setItem(i, 0, new QTableWidgetItem(timestamp));
            
            // Level with color
            QString level = log.value("level").toString();
            QTableWidgetItem* levelItem = new QTableWidgetItem(level);
            if (level == "ERROR") {
                levelItem->setForeground(QBrush(QColor("#FF3B30")));
            } else if (level == "WARNING") {
                levelItem->setForeground(QBrush(QColor("#FF9500")));
            } else if (level == "SUCCESS") {
                levelItem->setForeground(QBrush(QColor("#4CD964")));
            } else if (level == "DEBUG") {
                levelItem->setForeground(QBrush(QColor("#8E8E93")));
            }
            m_logTable->setItem(i, 1, levelItem);
            
            m_logTable->setItem(i, 2, new QTableWidgetItem(log.value("module").toString()));
            
            QString account = log.value("account").toString();
            if (!account.isEmpty()) {
                account = QString("@%1").arg(account);
            }
            m_logTable->setItem(i, 3, new QTableWidgetItem(account));
            
            m_logTable->setItem(i, 4, new QTableWidgetItem(log.value("message").toString()));
            m_logTable->setItem(i, 5, new QTableWidgetItem(log.value("details").toString()));
        }
        
        m_logTable->scrollToBottom();
        
    } catch (const std::exception& e) {
        showMessage("Error", QString("Could not load logs: %1").arg(e.what()));
    }
}

void TwitterBotGUI::clearLogFilters()
{
    // Implementation for clearing log filters
}

void TwitterBotGUI::applyLogFilters()
{
    // Implementation for applying log filters
}

void TwitterBotGUI::clearLogs()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Clear Logs",
        "Are you sure you want to clear all logs? This action cannot be undone.",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        if (m_database->clearLogs()) {
            loadLogData();
            showMessage("Success", "Logs cleared successfully!");
        } else {
            showMessage("Error", "Could not clear logs.");
        }
    }
}

void TwitterBotGUI::exportLogs()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export Logs",
        "",
        "CSV Files (*.csv);;JSON Files (*.json);;Text Files (*.txt)"
    );
    
    if (!filePath.isEmpty()) {
        if (m_database->exportLogs(filePath)) {
            showMessage("Success", "Logs exported successfully!");
        } else {
            showMessage("Error", "Could not export logs.");
        }
    }
}

void TwitterBotGUI::startLogAutoRefresh()
{
    m_logRefreshTimer->start(10000); // Default 10 seconds
}

void TwitterBotGUI::onAutoRefreshChanged(const QString& value)
{
    m_logRefreshTimer->stop();
    
    if (value == "5 seconds") {
        m_logRefreshTimer->start(5000);
    } else if (value == "10 seconds") {
        m_logRefreshTimer->start(10000);
    } else if (value == "30 seconds") {
        m_logRefreshTimer->start(30000);
    } else if (value == "1 minute") {
        m_logRefreshTimer->start(60000);
    }
}

void TwitterBotGUI::clearContentArea()
{
    while (m_contentLayout->count() > 0) {
        QLayoutItem* item = m_contentLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void TwitterBotGUI::showMessage(const QString& title, const QString& message)
{
    QMessageBox::information(this, title, message);
}

void TwitterBotGUI::log(const QString& message)
{
    qDebug() << "LOG:" << message;
    
    // Add to database
    if (m_database) {
        m_database->addLog("INFO", "GUI", QString(), message);
    }
}
