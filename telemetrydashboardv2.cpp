#include "telemetrydashboardv2.h"
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QProgressBar>
#include <QPushButton>
#include <QMap>
#include <QVBoxLayout>
#include <QWidget>
#include "telemetrymanager.h"

TelemetryDashboardV2::TelemetryDashboardV2(QWidget *parent)
    : QDockWidget(parent)
{
    QWidget *container = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(container);

    bestMoveLabel = new QLabel(tr("Best Move %: N/A"), container);
    averageCpLabel = new QLabel(tr("Average cpDelta: N/A"), container);

    rankTable = new QTableWidget(0, 2, container);
    QStringList headers;
    headers << tr("Rank") << tr("Count");
    rankTable->setHorizontalHeaderLabels(headers);
    rankTable->horizontalHeader()->setStretchLastSection(true);
    rankTable->verticalHeader()->setVisible(false);

    for (int i = 0; i < 5; ++i) {
        QProgressBar *bar = new QProgressBar(container);
        bar->setRange(0, 100);
        thinkBars.append(bar);
    }

    clearButton = new QPushButton(tr("Clear Telemetry"), container);

    layout->addWidget(bestMoveLabel);
    layout->addWidget(averageCpLabel);
    layout->addWidget(rankTable);
    for (QProgressBar *bar : thinkBars)
        layout->addWidget(bar);
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

    bestMoveLabel->setText(tr("Best Move %: %1")
                           .arg(manager->bestMovePercent(), 0, 'f', 1));
    averageCpLabel->setText(tr("Average cpDelta: %1")
                            .arg(manager->averageCpDelta(), 0, 'f', 1));

    QMap<int, int> counts = manager->rankCounts();
    rankTable->setRowCount(counts.size());
    int row = 0;
    for (auto it = counts.cbegin(); it != counts.cend(); ++it, ++row) {
        rankTable->setItem(row, 0,
                           new QTableWidgetItem(QString::number(it.key())));
        rankTable->setItem(row, 1,
                           new QTableWidgetItem(QString::number(it.value())));
    }

    QList<int> times = manager->recentThinkTimes(thinkBars.size());
    for (int i = 0; i < thinkBars.size() && i < times.size(); ++i)
        thinkBars[i]->setValue(times[i]);
}
