#pragma once

#include <QPair>
#include <QString>
#include <QWidget>

class ArrowOverlay : public QWidget {
  Q_OBJECT
public:
  explicit ArrowOverlay(QWidget *parent = nullptr);
  void setArrows(const QList<QPair<QString, QString>> &arrows, bool flipped);
  void setHighlights(const QString &from, const QString &to);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QList<QPair<QString, QString>> arrows;
  bool flipped = false;
  QString highlightFrom;
  QString highlightTo;
  QPoint squareToPosition(const QString &square) const;
  QPoint squareCenter(const QString &square) const;
};
