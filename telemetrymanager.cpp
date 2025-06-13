#include "telemetrymanager.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QDate>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>


TelemetryManager::TelemetryManager(QObject *parent)
    : QObject(parent),
    firstEntry(true)
{
    QString path = QCoreApplication::applicationDirPath() + "/telemetry_log.json";
    QFileInfo info(path);
    if (info.exists() && info.size() > 5 * 1024 * 1024) {
        QString rotated = QCoreApplication::applicationDirPath() +
                          "/telemetry_log_" + QDate::currentDate().toString("yyyyMMdd") + ".json";
        QFile::rename(path, rotated);
    }

    logFile.setFileName(path);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        logFile.write("[");
        logFile.flush();
    }
}

TelemetryManager::~TelemetryManager() {
    if (logFile.isOpen()) {
        logFile.write("\n]\n");
        logFile.close();
    }
}

void TelemetryManager::logEntry(const TelemetryEntry &entry) {
    entries.append(entry);
    if (!logFile.isOpen())
        return;

    if (!firstEntry)
        logFile.write(",\n");
    firstEntry = false;

    QJsonObject obj;
    obj["timestamp"] = entry.timestamp;
    obj["fen"] = entry.fen;
    obj["move"] = entry.move;
    obj["rank"] = entry.rank;
    obj["evalPlayed"] = entry.evalPlayed;
    obj["evalBest"] = entry.evalBest;
    obj["cpDelta"] = entry.cpDelta;
    obj["thinkTimeMs"] = entry.thinkTimeMs;

    QJsonDocument doc(obj);
    logFile.write(doc.toJson(QJsonDocument::Compact));
    logFile.flush();
}

void TelemetryManager::clearLog() {
    if (logFile.isOpen()) {
        logFile.close();
    }
    QFile::remove(logFile.fileName());
    entries.clear();
    firstEntry = true;
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        logFile.write("[");
        logFile.flush();
    }
}

double TelemetryManager::bestMovePercent() const {
    if (entries.isEmpty())
        return 0.0;
    int best = 0;
    for (const TelemetryEntry &e : entries) {
        if (e.rank == 1)
            ++best;
    }
    return 100.0 * best / entries.size();
}

double TelemetryManager::averageCpDelta() const {
    if (entries.isEmpty())
        return 0.0;
    long long sum = 0;
    for (const TelemetryEntry &e : entries)
        sum += e.cpDelta;
    return double(sum) / entries.size();
}

QMap<int,int> TelemetryManager::rankCounts() const {
    QMap<int,int> counts;
    for (const TelemetryEntry &e : entries)
        counts[e.rank] = counts.value(e.rank,0) + 1;
    return counts;
}

QList<int> TelemetryManager::recentThinkTimes(int max) const {
    QList<int> times;
    int start = qMax(0, entries.size() - max);
    for (int i=start; i<entries.size(); ++i)
        times.append(entries[i].thinkTimeMs);
    return times;
}

