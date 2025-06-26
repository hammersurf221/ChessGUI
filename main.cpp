#include <QApplication>
#include <QFontDatabase>
#include <QFile>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QLoggingCategory>
#include "mainwindow.h"

// Define logging categories
Q_LOGGING_CATEGORY(main, "main")
Q_LOGGING_CATEGORY(ui, "ui")

int main(int argc, char *argv[])
{
    // Set application properties
    QApplication::setApplicationName("FENgineLive");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("FENgineLive");
    QApplication::setOrganizationDomain("fenginelive.com");

    // Enable high DPI support
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // Use desktop OpenGL for better performance
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication app(argc, argv);

    // Configure logging
    QLoggingCategory::setFilterRules("main.debug=true\n"
                                    "ui.debug=false\n"
                                    "*.info=true\n"
                                    "*.warning=true\n"
                                    "*.critical=true");

    qCInfo(main) << "Starting FENgineLive v" << app.applicationVersion();

    // Style and palette setup
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QPalette palette;

    // Dark theme colors
    palette.setColor(QPalette::Window, QColor("#1E1E1E"));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor("#2A2A2A"));
    palette.setColor(QPalette::AlternateBase, QColor("#3A3A3A"));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor("#2A2A2A"));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Highlight, QColor("#00E676"));
    palette.setColor(QPalette::HighlightedText, Qt::black);

    app.setPalette(palette);

    // Load custom font
    QString fontPath = "assets/fonts/Inter-Regular.ttf";
    if (QFile::exists(fontPath)) {
        int fontId = QFontDatabase::addApplicationFont(fontPath);
        if (fontId != -1) {
            QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
            QFont font(family, 10);
            app.setFont(font);
            qCInfo(main) << "Loaded custom font:" << family;
        } else {
            qCWarning(main) << "Failed to load custom font:" << fontPath;
        }
    } else {
        qCWarning(main) << "Custom font not found:" << fontPath;
    }

    // Load QSS stylesheet
    QString stylePath = "assets/style.qss";
    if (QFile::exists(stylePath)) {
        QFile styleFile(stylePath);
        if (styleFile.open(QFile::ReadOnly)) {
            QString style = QLatin1String(styleFile.readAll());
            app.setStyleSheet(style);
            qCInfo(main) << "Loaded custom stylesheet:" << stylePath;
        } else {
            qCWarning(main) << "Failed to open stylesheet:" << stylePath;
        }
    } else {
        qCWarning(main) << "Stylesheet not found:" << stylePath;
    }

    // Create and show main window
    try {
        MainWindow w;
        w.show();
        
        qCInfo(main) << "Application started successfully";
        return app.exec();
    } catch (const std::exception& e) {
        qCCritical(main) << "Fatal error:" << e.what();
        QMessageBox::critical(nullptr, "Fatal Error", 
                             QString("Application failed to start: %1").arg(e.what()));
        return 1;
    } catch (...) {
        qCCritical(main) << "Unknown fatal error occurred";
        QMessageBox::critical(nullptr, "Fatal Error", 
                             "Application failed to start due to an unknown error.");
        return 1;
    }
}
