#include "boardwidget.h"
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>
#include <QResizeEvent>
#include <QLinearGradient>

BoardWidget::BoardWidget(QWidget *parent) : QWidget(parent) {

  arrowOverlay = new ArrowOverlay(this);
  arrowOverlay->setGeometry(rect());
  arrowOverlay->raise();
  arrowOverlay->show();

  setAttribute(Qt::WA_OpaquePaintEvent);
}

QString BoardWidget::squareToKey(int rank, int file) const {
  return QString("%1%2").arg(QChar('a' + file)).arg(8 - rank);
}


QPoint BoardWidget::squareToPosition(const QString &square,
                                     bool flipped) const {
  int file = square[0].unicode() - 'a';  // 0–7
  int rank = 8 - square[1].digitValue(); // 0–7


  if (flipped) {
    file = 7 - file;
    rank = 7 - rank;
  }


  int tileW = width() / 8;
  int tileH = height() / 8;

  int x = file * tileW;
  int y = rank * tileH;


  return QPoint(x, y);
}

void BoardWidget::setPositionFromFen(const QString &fen, bool flipped) {
  if (fen == currentFen && flipped == currentFlipped)
    return;

  currentFen = fen;
  currentFlipped = flipped;
  parseFen(fen);
  update();
}

void BoardWidget::parseFen(const QString &fen) {
  boardPieces.clear();

  QStringList parts = fen.split(" ");
  if (parts.isEmpty())
    return;

  QString board = parts[0];

  int rank = 0, file = 0;
  for (QChar c : board) {
    if (c == '/') {
      rank++;
      file = 0;
      continue;
    }

    if (c.isDigit()) {
      file += c.digitValue();
      continue;
    }

    QString key = squareToKey(rank, file);
    boardPieces.insert(key, c);
    file++;
  }
}

void BoardWidget::preparePiecePixmaps(int size) {
    piecePixmaps.clear();
    const QStringList codes = {"wK", "wQ", "wR", "wB", "wN", "wP",
                                "bK", "bQ", "bR", "bB", "bN", "bP"};
    for (const QString &code : codes) {
        QSvgRenderer renderer(QString("assets/pieces/%1.svg").arg(code));
        QPixmap pix(size, size);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        renderer.render(&p, QRectF(0, 0, size, size));
        piecePixmaps.insert(code, pix);
    }
    cachedPieceSize = size;
}

void BoardWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    int tileW = width() / 8;
    int tileH = height() / 8;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QRect square(col * tileW, row * tileH, tileW, tileH);
            bool light = (row + col) % 2 == 0;
            QLinearGradient grad(square.topLeft(), square.bottomRight());
            QColor base = light ? lightSquare : darkSquare;
            grad.setColorAt(0.0, base.lighter(108));
            grad.setColorAt(1.0, base.darker(108));
            painter.fillRect(square, grad);
        }
    }

    int pieceSize = qRound(qMin(static_cast<qreal>(tileW), static_cast<qreal>(tileH)));
    if (pieceSize != cachedPieceSize)
        preparePiecePixmaps(pieceSize);

    for (auto it = boardPieces.constBegin(); it != boardPieces.constEnd(); ++it) {
        QString key = it.key();
        QChar c = it.value();
        QString code = QString("%1%2").arg(c.isUpper() ? 'w' : 'b').arg(c.toUpper());
        QPoint pos = squareToPosition(key, currentFlipped);
        int centeredX = qRound(pos.x() + tileW / 2.0 - pieceSize / 2.0);
        int centeredY = qRound(pos.y() + tileH / 2.0 - pieceSize / 2.0);
        painter.drawPixmap(centeredX, centeredY, piecePixmaps.value(code));
    }
}

void BoardWidget::setArrows(const QList<QPair<QString, QString>> &newArrows) {
  if (arrowOverlay) {
    arrowOverlay->setArrows(newArrows, currentFlipped);
    arrowOverlay->raise();
  }
}


QSize BoardWidget::sizeHint() const {
  return QSize(512, 512);
}

void BoardWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (arrowOverlay) {
        arrowOverlay->setGeometry(rect());
        arrowOverlay->raise();
    }
    cachedPieceSize = -1; // force regeneration of pixmaps
    update();
}

