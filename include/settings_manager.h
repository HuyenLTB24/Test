#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariantMap>
#include <QString>

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    explicit SettingsManager(QObject* parent = nullptr);
    
    // Application settings
    void setWindowGeometry(const QByteArray& geometry);
    QByteArray getWindowGeometry() const;
    void setWindowState(const QByteArray& state);
    QByteArray getWindowState() const;
    
    // UI settings
    void setTheme(const QString& theme);
    QString getTheme() const;
    void setLanguage(const QString& language);
    QString getLanguage() const;
    
    // Default bot settings
    void setDefaultBotSettings(const QVariantMap& settings);
    QVariantMap getDefaultBotSettings() const;
    
    // API settings
    void setDefaultApiSettings(const QVariantMap& settings);
    QVariantMap getDefaultApiSettings() const;
    
    // Logging settings
    void setLoggingEnabled(bool enabled);
    bool isLoggingEnabled() const;
    void setLogLevel(const QString& level);
    QString getLogLevel() const;
    void setMaxLogFiles(int count);
    int getMaxLogFiles() const;
    
    // Auto-save settings
    void setAutoSaveInterval(int minutes);
    int getAutoSaveInterval() const;
    void setAutoBackupEnabled(bool enabled);
    bool isAutoBackupEnabled() const;
    
    // Network settings
    void setNetworkTimeout(int seconds);
    int getNetworkTimeout() const;
    void setRetryAttempts(int attempts);
    int getRetryAttempts() const;
    
    // Security settings
    void setRememberPasswords(bool remember);
    bool shouldRememberPasswords() const;
    void setEncryptDatabase(bool encrypt);
    bool shouldEncryptDatabase() const;
    
    // Recent files
    void addRecentFile(const QString& filePath);
    QStringList getRecentFiles() const;
    void clearRecentFiles();
    
    // Custom settings
    void setValue(const QString& key, const QVariant& value);
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;
    
    // Reset to defaults
    void resetToDefaults();

signals:
    void settingsChanged(const QString& key, const QVariant& value);

private:
    QSettings* m_settings;
    
    // Default values
    static const QVariantMap getDefaultSettings();
};

#endif // SETTINGS_MANAGER_H
