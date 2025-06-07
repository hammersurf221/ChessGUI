#include "arrowoverlay.h"
#include <QPainter>
#include <QRect>
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

  QRect rect = boardRect();
  qreal tileWidth = static_cast<qreal>(rect.width()) / 8.0;
  qreal tileHeight = static_cast<qreal>(rect.height()) / 8.0;
  int x = rect.left() + qRound(file * tileWidth);
  int y = rect.top() + qRound(rank * tileHeight);
  return QPoint(x, y);
}

QRect ArrowOverlay::boardRect() const {
  int side = qMin(width(), height());
  int left = (width() - side) / 2;
  int top = (height() - side) / 2;
  return QRect(left, top, side, side);
}

void ArrowOverlay::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QRect rect = boardRect();
  qreal tileWidth = static_cast<qreal>(rect.width()) / 8.0;
  qreal tileHeight = static_cast<qreal>(rect.height()) / 8.0;

  auto drawHighlight = [&](const QString &square) {
    if (square.isEmpty())
      return;
    int file = square[0].unicode() - 'a';
    int rank = 8 - square[1].digitValue();
    if (flipped) {
      file = 7 - file;
      rank = 7 - rank;
    }
    int left = rect.left() + qRound(file * tileWidth);
    int top = rect.top() + qRound(rank * tileHeight);
    int right = rect.left() + qRound((file + 1) * tileWidth);
    int bottom = rect.top() + qRound((rank + 1) * tileHeight);
    QRect square(QPoint(left, top), QPoint(right, bottom));
    painter.fillRect(square, QColor(255, 215, 0, 120));
  };

  drawHighlight(highlightFrom);
  drawHighlight(highlightTo);

  for (const auto &arrow : arrows) {
    QPoint from = squareToPosition(arrow.first);
    QPoint to = squareToPosition(arrow.second);
    QPoint fromCenter =
        from + QPoint(qRound(tileWidth / 2.0), qRound(tileHeight / 2.0));
    QPoint toCenter =
        to + QPoint(qRound(tileWidth / 2.0), qRound(tileHeight / 2.0));

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
  qDebug() << "Drawing arrows, count =" << arrows.size();
}
