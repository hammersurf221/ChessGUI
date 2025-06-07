#include "regionselector.h"
#include <QWindow>
#include <QScreen>
#include <QGuiApplication>

RegionSelector::RegionSelector(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);

    setCursor(Qt::CrossCursor);

    // Set size to full screen
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    setGeometry(screenGeometry);
}



void RegionSelector::mousePressEvent(QMouseEvent* event) {
    origin = event->pos();
    selection = QRect();
    selecting = true;
    update();
}

void RegionSelector::mouseMoveEvent(QMouseEvent* event) {
    if (selecting) {
        selection = QRect(origin, event->pos()).normalized();
        update();
    }
}

void RegionSelector::mouseReleaseEvent(QMouseEvent*) {
    selecting = false;
    emit regionSelected(selection);
    close();
}

void RegionSelector::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    // Dimmed background
    QColor dim(0, 0, 0, 100);  // Semi-transparent black
    painter.fillRect(rect(), dim);

    // Draw selection box
    if (!selection.isNull()) {
        painter.setPen(QPen(Qt::cyan, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(selection);
    }
}




QRect RegionSelector::selectedRegion() const {
    return selection;
}
