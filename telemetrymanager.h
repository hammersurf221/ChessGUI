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
    int rank = 0;
    int evalPlayed = 0;
    int evalBest = 0;
    int cpDelta = 0;
    int thinkTimeMs = 0;
};

class TelemetryManager : public QObject {
    Q_OBJECT
public:
    explicit TelemetryManager(QObject *parent = nullptr);
    ~TelemetryManager();

    void logEntry(const TelemetryEntry &entry);

    double bestMovePercent() const;
    double averageCpDelta() const;
    QMap<int,int> rankCounts() const;
    QList<int> recentThinkTimes(int max = 10) const;

private:
    QFile logFile;
    QVector<TelemetryEntry> entries;
    bool firstEntry = true;
};

#endif // TELEMETRYMANAGER_H
