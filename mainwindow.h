#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "regionselector.h"
#include "chessboard_detector.h"
#include "boardwidget.h"
#include <QLabel>
#include <QMainWindow>
#include <QTimer>
#include <QRect>
#include <QProcess>
#include <QQueue>
#include <QMap>
#include <QElapsedTimer>
#include "globalhotkeymanager.h"
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>


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

private:
    Ui::MainWindow *ui;
    QRect captureRegion;
    QTimer* screenshotTimer;
    bool analysisRunning = false;
    QProcess* pythonProcess = nullptr;
    QProcess* stockfishProcess = nullptr;
    QString lastFen;
    int analysisInterval = 1000;  // milliseconds
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
    void startFenServer();
    QLabel* evalScoreLabel = nullptr;
    void updateEvalLabel();
    void updateMoveRankChart();
    QString currentBestMove;
    void playBestMove();
    bool isMyTurn = false;
    QString lastEvaluatedFen;
    QQueue<QString> recentBestMoves;
    QString lastPlayedFen;
    bool automoveInProgress = false;
    QMap<int, QString> multipvMoves;
    int selectedBestMoveRank = 1;
    QElapsedTimer screenshotElapsed;
    QElapsedTimer fenElapsed;
    QElapsedTimer evalElapsed;
    GlobalHotkeyManager* hotkeyManager = nullptr;

    QtCharts::QChartView* moveRankChartView = nullptr;
    QtCharts::QChart* moveRankChart = nullptr;
    QtCharts::QBarSet* moveRankBarSet = nullptr;
    QVector<int> stealthMoveRankCounts = QVector<int>(4, 0);



protected:
    bool eventFilter(QObject* obj, QEvent* event) override;




};
#endif // MAINWINDOW_H
