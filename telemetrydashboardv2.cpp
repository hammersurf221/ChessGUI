#include "telemetrydashboardv2.h"
#include <QLabel>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGraphs/qchartview.h>
#include <QtGraphs/qchart.h>
#include <QtGraphs/qbarset.h>
#include <QtGraphs/qbarseries.h>
#include <QtGraphs/qbarcategoryaxis.h>
#include <QtGraphs/qvalueaxis.h>
#include "telemetrymanager.h"

TelemetryDashboardV2::TelemetryDashboardV2(QWidget *parent)
    : QDockWidget(parent)
{
    QWidget *container = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(container);

    averageLabel = new QLabel(tr("Average Human-likeness: N/A"), container);

    barSet = new QtGraphs::QBarSet(tr("Moves"));
    series = new QtGraphs::QBarSeries();
    series->append(barSet);

    QStringList categories;
    categories << "<0.2" << "0.2–0.4" << "0.4–0.6" << "0.6–0.8" << ">=0.8";
    QtGraphs::QBarCategoryAxis *axisX = new QtGraphs::QBarCategoryAxis();
    axisX->append(categories);
    axisY = new QtGraphs::QValueAxis();
    axisY->setRange(0, 1);

    QtGraphs::QChart *chart = new QtGraphs::QChart();
    chart->addSeries(series);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chartView = new QtGraphs::QChartView(chart, container);
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
