#include "telemetrydashboard.h"
#include <QVBoxLayout>
#include <QHeaderView>

TelemetryDashboard::TelemetryDashboard(QWidget *parent)
    : QDockWidget(parent), bestMoveLabel(new QLabel(this)), avgDeltaLabel(new QLabel(this)),
      rankTable(new QTableWidget(3, 2, this))
{
    QWidget *container = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->addWidget(bestMoveLabel);
    layout->addWidget(avgDeltaLabel);

    rankTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QStringList headers;
    headers << "Rank" << "Count";
    rankTable->setHorizontalHeaderLabels(headers);
    rankTable->verticalHeader()->setVisible(false);
    rankTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for (int i=0;i<3;++i) {
        rankTable->setItem(i,0,new QTableWidgetItem(QString::number(i+1)));
        rankTable->setItem(i,1,new QTableWidgetItem("0"));
    }
    layout->addWidget(rankTable);

    // simple bars for think time
    for (int i=0;i<5;++i) {
        QProgressBar *bar = new QProgressBar(this);
        bar->setRange(0,5000);
        bar->setValue(0);
        thinkBars.append(bar);
        layout->addWidget(bar);
    }

    container->setLayout(layout);
    setWidget(container);
    setWindowTitle("Telemetry");
}

void TelemetryDashboard::attachManager(TelemetryManager *m)
{
    if (manager)
        disconnect(manager, nullptr, this, nullptr);
    manager = m;
    if (manager) {
        connect(manager, &TelemetryManager::entryLogged,
                this, &TelemetryDashboard::onEntryLogged);
        connect(manager, &TelemetryManager::logCleared,
                this, &TelemetryDashboard::onLogCleared);
        refresh();
    }
}

void TelemetryDashboard::onEntryLogged(const TelemetryEntry &)
{
    refresh();
}

void TelemetryDashboard::onLogCleared()
{
    refresh();
}

void TelemetryDashboard::refresh()
{
    if (!manager)
        return;
    bestMoveLabel->setText(QString("Best Move %%: %1")
                           .arg(manager->bestMovePercent(),0,'f',1));
    avgDeltaLabel->setText(QString("Avg cpDelta: %1")
                           .arg(manager->averageCpDelta(),0,'f',1));

    QMap<int,int> counts = manager->rankCounts();
    for (int i=1;i<=3;++i) {
        int val = counts.value(i,0);
        if (!rankTable->item(i-1,1))
            rankTable->setItem(i-1,1,new QTableWidgetItem(QString::number(val)));
        else
            rankTable->item(i-1,1)->setText(QString::number(val));
    }

    QList<int> times = manager->recentThinkTimes(thinkBars.size());
    for (int i=0;i<thinkBars.size(); ++i) {
        int val = 0;
        if (i < times.size())
            val = times.at(times.size()-1-i); // show latest at bottom
        thinkBars[i]->setValue(val);
    }
}

