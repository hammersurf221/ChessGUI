#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "regionselector.h"
#include "chessboard_detector.h"
#include "boardwidget.h"
#include "settingsdialog.h"
#include <QLabel>
#include <QMainWindow>
#include <QTimer>
#include <QRect>
#include <QProcess>
#include <QQueue>
#include <QMap>
#include <QPair>
#include <QElapsedTimer>
#include <QVariantAnimation>
#include <QRandomGenerator>
#include <QDateTime>
#include "globalhotkeymanager.h"
#include "telemetrymanager.h"
#include "telemetrydashboardv2.h"
#include "stealthmoveselector.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_setRegionButton_clicked();
    void on_toggleAnalysisButton_clicked();
    void on_resetGameButton_clicked();
    void openSettings();
    void clearTelemetryLog();

private:
    Ui::MainWindow *ui;
    QRect captureRegion;
    QTimer* screenshotTimer;
    bool analysisRunning = false;
    QProcess* pythonProcess = nullptr;
    QProcess* engineProcess = nullptr;
    QString lastFen;
    int analysisInterval = 1000;  // milliseconds
    int engineDepth = 15;
    int autoMoveDelayMs = 0;
    bool autoMoveWhenReady = false;
    bool useAutoBoardDetectionSetting = true;
    bool forceManualRegionSetting = false;
    QString enginePath;
    QString fenModelPath;
    QString getMyColor() const;
    void captureScreenshot();
    void runFenPrediction(const QString& imagePath);
    QProcess* fenServer = nullptr;
    QString myColor = "w";
    void startEngine();
    void evaluatePosition(const QString& fen);
    QRect autoDetectedRegion;
    QDialog* autoOverlay = nullptr;
    BoardWidget* board = nullptr;
    void setStatusLight(const QString& color);
    void updateStatusLabel(const QString& text);
    void startFenServer();
    QLabel* evalScoreLabel = nullptr;
    QVariantAnimation* evalAnimation = nullptr;
    void setEvalBarValue(int value);
    int scaleEval(int cp) const;
    void updateEvalLabel();
    QString boardTurnColor;   // "w" or "b"
    QString weightsPath;
    double stealthTemperature = 0.035;
    int stealthInjectPct = 10;
    QString engineStrength = "Unrestricted";

    MoveCandidate pickBestMove(bool stealth, double temperature, int injectPct);
    SettingsDialog* settingsDialog = nullptr;
    QString currentBestMove;
    void playBestMove();
    void playMove(const QString &uci);
    void updateRepetitionTable(const QString &fen, bool afterMove = true);
    bool isMyTurn = false;
    QString lastEvaluatedFen;
    QQueue<QString> recentBestMoves;
    QString lastPlayedFen;
    QString lastOwnMove;
    QHash<QString, int> repetitionTable;
    bool automoveInProgress = false;
    QMap<int, QPair<QString, int>> multipvMoves;
    QMap<QString, double> policyProbMap;
    int selectedBestMoveRank = 1;
    double accuracy = 0.9;
    QElapsedTimer screenshotElapsed;
    QElapsedTimer fenElapsed;
    QElapsedTimer evalElapsed;
    int lastBestScore = 0;
    int lastMoveComplexity = 0;
    quint32 randomSeed = 0;
    QRandomGenerator randomGenerator;
    GlobalHotkeyManager* hotkeyManager = nullptr;

    TelemetryManager* telemetryManager = nullptr;
    TelemetryDashboardV2* telemetryDock = nullptr;
    TelemetryEntry pendingTelemetry;


    QStringList moveHistoryLines;
    QString detectUciMove(const QString& prevFen, const QString& currFen) const;
    int addMoveToHistory(const QString& moveUci, bool whiteMove);
    void appendEvalChangeToHistory(int index, double delta);

    double lastEvalForMe = 0.0;
    bool lastEvalValid = false;
    int pendingEvalLine = -1;

    bool restartEngineOnCrash = true;
    bool restartFenServerOnCrash = true;

    void setRandomSeed(quint32 seed) { randomSeed = seed; randomGenerator.seed(seed); }



protected:
    bool eventFilter(QObject* obj, QEvent* event) override;




};
#endif // MAINWINDOW_H
