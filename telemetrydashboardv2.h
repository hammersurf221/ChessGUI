#ifndef TELEMETRYDASHBOARDV2_H
#define TELEMETRYDASHBOARDV2_H

#include <QDockWidget>
#include <QList>

class QLabel;
class QTableWidget;
class QProgressBar;
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
    QLabel *bestMoveLabel;
    QLabel *averageCpLabel;
    QTableWidget *rankTable;
    QList<QProgressBar *> thinkBars;
    QPushButton *clearButton;
};

#endif // TELEMETRYDASHBOARDV2_H
