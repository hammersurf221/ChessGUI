#include "arrowoverlay.h"
#include <QPainter>
#include <cmath>

ArrowOverlay::ArrowOverlay(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setAttribute(Qt::WA_NoSystemBackground);
  setAttribute(Qt::WA_TranslucentBackground);
}

void ArrowOverlay::setArrows(const QList<QPair<QString, QString>> &newArrows,
                             bool flip) {
  arrows = newArrows;
  flipped = flip;
  update();
}

void ArrowOverlay::setHighlights(const QString &from, const QString &to) {
  highlightFrom = from;
  highlightTo = to;
  update();
}

QPoint ArrowOverlay::squareToPosition(const QString &square) const {
  int file = square[0].unicode() - 'a';
  int rank = 8 - square[1].digitValue();



  if (flipped) {
    file = 7 - file;
    rank = 7 - rank;
  }


  qreal tileWidth = static_cast<qreal>(width()) / 8.0;
  qreal tileHeight = static_cast<qreal>(height()) / 8.0;
  int x = qRound(file * tileWidth);
  int y = qRound(rank * tileHeight);

  return QPoint(x, y);
}

QPoint ArrowOverlay::squareCenter(const QString &square) const {
  int file = square[0].unicode() - 'a';
  int rank = 8 - square[1].digitValue();

  if (flipped) {
    file = 7 - file;
    rank = 7 - rank;
  }

  qreal tileWidth = static_cast<qreal>(width()) / 8.0;
  qreal tileHeight = static_cast<qreal>(height()) / 8.0;
  int x = qRound((file + 0.5) * tileWidth);
  int y = qRound((rank + 0.5) * tileHeight);

  return QPoint(x, y);
}

void ArrowOverlay::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);


  qreal tileWidth = static_cast<qreal>(width()) / 8.0;
  qreal tileHeight = static_cast<qreal>(height()) / 8.0;

  auto drawHighlight = [&](const QString &square) {
    if (square.isEmpty())
      return;
    int file = square[0].unicode() - 'a';
    int rank = 8 - square[1].digitValue();
    if (flipped) {
      file = 7 - file;
      rank = 7 - rank;
    }
    int left = qRound(file * tileWidth);
    int top = qRound(rank * tileHeight);
    int right = qRound((file + 1) * tileWidth);
    int bottom = qRound((rank + 1) * tileHeight);
    QRect rect(QPoint(left, top), QPoint(right, bottom));
    painter.fillRect(rect, QColor(255, 215, 0, 120));
  };


  drawHighlight(highlightFrom);
  drawHighlight(highlightTo);

  for (const auto &arrow : arrows) {
    QPoint fromCenter = squareCenter(arrow.first);
    QPoint toCenter = squareCenter(arrow.second);

    QPen pen(QColor("#66cc88")); // Teal color
    pen.setWidth(8);             // Thicker line
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawLine(fromCenter, toCenter);

    double angle = std::atan2(toCenter.y() - fromCenter.y(),
                              toCenter.x() - fromCenter.x());
    QPointF arrowP1 = toCenter - QPointF(15 * std::cos(angle - M_PI / 6),
                                         15 * std::sin(angle - M_PI / 6));
    QPointF arrowP2 = toCenter - QPointF(15 * std::cos(angle + M_PI / 6),
                                         15 * std::sin(angle + M_PI / 6));

    QPolygonF head;
    head << toCenter << arrowP1 << arrowP2;
    painter.setBrush(QColor("#66cc88"));
    painter.drawPolygon(head);
  }


}
