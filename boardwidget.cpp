#include "boardwidget.h"
#include <QBrush>
#include <QDir>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>
#include <QResizeEvent>

QPixmap BoardWidget::generateBoardPixmap(int width, int height) const {
  QPixmap pixmap(width, height);
  pixmap.fill(Qt::transparent);
  QPainter p(&pixmap);
  QColor light(234, 202, 155);
  QColor dark(181, 136, 99);
  int tileW = width / 8;
  int tileH = height / 8;
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      QRect square(col * tileW, row * tileH, tileW, tileH);
      bool isLight = (row + col) % 2 == 0;
      p.fillRect(square, isLight ? light : dark);
    }
  }
  p.end();
  return pixmap;
}

BoardWidget::BoardWidget(QWidget *parent) : QWidget(parent) {

  arrowOverlay = new ArrowOverlay(this);
  arrowOverlay->setGeometry(rect());
  arrowOverlay->raise();
  arrowOverlay->show();

  // Load static board image
  boardBackground = new QLabel(this);
  setFixedSize(512, 512);
  boardBackground->move(0, 0);
  boardBackground->lower();


  QString boardPath = "assets/board.png";
  QPixmap boardPixmap = generateBoardPixmap(512, 512);
  QDir().mkpath("assets");
  boardPixmap.save(boardPath);

  originalBoardPixmap = boardPixmap; // Store original unscaled image

  boardBackground->setFixedSize(512, 512);
  boardBackground->setPixmap(originalBoardPixmap);
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

void BoardWidget::clearAllPieces() {
  for (auto *label : pieceLabels.values()) {
    label->hide();
    label->deleteLater();
  }
  pieceLabels.clear();
}

void BoardWidget::setPositionFromFen(const QString &fen, bool flipped) {
  if (fen == currentFen && flipped == currentFlipped)
    return;

  currentFen = fen;
  currentFlipped = flipped;
  updatePieces(fen, flipped);
  update();
}

void BoardWidget::updatePieces(const QString &fen, bool flipped) {
  clearAllPieces();

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

    QString key = squareToKey(rank, file); // ranks 0 = 8th rank, 7 = 1st rank
    QString pieceCode =
        QString("%1%2").arg(c.isUpper() ? 'w' : 'b').arg(c.toUpper());

    QString piecePath = QString("assets/pieces/%1.svg").arg(pieceCode);


    QLabel *pieceLabel = new QLabel(this);
    QSvgRenderer renderer(piecePath);
    qreal tileWidth = static_cast<qreal>(width()) / 8.0;
    qreal tileHeight = static_cast<qreal>(height()) / 8.0;
    int pieceSize = qRound(qMin(tileWidth, tileHeight));
    QPixmap pixmap(pieceSize, pieceSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter, QRectF(0, 0, pieceSize, pieceSize));


    pieceLabel->setPixmap(pixmap);
    pieceLabel->setFixedSize(pieceSize, pieceSize);

    qreal tileW = static_cast<qreal>(width()) / 8.0;
    qreal tileH = static_cast<qreal>(height()) / 8.0;
    QPoint squarePos = squareToPosition(key, flipped);
    int centeredX = qRound(squarePos.x() + tileW / 2.0 - pieceSize / 2.0);
    int centeredY = qRound(squarePos.y() + tileH / 2.0 - pieceSize / 2.0);
    pieceLabel->move(centeredX, centeredY);
    pieceLabel->show();

    pieceLabels.insert(key, pieceLabel);
    file++;
  }
}

void BoardWidget::paintEvent(QPaintEvent *event) {
  QWidget::paintEvent(event);
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
    boardBackground->setFixedSize(size());
    if (arrowOverlay) {
        arrowOverlay->setGeometry(rect());
        arrowOverlay->raise();
    }
    updatePieces(currentFen, currentFlipped);
}

