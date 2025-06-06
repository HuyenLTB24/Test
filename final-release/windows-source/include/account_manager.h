#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

#include <QDialog>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
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
#include <QVariantMap>
#include <QStringList>

class DatabaseManager;

class AccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AccountDialog(DatabaseManager* database, const QString& profileId = QString(), QWidget* parent = nullptr);
    
    QVariantMap getAccountData() const;

private slots:
    void onAccept();
    void onReject();
    void testConnection();
    void showApiKey();
    void showApiSecret();

private:
    void setupUI();
    void loadAccountData(const QString& profileId);
    void populateFields(const QVariantMap& data);
    bool validateInput();

private:
    DatabaseManager* m_database;
    QString m_profileId;
    bool m_isEditMode;
    
    // Form fields
    QLineEdit* m_nameEdit;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_emailEdit;
    QLineEdit* m_phoneEdit;
    
    // Browser settings
    QLineEdit* m_debugPortEdit;
    QLineEdit* m_profilePathEdit;
    QCheckBox* m_useProxyCheck;
    QLineEdit* m_proxyEdit;
    
    // AI settings
    QCheckBox* m_useGeminiCheck;
    QLineEdit* m_geminiKeyEdit;
    QLineEdit* m_chatgptKeyEdit;
    
    // Bot settings
    QSpinBox* m_maxRepliesSpin;
    QSpinBox* m_minViewsSpin;
    QSpinBox* m_intervalSpin;
    QSpinBox* m_replyIntervalSpin;
    QCheckBox* m_skipRepliesCheck;
    QCheckBox* m_skipRetweetsCheck;
    QCheckBox* m_autoLikeCheck;
    QCheckBox* m_autoFollowVerifiedCheck;
    QCheckBox* m_autoRetweetCheck;
    
    // Schedule settings
    QCheckBox* m_scheduleEnabledCheck;
    QLineEdit* m_startTimeEdit;
    QLineEdit* m_endTimeEdit;
    QLineEdit* m_scheduleDaysEdit;
    
    // Buttons
    QPushButton* m_testButton;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
};

class AccountManager : public QWidget
{
    Q_OBJECT

public:
    explicit AccountManager(DatabaseManager* database, QWidget* parent = nullptr);

signals:
    void accountChanged(const QString& profileId);

public slots:
    void refreshAccounts();

private slots:
    void addAccount();
    void editAccount();
    void deleteAccount();
    void duplicateAccount();
    void exportAccounts();
    void importAccounts();
    void onAccountSelectionChanged();

private:
    void setupUI();
    void loadAccounts();
    void updateButtonStates();
    QString getCurrentProfileId() const;

private:
    DatabaseManager* m_database;
    
    // UI components
    QVBoxLayout* m_layout;
    QTableWidget* m_accountsTable;
    
    // Buttons
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_duplicateButton;
    QPushButton* m_exportButton;
    QPushButton* m_importButton;
    QPushButton* m_refreshButton;
    
    // Current selection
    int m_currentRow;
};

#endif // ACCOUNT_MANAGER_H
