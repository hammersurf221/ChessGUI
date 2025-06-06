#pragma once

#include <QWidget>
#include <QRect>
#include <QMouseEvent>
#include <QPainter>

class RegionSelector : public QWidget {
    Q_OBJECT

public:
    explicit RegionSelector(QWidget* parent = nullptr);
    QRect selectedRegion() const;

signals:
    void regionSelected(const QRect& region);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QPoint origin;
    QRect selection;
    bool selecting = false;
};
