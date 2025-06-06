#include <QApplication>
#include <QFontDatabase>
#include <QFile>
#include <QStyleFactory>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL); // This is OK before

    QApplication a(argc, argv);  // MUST come before all Qt setup

    // Style and palette setup
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QPalette palette;

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

    a.setPalette(palette);

    // Load custom font
    int fontId = QFontDatabase::addApplicationFont("assets/fonts/Inter-Regular.ttf");
    if (fontId != -1) {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont font(family, 10);
        a.setFont(font);
    }

    // Load QSS stylesheet
    QFile styleFile("assets/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        a.setStyleSheet(style);
    }

    MainWindow w;
    w.show();
    return a.exec();
}
