#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include "arrowoverlay.h"
#include <QWidget>
#include <QLabel>
#include <QMap>


class BoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget *parent = nullptr);
    void setPositionFromFen(const QString &fen, bool flipped);
    void setArrows(const QList<QPair<QString, QString>>& newArrows);
    void setLastMoveSquares(const QString& from, const QString& to);
    void paintEvent(QPaintEvent* event) override;

private:
    QLabel* boardBackground;
    QMap<QString, QLabel*> pieceLabels; // key = square like "e4"
    QString currentFen;
    bool currentFlipped = false;

    QString squareToKey(int rank, int file) const;
    QPoint squareToPosition(const QString &square, bool flipped) const;
    void updatePieces(const QString &fen, bool flipped);
    void clearAllPieces();
    QList<QPair<QString, QString>> arrows;

    ArrowOverlay* arrowOverlay;
    QString lastMoveFrom;
    QString lastMoveTo;
    QSize sizeHint() const override;

};

#endif // BOARDWIDGET_H
