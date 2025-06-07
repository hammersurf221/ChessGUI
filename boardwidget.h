#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include "arrowoverlay.h"
#include <QWidget>
#include <QLabel>
#include <QMap>
#include <QResizeEvent>


class BoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget *parent = nullptr);
    void setPositionFromFen(const QString &fen, bool flipped);
    void setArrows(const QList<QPair<QString, QString>>& newArrows);
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    bool hasHeightForWidth() const override;
    int heightForWidth(int w) const override;

private:
    QLabel* boardBackground;
    QMap<QString, QLabel*> pieceLabels; // key = square like "e4"
    QString currentFen;
    bool currentFlipped = false;

    QString squareToKey(int rank, int file) const;
    QPoint squareToPosition(const QString &square, bool flipped) const;
    void updatePieces(const QString &fen, bool flipped);
    void clearAllPieces();
    QRect boardRect() const;
    QList<QPair<QString, QString>> arrows;

    ArrowOverlay* arrowOverlay;
    QString highlightFrom;
    QString highlightTo;
    QSize sizeHint() const override;
    QPixmap originalBoardPixmap;


};

#endif // BOARDWIDGET_H
