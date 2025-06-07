#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QLabel>
#include <QMap>
#include <QWidget>
#include <QPixmap>
#include "arrowoverlay.h"

class BoardWidget : public QWidget {
  Q_OBJECT

public:
  explicit BoardWidget(QWidget *parent = nullptr);
  void setPositionFromFen(const QString &fen, bool flipped);
  void setArrows(const QList<QPair<QString, QString>> &newArrows);
  void setHighlights(const QString &from, const QString &to);
  void paintEvent(QPaintEvent *event) override;

protected:
  void resizeEvent(QResizeEvent *event) override;

private:

  QLabel *boardBackground;
  ArrowOverlay *arrowOverlay = nullptr;
  QMap<QString, QLabel *> pieceLabels; // key = square like "e4"
  QString currentFen;
  bool currentFlipped = false;
  QList<QPair<QString, QString>> currentArrows;
  QString highlightFrom;
  QString highlightTo;

  QString squareToKey(int rank, int file) const;
  QPoint squareToPosition(const QString &square, bool flipped) const;
  void updatePieces(const QString &fen, bool flipped);
  void clearAllPieces();
  QSize sizeHint() const override;
  QPixmap originalBoardPixmap;
  QPixmap generateBoardPixmap(int width, int height) const;

};

#endif // BOARDWIDGET_H
