#ifndef TELEMETRYMANAGER_H
#define TELEMETRYMANAGER_H

#include <QObject>
#include <QFile>
#include <QVector>
#include <QMap>
#include <QJsonObject>

struct TelemetryEntry {
    QString timestamp;
    QString fen;
    QString move;
    double policyProb = 0.0; // [0,1]
    int thinkTimeMs = 0;
};

class TelemetryManager : public QObject
{
    Q_OBJECT
public:
    explicit TelemetryManager(QObject *parent = nullptr);
    ~TelemetryManager();

    void logEntry(const TelemetryEntry &entry);
    void clearLog();

    double averagePolicyProb() const; // average over [0,1]
    QVector<int> policyHistogram() const; // 5 bins

signals:
    void entryLogged(const TelemetryEntry &entry);
    void logCleared();

private:
    QFile logFile;
    QVector<TelemetryEntry> entries;
    bool firstEntry = true;
};

#endif // TELEMETRYMANAGER_H
