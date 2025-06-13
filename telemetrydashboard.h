#ifndef TELEMETRYDASHBOARD_H
#define TELEMETRYDASHBOARD_H

#include <QDockWidget>
#include <QLabel>
#include <QTableWidget>
#include "telemetrymanager.h"

class TelemetryDashboard : public QDockWidget {
    Q_OBJECT
public:
    explicit TelemetryDashboard(QWidget *parent = nullptr);

    void attachManager(TelemetryManager *manager);

private slots:
    void onEntryLogged(const TelemetryEntry &entry);
    void onLogCleared();

private:
    void addEntry(const TelemetryEntry &entry);
    void updateSummary();

    TelemetryManager *manager = nullptr;
    QLabel *summaryLabel;
    QTableWidget *entryTable;
};

#endif // TELEMETRYDASHBOARD_H
