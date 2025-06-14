#ifndef TELEMETRYDASHBOARDV2_H
#define TELEMETRYDASHBOARDV2_H

#include <QDockWidget>
#include <QVector>
#include <QProgressBar>


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
    QVector<QProgressBar*> bars;
    QPushButton *clearButton;
};

#endif // TELEMETRYDASHBOARDV2_H
