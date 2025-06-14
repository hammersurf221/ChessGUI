#include "telemetrymanager.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QDate>
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

TelemetryManager::~TelemetryManager()
{
    if (logFile.isOpen()) {
        logFile.write("\n]\n");
        logFile.close();
    }
}

void TelemetryManager::logEntry(const TelemetryEntry &entry)
{
    entries.append(entry);
    if (!logFile.isOpen()) {
        emit entryLogged(entry);
        return;
    }

    if (!firstEntry)
        logFile.write(",\n");
    firstEntry = false;

    QJsonObject obj;
    obj["timestamp"] = entry.timestamp;
    obj["fen"] = entry.fen;
    obj["move"] = entry.move;
    obj["policyProb"] = entry.policyProb;
    obj["thinkTimeMs"] = entry.thinkTimeMs;

    QJsonDocument doc(obj);
    logFile.write(doc.toJson(QJsonDocument::Compact));
    logFile.flush();

    emit entryLogged(entry);
}

void TelemetryManager::clearLog()
{
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

    emit logCleared();
}

double TelemetryManager::averagePolicyProb() const
{
    if (entries.isEmpty())
        return 0.0;
    double sum = 0.0;
    for (const TelemetryEntry &e : entries)
        sum += e.policyProb;
    return sum / entries.size();
}

QVector<int> TelemetryManager::policyHistogram() const
{
    QVector<int> bins(5, 0);
    for (const TelemetryEntry &e : entries) {
        double p = e.policyProb;
        int idx = 0;
        if (p < 0.2)
            idx = 0;
        else if (p < 0.4)
            idx = 1;
        else if (p < 0.6)
            idx = 2;
        else if (p < 0.8)
            idx = 3;
        else
            idx = 4;
        bins[idx] += 1;
    }
    return bins;
}

