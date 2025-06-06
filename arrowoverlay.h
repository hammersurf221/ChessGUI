#pragma once

#include <QWidget>
#include <QPair>
#include <QString>

class ArrowOverlay : public QWidget {
    Q_OBJECT
public:
    explicit ArrowOverlay(QWidget* parent = nullptr);
    void setArrows(const QList<QPair<QString, QString>>& arrows, bool flipped);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QList<QPair<QString, QString>> arrows;
    bool flipped = false;
    QPoint squareToPosition(const QString& square) const;
};
