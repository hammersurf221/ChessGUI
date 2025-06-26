#ifndef STEALTHMOVESELECTOR_H
#define STEALTHMOVESELECTOR_H

#include <QVector>
#include <QString>
#include <QRandomGenerator>

struct MoveCandidate {
    QString move;   // e.g. "e2e4"
    int rank;       // 1 = best, 2 = second-best, etc.
    int score;      // evaluation in centipawns
    double policyProb = 0.0; // optional (for SABLE)
};

class StealthMoveSelector
{
public:
    StealthMoveSelector();

    void setTargetACPL(double acpl);
    void setInjectFrequency(int everyNMoves);      // default = 15
    void setStealthParams(double temperature, int cpTolerance);

    MoveCandidate pickMove(const QVector<MoveCandidate>& multipv,
                           int bestScore,
                           int moveCount,
                           const QString& fen);

private:
    double targetACPL = 60.0;
    int injectEveryN = 15;
    double temperature = 0.035;
    int cpTolerance = 100;
    QRandomGenerator rng;
};

#endif // STEALTHMOVESELECTOR_H
