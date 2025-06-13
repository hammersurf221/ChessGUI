#ifndef TELEMETRYDASHBOARD_H
#define TELEMETRYDASHBOARD_H

#include <QDockWidget>
#include <QLabel>
#include <QTableWidget>
#include <QProgressBar>
#include "telemetrymanager.h"

class TelemetryDashboard : public QDockWidget {
    Q_OBJECT
public:
    explicit TelemetryDashboard(QWidget *parent = nullptr);

    void refresh(TelemetryManager *manager);

private:
    QLabel *bestMoveLabel;
    QLabel *avgDeltaLabel;
    QTableWidget *rankTable;
    QList<QProgressBar*> thinkBars;
};

#endif // TELEMETRYDASHBOARD_H
