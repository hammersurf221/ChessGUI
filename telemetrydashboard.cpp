#include "telemetrydashboard.h"
#include <QVBoxLayout>
#include <QHeaderView>

TelemetryDashboard::TelemetryDashboard(QWidget *parent)
    : QDockWidget(parent), summaryLabel(new QLabel(this)), entryTable(new QTableWidget(this))
{
    QWidget *container = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->addWidget(summaryLabel);

    QStringList headers;
    headers << "Time" << "Move" << "Rank" << "cpΔ" << "Think ms";
    entryTable->setColumnCount(headers.size());
    entryTable->setHorizontalHeaderLabels(headers);
    entryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    entryTable->verticalHeader()->setVisible(false);
    entryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(entryTable);

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
        connect(manager, &TelemetryManager::entryLogged, this, &TelemetryDashboard::onEntryLogged);
        connect(manager, &TelemetryManager::logCleared, this, &TelemetryDashboard::onLogCleared);
        onLogCleared();
    }
}

void TelemetryDashboard::onEntryLogged(const TelemetryEntry &entry)
{
    addEntry(entry);
    updateSummary();
}

void TelemetryDashboard::onLogCleared()
{
    entryTable->clearContents();
    entryTable->setRowCount(0);
    updateSummary();
}

void TelemetryDashboard::addEntry(const TelemetryEntry &e)
{
    int row = entryTable->rowCount();
    entryTable->insertRow(row);
    entryTable->setItem(row, 0, new QTableWidgetItem(e.timestamp));
    entryTable->setItem(row, 1, new QTableWidgetItem(e.move));
    entryTable->setItem(row, 2, new QTableWidgetItem(QString::number(e.rank)));
    entryTable->setItem(row, 3, new QTableWidgetItem(QString::number(e.cpDelta)));
    entryTable->setItem(row, 4, new QTableWidgetItem(QString::number(e.thinkTimeMs)));
    entryTable->scrollToBottom();
}

void TelemetryDashboard::updateSummary()
{
    if (!manager) {
        summaryLabel->setText("No telemetry");
        return;
    }
    double bestPct = manager->bestMovePercent();
    double avgDelta = manager->averageCpDelta();
    QMap<int, int> counts = manager->rankCounts();
    QString summary = QString("Best %1% | Avg Δcp %2 | 1st %3 2nd %4 3rd %5")
                           .arg(bestPct, 0, 'f', 1)
                           .arg(avgDelta, 0, 'f', 1)
                           .arg(counts.value(1))
                           .arg(counts.value(2))
                           .arg(counts.value(3));
    summaryLabel->setText(summary);
}

