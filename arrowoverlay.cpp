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

void ArrowOverlay::setHighlights(const QString& from, const QString& to) {
    highlightFrom = from;
    highlightTo = to;
    update();
}

QPoint ArrowOverlay::squareToPosition(const QString& square) const {
    int file = square[0].unicode() - 'a';
    int rank = 8 - square[1].digitValue();

    if (flipped) {
        file = 7 - file;
        rank = 7 - rank;
    }

    int tileSize = width() / 8;
    return QPoint(file * tileSize, rank * tileSize);
}

void ArrowOverlay::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int tileSize = width() / 8;

    auto drawHighlight = [&](const QString& square) {
        if (square.isEmpty())
            return;
        QPoint pos = squareToPosition(square);
        QRect rect(pos, QSize(tileSize, tileSize));
        painter.fillRect(rect, QColor(255, 215, 0, 120));
    };

    drawHighlight(highlightFrom);
    drawHighlight(highlightTo);

    for (const auto& arrow : arrows) {
        QPoint from = squareToPosition(arrow.first);
        QPoint to = squareToPosition(arrow.second);
        QPoint fromCenter = from + QPoint(tileSize / 2, tileSize / 2);
        QPoint toCenter = to + QPoint(tileSize / 2, tileSize / 2);

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
