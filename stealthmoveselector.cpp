#include "stealthmoveselector.h"
#include <QtMath>

StealthMoveSelector::StealthMoveSelector()
{
    rng.seed(QRandomGenerator::securelySeeded().generate());
}

void StealthMoveSelector::setTargetACPL(double acpl)
{
    targetACPL = acpl;
}

void StealthMoveSelector::setInjectFrequency(int everyNMoves)
{
    injectEveryN = everyNMoves;
}

void StealthMoveSelector::setStealthParams(double temp, int tolerance)
{
    temperature = temp;
    cpTolerance = tolerance;
}

MoveCandidate StealthMoveSelector::pickMove(const QVector<MoveCandidate>& multipv,
                                            int bestScore,
                                            int moveCount,
                                            const QString& fen)
{
    Q_UNUSED(fen);
    MoveCandidate result;
    if (multipv.isEmpty())
        return result;

    if (injectEveryN > 0 && moveCount > 0 && moveCount % injectEveryN == 0) {
        QVector<MoveCandidate> choices;
        for (const MoveCandidate& c : multipv) {
            if (bestScore - c.score >= cpTolerance)
                choices.append(c);
        }
        if (!choices.isEmpty()) {
            int idx = rng.bounded(choices.size());
            return choices[idx];
        }
    }

    QVector<double> weights;
    weights.reserve(multipv.size());
    double sum = 0.0;
    for (const MoveCandidate& c : multipv) {
        double cpDiff = bestScore - c.score;
        double w = std::exp(-cpDiff / (temperature * 100.0));
        weights.append(w);
        sum += w;
    }
    if (sum <= 0.0)
        return multipv.first();

    double r = rng.generateDouble() * sum;
    int idx = 0;
    while (idx < weights.size() - 1 && r > weights[idx]) {
        r -= weights[idx];
        ++idx;
    }
    return multipv[idx];
}

