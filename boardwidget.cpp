#include "boardwidget.h"
#include <QPixmap>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSvgRenderer>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QVector>
#include <QResizeEvent>



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

    originalBoardPixmap = boardPixmap;  // ✅ Store original unscaled image

    // 🚫 Don't use event->size() in constructor
    boardBackground->setPixmap(originalBoardPixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));




}

QString BoardWidget::squareToKey(int rank, int file) const {
    return QString("%1%2").arg(QChar('a' + file)).arg(8 - rank);
}

QPoint BoardWidget::squareToPosition(const QString &square, bool flipped) const {
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
    for (auto* label : pieceLabels.values()) {
        label->hide();
        label->deleteLater();
    }
    pieceLabels.clear();
}

void BoardWidget::setPositionFromFen(const QString &fen, bool flipped) {
    if (fen == currentFen && flipped == currentFlipped)
        return;

    QString prevFen = currentFen;
    currentFen = fen;
    currentFlipped = flipped;
    updatePieces(fen, flipped);

    QString highlightFrom;
    QString highlightTo;

    highlightFrom.clear();
    highlightTo.clear();

    if (!prevFen.isEmpty()) {
        auto parseBoard = [](const QString& layout) {
            QVector<QVector<QChar>> board(8, QVector<QChar>(8, '.'));
            QStringList ranks = layout.split('/');
            for (int r = 0; r < qMin(8, ranks.size()); ++r) {
                int c = 0;
                for (QChar ch : ranks[r]) {
                    if (ch.isDigit()) {
                        c += ch.digitValue();
                    } else {
                        if (c < 8) board[r][c] = ch;
                        ++c;
                    }
                }
            }
            return board;
        };

        auto prevBoard = parseBoard(prevFen);
        auto currBoard = parseBoard(fen);

        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                if (prevBoard[r][c] != currBoard[r][c]) {
                    if (prevBoard[r][c] != '.' && currBoard[r][c] == '.') {
                        highlightFrom = QString(QChar('a' + c)) + QString::number(8 - r);
                    } else if (prevBoard[r][c] == '.' && currBoard[r][c] != '.') {
                        highlightTo = QString(QChar('a' + c)) + QString::number(8 - r);
                    } else if (prevBoard[r][c] != currBoard[r][c] && currBoard[r][c] != '.') {
                        highlightTo = QString(QChar('a' + c)) + QString::number(8 - r);
                        if (highlightFrom.isEmpty())
                            highlightFrom = QString(QChar('a' + c)) + QString::number(8 - r);
                    }
                }
            }
        }
    }


    if (arrowOverlay) {
        arrowOverlay->setHighlights(highlightFrom, highlightTo);
        arrowOverlay->raise();
    }

    update();

    update();

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
        qreal tileWidth = static_cast<qreal>(width()) / 8.0;
        qreal tileHeight = static_cast<qreal>(height()) / 8.0;
        int pieceSize = qRound(qMin(tileWidth, tileHeight));
        QPixmap pixmap(pieceSize, pieceSize);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        renderer.render(&painter, QRectF(0, 0, pieceSize, pieceSize));

        pieceLabel->setPixmap(pixmap);
        pieceLabel->setFixedSize(pieceSize, pieceSize);
        int tileW = width() / 8;
        int tileH = height() / 8;
        QPoint squarePos = squareToPosition(key, flipped);
        int centeredX = squarePos.x() + (tileW - pieceSize) / 2;
        int centeredY = squarePos.y() + (tileH - pieceSize) / 2;
        pieceLabel->move(centeredX, centeredY);
        pieceLabel->show();

        pieceLabels.insert(key, pieceLabel);
        file++;
    }
}




void BoardWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);  // draw base

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);



    qreal tileWidth = static_cast<qreal>(width()) / 8.0;
    qreal tileHeight = static_cast<qreal>(height()) / 8.0;

    auto drawHighlight = [&](const QString& square) {
        if (square.isEmpty()) return;
        QPoint pos = squareToPosition(square, currentFlipped);
        QRect rect(pos, QSize(qRound(tileWidth), qRound(tileHeight)));
        painter.fillRect(rect, QColor(255, 215, 0, 120));
    };

    drawHighlight(highlightFrom);
    drawHighlight(highlightTo);


    if (arrows.isEmpty())
        return;

    for (const auto& arrow : arrows) {
        QString fromSquare = arrow.first;
        QString toSquare = arrow.second;

        QPoint fromPos = squareToPosition(fromSquare, currentFlipped);
        QPoint toPos = squareToPosition(toSquare, currentFlipped);

        // Offset to center of squares
        QPoint fromCenter = fromPos + QPoint(qRound(tileWidth / 2.0), qRound(tileHeight / 2.0));
        QPoint toCenter = toPos + QPoint(qRound(tileWidth / 2.0), qRound(tileHeight / 2.0));

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

void BoardWidget::resizeEvent(QResizeEvent* event) {
    boardBackground->resize(event->size());
    boardBackground->setPixmap(originalBoardPixmap.scaled(event->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    if (arrowOverlay)
        arrowOverlay->resize(event->size());

    updatePieces(currentFen, currentFlipped);
    QWidget::resizeEvent(event);
}


QSize BoardWidget::sizeHint() const {
    int side = qMin(width(), height());
    return QSize(side, side);
}



