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
#include "globalhotkeymanager.h"


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
    void openSettings();

private:
    Ui::MainWindow *ui;
    QRect captureRegion;
    QTimer* screenshotTimer;
    bool analysisRunning = false;
    QProcess* pythonProcess = nullptr;
    QProcess* stockfishProcess = nullptr;
    QString lastFen;
    int analysisInterval = 1000;  // milliseconds
    int stockfishDepth = 15;
    int autoMoveDelayMs = 0;
    bool autoMoveWhenReady = false;
    bool useAutoBoardDetectionSetting = true;
    bool forceManualRegionSetting = false;
    QString stockfishPath;
    QString fenModelPath;
    QString getMyColor() const;
    void captureScreenshot();
    void runFenPrediction(const QString& imagePath);
    QProcess* fenServer = nullptr;
    QString myColor = "w";
    void startStockfish();
    void evaluatePosition(const QString& fen);
    QRect autoDetectedRegion;
    QDialog* autoOverlay = nullptr;
    BoardWidget* board = nullptr;
    void setStatusLight(const QString& color);
    void updateStatusLabel(const QString& text);
    void startFenServer();
    QLabel* evalScoreLabel = nullptr;
    void updateEvalLabel();
    SettingsDialog* settingsDialog = nullptr;
    QString currentBestMove;
    void playBestMove();
    bool isMyTurn = false;
    QString lastEvaluatedFen;
    QQueue<QString> recentBestMoves;
    QString lastPlayedFen;
    bool automoveInProgress = false;
    QMap<int, QPair<QString, int>> multipvMoves;
    int selectedBestMoveRank = 1;
    int bestMoveCount1 = 0;
    int bestMoveCount2 = 0;
    int bestMoveCount3 = 0;
    void updateStealthGraph();
    double accuracy = 0.9;
    QElapsedTimer screenshotElapsed;
    QElapsedTimer fenElapsed;
    QElapsedTimer evalElapsed;
    GlobalHotkeyManager* hotkeyManager = nullptr;

    QStringList moveHistoryLines;
    QString detectUciMove(const QString& prevFen, const QString& currFen) const;
    int addMoveToHistory(const QString& moveUci, bool whiteMove);
    void appendEvalChangeToHistory(int index, double delta);

    double lastEvalForMe = 0.0;
    bool lastEvalValid = false;
    int pendingEvalLine = -1;

    bool restartStockfishOnCrash = true;

    QString pythonExe;
    QString pythonScript;
    QString modelPath;


protected:
    bool eventFilter(QObject* obj, QEvent* event) override;




};
#endif // MAINWINDOW_H
