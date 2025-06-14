#ifndef TELEMETRYDASHBOARDV2_H
#define TELEMETRYDASHBOARDV2_H

#include <QDockWidget>
#include <QList>

namespace QtGraphs {
class QChartView;
class QBarSeries;
class QBarSet;
class QBarCategoryAxis;
class QValueAxis;
}

class QLabel;
class QPushButton;
class TelemetryManager;

class TelemetryDashboardV2 : public QDockWidget
{
    Q_OBJECT
public:
    explicit TelemetryDashboardV2(QWidget *parent = nullptr);

    void refresh(TelemetryManager *manager);

signals:
    void clearTelemetryRequested();

private:
    QLabel *averageLabel;
    QtGraphs::QChartView *chartView;
    QtGraphs::QBarSeries *series;
    QtGraphs::QBarSet *barSet;
    QtGraphs::QValueAxis *axisY;
    QPushButton *clearButton;
};

#endif // TELEMETRYDASHBOARDV2_H
