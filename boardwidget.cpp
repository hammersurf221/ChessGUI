#include "boardwidget.h"
#include <QPixmap>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSvgRenderer>
#include <QPainter>
#include <QPen>
#include <QBrush>



BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent)
{

    arrowOverlay = new ArrowOverlay(this);
    arrowOverlay->resize(this->size());
    arrowOverlay->raise();  // Ensure it is on top
    arrowOverlay->show();



    // Load static board image
    boardBackground = new QLabel(this);
    setMinimumSize(512, 512);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    boardBackground->move(0, 0);
    boardBackground->lower(); // stays behind all pieces

    QString boardPath = "assets/board.png";
    QPixmap boardPixmap;

    if (QFile::exists(boardPath)) {
        boardPixmap.load(boardPath);
    } else {
        // Generate the board image
        boardPixmap = QPixmap(512, 512);
        boardPixmap.fill(Qt::transparent);

        QPainter p(&boardPixmap);

        QColor light(234, 202, 155);  // light wood
        QColor dark(181, 136, 99);    // dark wood

        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                QRect square(col * 64, row * 64, 64, 64);
                bool isLight = (row + col) % 2 == 0;
                p.fillRect(square, isLight ? light : dark);
            }
        }

        p.end();

        // Save it for future use
        QDir().mkpath("assets");
        boardPixmap.save(boardPath);
    }

    boardBackground->setPixmap(boardPixmap);
    boardBackground->setScaledContents(true);

}

QString BoardWidget::squareToKey(int rank, int file) const {
    return QString("%1%2").arg(QChar('a' + file)).arg(8 - rank);
}

QPoint BoardWidget::squareToPosition(const QString &square, bool flipped) const {
    int file = square[0].unicode() - 'a'; // a–h → 0–7
    int rank = 8 - square[1].digitValue(); // 1–8 → 7–0

    int tileSize = width() / 8;

    if (flipped) {
        file = 7 - file;
        rank = 7 - rank;
    }

    int x = file * tileSize;
    int y = rank * tileSize;
    return QPoint(x, y);
}

void BoardWidget::clearAllPieces() {
    for (auto* label : pieceLabels.values()) {
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

    if (arrowOverlay)
        arrowOverlay->raise();  // 🟢 Ensure overlay is always on top after updates
}


void BoardWidget::updatePieces(const QString &fen, bool flipped) {
    clearAllPieces();

    QStringList parts = fen.split(" ");
    if (parts.isEmpty()) return;

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
        QString pieceCode = QString("%1%2")
                                .arg(c.isUpper() ? 'w' : 'b')
                                .arg(c.toUpper());

        QString piecePath = QString("assets/pieces/%1.svg").arg(pieceCode);

        QLabel* pieceLabel = new QLabel(this);
        QSvgRenderer renderer(piecePath);
        int tileSize = width() / 8;
        QPixmap pixmap(tileSize, tileSize);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        renderer.render(&painter, QRectF(0, 0, tileSize, tileSize));

        pieceLabel->setPixmap(pixmap);
        pieceLabel->setFixedSize(tileSize, tileSize);
        pieceLabel->move(squareToPosition(key, flipped));  // ← render visually flipped
        pieceLabel->show();

        pieceLabels.insert(key, pieceLabel);
        file++;
    }
}




void BoardWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);  // draw base

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int tileSize = width() / 8;

    QColor highlightColor(255, 255, 0, 100);
    if (!lastMoveFrom.isEmpty()) {
        QPoint pos = squareToPosition(lastMoveFrom, currentFlipped);
        painter.fillRect(QRect(pos, QSize(tileSize, tileSize)), highlightColor);
    }
    if (!lastMoveTo.isEmpty()) {
        QPoint pos = squareToPosition(lastMoveTo, currentFlipped);
        painter.fillRect(QRect(pos, QSize(tileSize, tileSize)), highlightColor);
    }

    if (arrows.isEmpty()) return;



    for (const auto& arrow : arrows) {
        QString fromSquare = arrow.first;
        QString toSquare = arrow.second;

        QPoint fromPos = squareToPosition(fromSquare, currentFlipped);
        QPoint toPos = squareToPosition(toSquare, currentFlipped);

        // Offset to center of squares
        QSize squareSize(width() / 8, height() / 8);
        QPoint fromCenter = fromPos + QPoint(squareSize.width() / 2, squareSize.height() / 2);
        QPoint toCenter = toPos + QPoint(squareSize.width() / 2, squareSize.height() / 2);

        // Draw arrow line
        QPen pen(QColor(255, 0, 0, 200));  // semi-transparent red
        pen.setWidth(4);
        painter.setPen(pen);
        painter.drawLine(fromCenter, toCenter);

        // Draw arrowhead
        double angle = std::atan2(toCenter.y() - fromCenter.y(), toCenter.x() - fromCenter.x());
        QPointF arrowP1 = toCenter - QPointF(15 * std::cos(angle - M_PI / 6),
                                             15 * std::sin(angle - M_PI / 6));
        QPointF arrowP2 = toCenter - QPointF(15 * std::cos(angle + M_PI / 6),
                                             15 * std::sin(angle + M_PI / 6));

        QPolygonF arrowHead;
        arrowHead << toCenter << arrowP1 << arrowP2;
        painter.setBrush(QColor(255, 0, 0, 200));
        painter.drawPolygon(arrowHead);
    }
}


void BoardWidget::setArrows(const QList<QPair<QString, QString>>& newArrows) {
    if (arrowOverlay)
        arrowOverlay->setArrows(newArrows, currentFlipped);
}

void BoardWidget::setLastMoveSquares(const QString& from, const QString& to) {
    lastMoveFrom = from;
    lastMoveTo = to;
    update();
}

QSize BoardWidget::sizeHint() const {
    int side = qMin(width(), height());
    return QSize(side, side);
}



