#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QLoggingCategory>
#include "twitter_bot_gui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("X2 Modern");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("X2 Modern - Twitter Bot with Modern UI");
    app.setOrganizationName("X2 Modern");
    app.setOrganizationDomain("x2modern.com");
    
    // Set application icon
    app.setWindowIcon(QIcon(":/resources/icon.ico"));
    
    // Enable high DPI support
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // Set style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Create application data directory
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir appDataDir(appDataPath);
    if (!appDataDir.exists()) {
        if (!appDataDir.mkpath(".")) {
            QMessageBox::critical(nullptr, "Error", 
                                "Failed to create application data directory: " + appDataPath);
            return -1;
        }
    }
    
    // Enable logging categories
    QLoggingCategory::setFilterRules("qt.webengine.chromium.debug=false");
    
    try {
        // Create and show main window
        TwitterBotGUI window;
        window.show();
        
        return app.exec();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Fatal Error", 
                            QString("Application failed to start: %1").arg(e.what()));
        return -1;
    }
}
