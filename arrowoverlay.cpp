#include "arrowoverlay.h"
#include <QPainter>
#include <cmath>

ArrowOverlay::ArrowOverlay(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(512, 512);  // Match board size
}

void ArrowOverlay::setArrows(const QList<QPair<QString, QString>>& newArrows, bool flip) {
    arrows = newArrows;
    flipped = flip;
    update();
}

QPoint ArrowOverlay::squareToPosition(const QString& square) const {
    int file = square[0].unicode() - 'a';
    int rank = 8 - square[1].digitValue();

    if (flipped) {
        file = 7 - file;
        rank = 7 - rank;
    }

    return QPoint(file * 64, rank * 64);
}

void ArrowOverlay::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const auto& arrow : arrows) {
        QPoint from = squareToPosition(arrow.first);
        QPoint to = squareToPosition(arrow.second);

        QPoint fromCenter = from + QPoint(32, 32);
        QPoint toCenter = to + QPoint(32, 32);

        QPen pen(QColor("#66cc88"));  // Teal color
        pen.setWidth(8);              // Thicker line
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);
        painter.drawLine(fromCenter, toCenter);

        double angle = std::atan2(toCenter.y() - fromCenter.y(), toCenter.x() - fromCenter.x());
        QPointF arrowP1 = toCenter - QPointF(15 * std::cos(angle - M_PI / 6),
                                             15 * std::sin(angle - M_PI / 6));
        QPointF arrowP2 = toCenter - QPointF(15 * std::cos(angle + M_PI / 6),
                                             15 * std::sin(angle + M_PI / 6));

        QPolygonF head;
        head << toCenter << arrowP1 << arrowP2;
        painter.setBrush(QColor("#66cc88"));
        painter.drawPolygon(head);
    }
    qDebug() << "Drawing arrows, count =" << arrows.size();
}
