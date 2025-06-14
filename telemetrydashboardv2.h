#ifndef TELEMETRYDASHBOARDV2_H
#define TELEMETRYDASHBOARDV2_H

#include <QDockWidget>
#include <QList>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>


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
    QtCharts::QChartView *chartView;
    QtCharts::QBarSeries *series;
    QtCharts::QBarSet *barSet;
    QtCharts::QValueAxis *axisY;
    QPushButton *clearButton;
};

#endif // TELEMETRYDASHBOARDV2_H
