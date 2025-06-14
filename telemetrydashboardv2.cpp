#include "telemetrydashboardv2.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include "telemetrymanager.h"

TelemetryDashboardV2::TelemetryDashboardV2(QWidget *parent)
    : QDockWidget(parent)
{
    QWidget *container = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(container);

    averageLabel = new QLabel(tr("Average Human-likeness: N/A"), container);

    barSet = new QtCharts::QBarSet(tr("Moves"));
    series = new QtCharts::QBarSeries();
    series->append(barSet);

    QStringList categories;
    categories << "<0.2" << "0.2–0.4" << "0.4–0.6" << "0.6–0.8" << ">=0.8";
    QtCharts::QBarCategoryAxis *axisX = new QtCharts::QBarCategoryAxis();
    axisX->append(categories);
    axisY = new QtCharts::QValueAxis();
    axisY->setRange(0, 1);

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chartView = new QtCharts::QChartView(chart, container);
    chartView->setRenderHint(QPainter::Antialiasing);

    clearButton = new QPushButton(tr("Clear Telemetry"), container);

    layout->addWidget(averageLabel);
    layout->addWidget(chartView);
    layout->addStretch();
    layout->addWidget(clearButton);

    container->setLayout(layout);
    setWidget(container);

    connect(clearButton, &QPushButton::clicked,
            this, &TelemetryDashboardV2::clearTelemetryRequested);
}

void TelemetryDashboardV2::refresh(TelemetryManager *manager)
{
    if (!manager)
        return;

    double avg = manager->averagePolicyProb() * 100.0;
    averageLabel->setText(tr("Average Human-likeness: %1%")
                          .arg(avg, 0, 'f', 1));

    QVector<int> counts = manager->policyHistogram();
    int maxVal = 1;
    for (int i = 0; i < counts.size(); ++i) {
        barSet->replace(i, counts[i]);
        if (counts[i] > maxVal)
            maxVal = counts[i];
    }
    axisY->setMax(maxVal);
}
