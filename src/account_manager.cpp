#include "account_manager.h"
#include "database_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QTextEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHeaderView>
#include <QSplitter>
#include <QApplication>
#include <QDebug>

// AccountDialog Implementation
AccountDialog::AccountDialog(DatabaseManager* database, const QString& profileId, QWidget* parent)
    : QDialog(parent)
    , m_database(database)
    , m_profileId(profileId)
    , m_isEditMode(!profileId.isEmpty())
    , m_currentRow(-1)
{
    setWindowTitle(m_isEditMode ? "Edit Account" : "Add Account");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(true);
    resize(500, 600);
    
    setupUI();
    
    if (m_isEditMode) {
        loadAccountData(profileId);
    }
}

void AccountDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    QTabWidget* tabWidget = new QTabWidget();
    
    // Account Info Tab
    QWidget* accountTab = new QWidget();
    QFormLayout* accountLayout = new QFormLayout(accountTab);
    
    m_nameEdit = new QLineEdit();
    m_usernameEdit = new QLineEdit();
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_emailEdit = new QLineEdit();
    m_phoneEdit = new QLineEdit();
    
    accountLayout->addRow("Display Name:", m_nameEdit);
    accountLayout->addRow("Username:", m_usernameEdit);
    accountLayout->addRow("Password:", m_passwordEdit);
    accountLayout->addRow("Email:", m_emailEdit);
    accountLayout->addRow("Phone:", m_phoneEdit);
    
    tabWidget->addTab(accountTab, "Account Info");
    
    // Browser Settings Tab
    QWidget* browserTab = new QWidget();
    QFormLayout* browserLayout = new QFormLayout(browserTab);
    
    m_debugPortEdit = new QLineEdit("9222");
    m_profilePathEdit = new QLineEdit();
    m_useProxyCheck = new QCheckBox();
    m_proxyEdit = new QLineEdit();
    m_proxyEdit->setEnabled(false);
    
    connect(m_useProxyCheck, &QCheckBox::toggled, m_proxyEdit, &QLineEdit::setEnabled);
    
    browserLayout->addRow("Debug Port:", m_debugPortEdit);
    browserLayout->addRow("Profile Path:", m_profilePathEdit);
    browserLayout->addRow("Use Proxy:", m_useProxyCheck);
    browserLayout->addRow("Proxy Address:", m_proxyEdit);
    
    tabWidget->addTab(browserTab, "Browser Settings");
    
    // AI Settings Tab
    QWidget* aiTab = new QWidget();
    QFormLayout* aiLayout = new QFormLayout(aiTab);
    
    m_useGeminiCheck = new QCheckBox();
    m_geminiKeyEdit = new QLineEdit();
    m_chatgptKeyEdit = new QLineEdit();
    
    aiLayout->addRow("Use Gemini:", m_useGeminiCheck);
    aiLayout->addRow("Gemini API Key:", m_geminiKeyEdit);
    aiLayout->addRow("ChatGPT API Key:", m_chatgptKeyEdit);
    
    tabWidget->addTab(aiTab, "AI Settings");
    
    // Bot Settings Tab
    QWidget* botTab = new QWidget();
    QFormLayout* botLayout = new QFormLayout(botTab);
    
    m_maxRepliesSpin = new QSpinBox();
    m_maxRepliesSpin->setRange(1, 1000);
    m_maxRepliesSpin->setValue(50);
    
    m_minViewsSpin = new QSpinBox();
    m_minViewsSpin->setRange(0, 100000);
    m_minViewsSpin->setValue(100);
    
    m_intervalSpin = new QSpinBox();
    m_intervalSpin->setRange(30, 3600);
    m_intervalSpin->setValue(60);
    m_intervalSpin->setSuffix(" seconds");
    
    m_replyIntervalSpin = new QSpinBox();
    m_replyIntervalSpin->setRange(60, 7200);
    m_replyIntervalSpin->setValue(180);
    m_replyIntervalSpin->setSuffix(" seconds");
    
    m_skipRepliesCheck = new QCheckBox();
    m_skipRetweetsCheck = new QCheckBox();
    m_autoLikeCheck = new QCheckBox();
    m_autoFollowVerifiedCheck = new QCheckBox();
    m_autoRetweetCheck = new QCheckBox();
    
    botLayout->addRow("Max Replies:", m_maxRepliesSpin);
    botLayout->addRow("Min Views:", m_minViewsSpin);
    botLayout->addRow("Interval:", m_intervalSpin);
    botLayout->addRow("Reply Interval:", m_replyIntervalSpin);
    botLayout->addRow("Skip Replies:", m_skipRepliesCheck);
    botLayout->addRow("Skip Retweets:", m_skipRetweetsCheck);
    botLayout->addRow("Auto Like:", m_autoLikeCheck);
    botLayout->addRow("Auto Follow Verified:", m_autoFollowVerifiedCheck);
    botLayout->addRow("Auto Retweet:", m_autoRetweetCheck);
    
    tabWidget->addTab(botTab, "Bot Settings");
    
    // Schedule Settings Tab
    QWidget* scheduleTab = new QWidget();
    QFormLayout* scheduleLayout = new QFormLayout(scheduleTab);
    
    m_scheduleEnabledCheck = new QCheckBox();
    m_startTimeEdit = new QLineEdit("09:00");
    m_endTimeEdit = new QLineEdit("17:00");
    m_scheduleDaysEdit = new QLineEdit("Monday,Tuesday,Wednesday,Thursday,Friday");
    
    scheduleLayout->addRow("Enable Schedule:", m_scheduleEnabledCheck);
    scheduleLayout->addRow("Start Time:", m_startTimeEdit);
    scheduleLayout->addRow("End Time:", m_endTimeEdit);
    scheduleLayout->addRow("Days:", m_scheduleDaysEdit);
    
    tabWidget->addTab(scheduleTab, "Schedule");
    
    mainLayout->addWidget(tabWidget);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_testButton = new QPushButton("Test Connection");
    m_saveButton = new QPushButton(m_isEditMode ? "Update" : "Add");
    m_cancelButton = new QPushButton("Cancel");
    
    m_saveButton->setDefault(true);
    
    buttonLayout->addWidget(m_testButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_testButton, &QPushButton::clicked, this, &AccountDialog::testConnection);
    connect(m_saveButton, &QPushButton::clicked, this, &AccountDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &AccountDialog::onReject);
}

void AccountDialog::loadAccountData(const QString& profileId)
{
    if (!m_database || profileId.isEmpty()) {
        return;
    }
    
    QVariantMap accountData = m_database->getAccount(profileId);
    if (!accountData.isEmpty()) {
        populateFields(accountData);
    }
}

void AccountDialog::populateFields(const QVariantMap& data)
{
    m_nameEdit->setText(data.value("name").toString());
    m_usernameEdit->setText(data.value("username").toString());
    m_passwordEdit->setText(data.value("password").toString());
    m_emailEdit->setText(data.value("email").toString());
    m_phoneEdit->setText(data.value("phone").toString());
    
    m_debugPortEdit->setText(data.value("debug_port", "9222").toString());
    m_profilePathEdit->setText(data.value("profile_path").toString());
    m_useProxyCheck->setChecked(data.value("use_proxy", false).toBool());
    m_proxyEdit->setText(data.value("proxy").toString());
    
    m_useGeminiCheck->setChecked(data.value("use_gemini", true).toBool());
    m_geminiKeyEdit->setText(data.value("gemini_key").toString());
    m_chatgptKeyEdit->setText(data.value("chatgpt_key").toString());
    
    m_maxRepliesSpin->setValue(data.value("max_replies", 50).toInt());
    m_minViewsSpin->setValue(data.value("min_views", 100).toInt());
    m_intervalSpin->setValue(data.value("interval", 60).toInt());
    m_replyIntervalSpin->setValue(data.value("reply_interval", 180).toInt());
    m_skipRepliesCheck->setChecked(data.value("skip_replies", false).toBool());
    m_skipRetweetsCheck->setChecked(data.value("skip_retweets", false).toBool());
    m_autoLikeCheck->setChecked(data.value("auto_like", true).toBool());
    m_autoFollowVerifiedCheck->setChecked(data.value("auto_follow_verified", false).toBool());
    m_autoRetweetCheck->setChecked(data.value("auto_retweet", false).toBool());
    
    m_scheduleEnabledCheck->setChecked(data.value("schedule_enabled", false).toBool());
    m_startTimeEdit->setText(data.value("start_time", "09:00").toString());
    m_endTimeEdit->setText(data.value("end_time", "17:00").toString());
    m_scheduleDaysEdit->setText(data.value("schedule_days", "Monday,Tuesday,Wednesday,Thursday,Friday").toString());
}

QVariantMap AccountDialog::getAccountData() const
{
    QVariantMap data;
    
    data["name"] = m_nameEdit->text();
    data["username"] = m_usernameEdit->text();
    data["password"] = m_passwordEdit->text();
    data["email"] = m_emailEdit->text();
    data["phone"] = m_phoneEdit->text();
    
    data["debug_port"] = m_debugPortEdit->text();
    data["profile_path"] = m_profilePathEdit->text();
    data["use_proxy"] = m_useProxyCheck->isChecked();
    data["proxy"] = m_proxyEdit->text();
    
    data["use_gemini"] = m_useGeminiCheck->isChecked();
    data["gemini_key"] = m_geminiKeyEdit->text();
    data["chatgpt_key"] = m_chatgptKeyEdit->text();
    
    data["max_replies"] = m_maxRepliesSpin->value();
    data["min_views"] = m_minViewsSpin->value();
    data["interval"] = m_intervalSpin->value();
    data["reply_interval"] = m_replyIntervalSpin->value();
    data["skip_replies"] = m_skipRepliesCheck->isChecked();
    data["skip_retweets"] = m_skipRetweetsCheck->isChecked();
    data["auto_like"] = m_autoLikeCheck->isChecked();
    data["auto_follow_verified"] = m_autoFollowVerifiedCheck->isChecked();
    data["auto_retweet"] = m_autoRetweetCheck->isChecked();
    
    data["schedule_enabled"] = m_scheduleEnabledCheck->isChecked();
    data["start_time"] = m_startTimeEdit->text();
    data["end_time"] = m_endTimeEdit->text();
    data["schedule_days"] = m_scheduleDaysEdit->text();
    
    return data;
}

bool AccountDialog::validateInput()
{
    if (m_nameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Display name is required.");
        return false;
    }
    
    if (m_usernameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Username is required.");
        return false;
    }
    
    return true;
}

void AccountDialog::onAccept()
{
    if (!validateInput()) {
        return;
    }
    
    QVariantMap accountData = getAccountData();
    
    if (!m_database) {
        QMessageBox::critical(this, "Error", "Database not available.");
        return;
    }
    
    bool success;
    if (m_isEditMode) {
        success = m_database->updateAccount(m_profileId, accountData);
    } else {
        success = m_database->addAccount(accountData);
    }
    
    if (success) {
        accept();
    } else {
        QMessageBox::critical(this, "Error", "Failed to save account data.");
    }
}

void AccountDialog::onReject()
{
    reject();
}

void AccountDialog::testConnection()
{
    // In a real implementation, you would test the browser connection
    QMessageBox::information(this, "Test Connection", "Connection test would be implemented here.");
}

void AccountDialog::showApiKey()
{
    // Toggle password visibility for API keys
}

void AccountDialog::showApiSecret()
{
    // Toggle password visibility for API secrets
}

// AccountManager Implementation
AccountManager::AccountManager(DatabaseManager* database, QWidget* parent)
    : QWidget(parent)
    , m_database(database)
    , m_currentRow(-1)
{
    setupUI();
    loadAccounts();
}

void AccountManager::setupUI()
{
    m_layout = new QVBoxLayout(this);
    
    // Toolbar
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    
    m_addButton = new QPushButton("Add Account");
    m_editButton = new QPushButton("Edit");
    m_deleteButton = new QPushButton("Delete");
    m_duplicateButton = new QPushButton("Duplicate");
    m_exportButton = new QPushButton("Export");
    m_importButton = new QPushButton("Import");
    m_refreshButton = new QPushButton("Refresh");
    
    toolbarLayout->addWidget(m_addButton);
    toolbarLayout->addWidget(m_editButton);
    toolbarLayout->addWidget(m_deleteButton);
    toolbarLayout->addWidget(m_duplicateButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_exportButton);
    toolbarLayout->addWidget(m_importButton);
    toolbarLayout->addWidget(m_refreshButton);
    
    m_layout->addLayout(toolbarLayout);
    
    // Accounts table
    m_accountsTable = new QTableWidget();
    m_accountsTable->setColumnCount(6);
    
    QStringList headers = {"Name", "Username", "Status", "Last Active", "Tweets", "Profile ID"};
    m_accountsTable->setHorizontalHeaderLabels(headers);
    
    m_accountsTable->horizontalHeader()->setStretchLastSection(true);
    m_accountsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_accountsTable->setAlternatingRowColors(true);
    
    m_layout->addWidget(m_accountsTable);
    
    // Connect signals
    connect(m_addButton, &QPushButton::clicked, this, &AccountManager::addAccount);
    connect(m_editButton, &QPushButton::clicked, this, &AccountManager::editAccount);
    connect(m_deleteButton, &QPushButton::clicked, this, &AccountManager::deleteAccount);
    connect(m_duplicateButton, &QPushButton::clicked, this, &AccountManager::duplicateAccount);
    connect(m_exportButton, &QPushButton::clicked, this, &AccountManager::exportAccounts);
    connect(m_importButton, &QPushButton::clicked, this, &AccountManager::importAccounts);
    connect(m_refreshButton, &QPushButton::clicked, this, &AccountManager::refreshAccounts);
    
    connect(m_accountsTable, &QTableWidget::itemSelectionChanged, 
            this, &AccountManager::onAccountSelectionChanged);
    connect(m_accountsTable, &QTableWidget::itemDoubleClicked, 
            this, &AccountManager::editAccount);
    
    updateButtonStates();
}

void AccountManager::loadAccounts()
{
    if (!m_database) {
        return;
    }
    
    QList<QVariantMap> accounts = m_database->getAllAccounts();
    
    m_accountsTable->setRowCount(accounts.size());
    
    for (int i = 0; i < accounts.size(); ++i) {
        const QVariantMap& account = accounts[i];
        
        m_accountsTable->setItem(i, 0, new QTableWidgetItem(account.value("name").toString()));
        m_accountsTable->setItem(i, 1, new QTableWidgetItem(account.value("username").toString()));
        m_accountsTable->setItem(i, 2, new QTableWidgetItem("Offline")); // Status would be updated from bot state
        m_accountsTable->setItem(i, 3, new QTableWidgetItem(account.value("last_active").toString()));
        m_accountsTable->setItem(i, 4, new QTableWidgetItem("0")); // Tweet count would come from stats
        m_accountsTable->setItem(i, 5, new QTableWidgetItem(account.value("profile_id").toString()));
    }
    
    updateButtonStates();
}

void AccountManager::addAccount()
{
    AccountDialog dialog(m_database, QString(), this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshAccounts();
        emit accountChanged(QString()); // Signal that accounts changed
    }
}

void AccountManager::editAccount()
{
    QString profileId = getCurrentProfileId();
    if (profileId.isEmpty()) {
        return;
    }
    
    AccountDialog dialog(m_database, profileId, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshAccounts();
        emit accountChanged(profileId);
    }
}

void AccountManager::deleteAccount()
{
    QString profileId = getCurrentProfileId();
    if (profileId.isEmpty()) {
        return;
    }
    
    QString username = m_accountsTable->item(m_currentRow, 1)->text();
    
    int ret = QMessageBox::question(this, "Confirm Delete", 
                                   QString("Are you sure you want to delete account '%1'?").arg(username),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_database && m_database->deleteAccount(profileId)) {
            refreshAccounts();
            emit accountChanged(QString());
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete account.");
        }
    }
}

void AccountManager::duplicateAccount()
{
    QString profileId = getCurrentProfileId();
    if (profileId.isEmpty()) {
        return;
    }
    
    QVariantMap accountData = m_database->getAccount(profileId);
    if (accountData.isEmpty()) {
        return;
    }
    
    // Modify the data for the duplicate
    accountData["name"] = accountData.value("name").toString() + " (Copy)";
    accountData["username"] = accountData.value("username").toString() + "_copy";
    accountData.remove("profile_id"); // Let the database generate a new ID
    
    AccountDialog dialog(m_database, QString(), this);
    // You would need to add a method to populate the dialog with existing data
    if (dialog.exec() == QDialog::Accepted) {
        refreshAccounts();
        emit accountChanged(QString());
    }
}

void AccountManager::exportAccounts()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Accounts", 
                                                   "accounts.json", "JSON Files (*.json)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QList<QVariantMap> accounts = m_database->getAllAccounts();
    
    QJsonArray jsonArray;
    for (const QVariantMap& account : accounts) {
        QJsonObject jsonObj = QJsonObject::fromVariantMap(account);
        jsonArray.append(jsonObj);
    }
    
    QJsonDocument doc(jsonArray);
    
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        QMessageBox::information(this, "Export", "Accounts exported successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to export accounts.");
    }
}

void AccountManager::importAccounts()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Import Accounts", 
                                                   "", "JSON Files (*.json)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Failed to open file.");
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray jsonArray = doc.array();
    
    int imported = 0;
    for (const QJsonValue& value : jsonArray) {
        QJsonObject jsonObj = value.toObject();
        QVariantMap accountData = jsonObj.toVariantMap();
        
        if (m_database->addAccount(accountData)) {
            imported++;
        }
    }
    
    refreshAccounts();
    QMessageBox::information(this, "Import", QString("Imported %1 accounts.").arg(imported));
}

void AccountManager::refreshAccounts()
{
    loadAccounts();
}

void AccountManager::onAccountSelectionChanged()
{
    QList<QTableWidgetItem*> selected = m_accountsTable->selectedItems();
    m_currentRow = selected.isEmpty() ? -1 : selected.first()->row();
    updateButtonStates();
}

void AccountManager::updateButtonStates()
{
    bool hasSelection = (m_currentRow >= 0);
    
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    m_duplicateButton->setEnabled(hasSelection);
}

QString AccountManager::getCurrentProfileId() const
{
    if (m_currentRow < 0 || m_currentRow >= m_accountsTable->rowCount()) {
        return QString();
    }
    
    QTableWidgetItem* item = m_accountsTable->item(m_currentRow, 5); // Profile ID column
    return item ? item->text() : QString();
}
