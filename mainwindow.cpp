#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QScreen>
#include <QGuiApplication>
#include <QPixmap>
#include <QDateTime>
#include <QProcess>
#include <QRegularExpression>
#include <QDialog>
#include <QDir>
#include <QStandardPaths>
#include <QRandomGenerator>
#include <QLabel>
#include <QDebug>
#include <QShortcut>
#include <QScrollBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QPainter>
#include <QFile>
#include "globalhotkeymanager.h"
#include "settingsdialog.h"
#include "telemetrymanager.h"
#include "telemetrydashboardv2.h"
#include "stealthmoveselector.h"
#include <QSettings>
#include <QThread>
#include <QEasingCurve>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>

QString determinePhaseFromFEN(const QString& fen) {
    QStringList parts = fen.split(' ');
    if (parts.size() >= 6) {
        bool ok = false;
        int fullMoveNum = parts[5].toInt(&ok);
        if (ok) {
            if (fullMoveNum <= 10) return "opening";
            if (fullMoveNum <= 30) return "mid";
            return "end";
        }
    }
    return "mid";
}

int computeMoveComplexity(const QVector<MoveCandidate>& multipv) {
    if (multipv.size() < 3) return 0;
    double spread = std::abs(multipv[0].score - multipv[2].score);
    return std::clamp(int(spread / 30), 0, 5);
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    telemetryManager = new TelemetryManager(this);
    telemetryDock = new TelemetryDashboardV2(this);
    addDockWidget(Qt::RightDockWidgetArea, telemetryDock);
    connect(telemetryDock, &TelemetryDashboardV2::clearTelemetryRequested,
            this, &MainWindow::clearTelemetryLog);
    telemetryDock->refresh(telemetryManager);
    randomSeed = static_cast<quint32>(QDateTime::currentMSecsSinceEpoch());
    randomGenerator.seed(randomSeed);
    QSettings settings("ChessGUI", "ChessGUI");
    analysisInterval = settings.value("analysisInterval", 1000).toInt();
    engineDepth = settings.value("engineDepth", 15).toInt();
    autoMoveDelayMs = settings.value("autoMoveDelay", 0).toInt();
    autoMoveWhenReady = settings.value("autoMoveWhenReady", false).toBool();
    stealthTemperature = settings.value("stealthTemperature", 0.035).toDouble();
    stealthInjectPct = settings.value("stealthInjectPct", 10).toInt();
    engineStrength = settings.value("engineStrength", "Unrestricted").toString();
    boardTurnColor = "w";
    useAutoBoardDetectionSetting = settings.value("autoBoardDetection", true).toBool();
    forceManualRegionSetting = settings.value("forceManualRegion", false).toBool();
    enginePath = settings.value("enginePath",
        QCoreApplication::applicationDirPath() + "/lc0.exe").toString();
    fenModelPath = settings.value("fenModelPath",
        QCoreApplication::applicationDirPath() +
        "/python/fen_tracker/ccn_model_default.pth").toString();

    ui->automoveCheck->setChecked(autoMoveWhenReady);
    ui->stealthCheck->setChecked(settings.value("stealthMode", false).toBool());
    QString color = settings.value("defaultColor", "White").toString();
    if (color.compare("Black", Qt::CaseInsensitive) == 0)
        ui->blackRadioButton->setChecked(true);
    else
        ui->whiteRadioButton->setChecked(true);
    updateStatusLabel("Idle");
    setStatusLight("gray");
    // === Hotkeys ===
#ifdef Q_OS_WIN
    hotkeyManager = new GlobalHotkeyManager(this);
    hotkeyManager->registerHotkey(1, QKeySequence("Ctrl+M"));
    hotkeyManager->registerHotkey(2, QKeySequence("Ctrl+S"));
    hotkeyManager->registerHotkey(3, QKeySequence("Ctrl+A"));
    connect(hotkeyManager, &GlobalHotkeyManager::activated, this, [this](int id) {
        switch (id) {
        case 1:
            ui->automoveCheck->setChecked(!ui->automoveCheck->isChecked());
            statusBar()->showMessage(QString("Automove: %1").arg(ui->automoveCheck->isChecked() ? "ON" : "OFF"));
            updateStatusLabel(QString("Automove: %1").arg(ui->automoveCheck->isChecked() ? "ON" : "OFF"));
            break;
        case 2:
            ui->stealthCheck->setChecked(!ui->stealthCheck->isChecked());
            statusBar()->showMessage(QString("Stealth Mode: %1").arg(ui->stealthCheck->isChecked() ? "ON" : "OFF"));
            updateStatusLabel(QString("Stealth Mode: %1").arg(ui->stealthCheck->isChecked() ? "ON" : "OFF"));
            break;
        case 3:
            on_toggleAnalysisButton_clicked();
            break;
        }
    });
#else
    QShortcut* toggleAutoMove = new QShortcut(QKeySequence("Ctrl+M"), this);
    connect(toggleAutoMove, &QShortcut::activated, this, [this]() {
        ui->automoveCheck->setChecked(!ui->automoveCheck->isChecked());
        statusBar()->showMessage(QString("Automove: %1").arg(ui->automoveCheck->isChecked() ? "ON" : "OFF"));
    });

    QShortcut* toggleStealth = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(toggleStealth, &QShortcut::activated, this, [this]() {
        ui->stealthCheck->setChecked(!ui->stealthCheck->isChecked());
        statusBar()->showMessage(QString("Stealth Mode: %1").arg(ui->stealthCheck->isChecked() ? "ON" : "OFF"));
    });

    QShortcut* toggleAnalysis = new QShortcut(QKeySequence("Ctrl+A"), this);
    connect(toggleAnalysis, &QShortcut::activated, this, [this]() {
        on_toggleAnalysisButton_clicked();
    });
#endif

    fenServer = new QProcess(this);
    startFenServer();
    board = new BoardWidget();
    QVBoxLayout* layout = new QVBoxLayout(ui->chessBoardFrame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(board);
    board->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    evalScoreLabel = new QLabel(ui->evalBar);

    evalScoreLabel->setObjectName("evalBarOverlay");
    evalScoreLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    evalScoreLabel->setFixedHeight(20);
    evalScoreLabel->setMinimumWidth(40);
    evalScoreLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    evalScoreLabel->setAlignment(Qt::AlignCenter);
    evalScoreLabel->setStyleSheet(R"(
    background-color: #1e1e1e;
    color: #f0f0f0;
    font-weight: bold;
    font-size: 11px;
    padding: 2px 6px;
    border: 1px solid #666;
    border-radius: 4px;
    )");


    evalScoreLabel->show();
    evalAnimation = new QVariantAnimation(this);
    evalAnimation->setDuration(300);
    evalAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(evalAnimation, &QVariantAnimation::valueChanged, this, [=](const QVariant &v) {
        ui->evalBar->setValue(v.toInt());
    });
    connect(evalAnimation, &QVariantAnimation::valueChanged, this, &MainWindow::updateEvalLabel);
    updateEvalLabel();
    connect(ui->evalBar, &QProgressBar::valueChanged, this, &MainWindow::updateEvalLabel);
    ui->fenDisplay->setPlainText("Waiting for FEN...");
    screenshotTimer = new QTimer(this);
    connect(screenshotTimer, &QTimer::timeout, this, &MainWindow::captureScreenshot);

    settingsDialog = new SettingsDialog(this);
    settingsDialog->setAnalysisInterval(analysisInterval);
    settingsDialog->setEngineDepth(engineDepth);
    connect(settingsDialog, &SettingsDialog::resetPgnRequested, this, [=]() {
        moveHistoryLines.clear();
        ui->pgnDisplay->clear();
    });
    connect(ui->actionOpen_Settings, &QAction::triggered, this, &MainWindow::openSettings);

    startEngine();  // Launch Maia/Lc0 engine


    connect(ui->whiteRadioButton, &QRadioButton::toggled, this, [=](bool checked) {
        if (checked) {
            myColor = "w";
            ui->evalBar->setInvertedAppearance(false);  // white on bottom
            updateEvalLabel();

            if (fenServer && fenServer->state() == QProcess::Running) {
                fenServer->write("[color] w\n");
            }
        }
    });
    connect(ui->blackRadioButton, &QRadioButton::toggled, this, [=](bool checked) {
        if (checked) {
            myColor = "b";
            ui->evalBar->setInvertedAppearance(true);
            updateEvalLabel();

            if (fenServer && fenServer->state() == QProcess::Running) {
                fenServer->write("[color] b\n");
            }
        }
    });


    connect(fenServer, &QProcess::readyReadStandardOutput, this, [=]() {
        QStringList lines = QString::fromUtf8(fenServer->readAllStandardOutput()).split("\n", Qt::SkipEmptyParts);
        for (const QString& rawLine : lines) {
            QString output = rawLine.trimmed();

            qDebug() << "[raw output]" << output;

            if (output == "ready") {
                qDebug() << "[fen_server] Ready";
                return;
            }

            if (output.startsWith("[error]")) {
                qDebug() << "[fen_server] Error:" << output;
                return;

            }

            if (output.startsWith("[skip]")) {
                qDebug() << "[fen_server] Skipped duplicate frame — no update";
                return;  // ✅ DO NOT render or evaluate
            }


            if (output.startsWith("[FEN] ")) {
                QString fen = output.mid(6);  // Skip "[FEN] "
                QString pieceLayout = fen.section(" ", 0, 0);
                QString turnColor = fen.section(" ", 1, 1);
                boardTurnColor = turnColor;
                qDebug() << "[timing] FEN processing:" << fenElapsed.elapsed() << "ms";

                isMyTurn = (getMyColor() == turnColor);
                bool fenChanged = (lastFen != fen);

                if (!lastFen.isEmpty() && fenChanged) {
                    QString uci = detectUciMove(lastFen, fen);
                    bool whiteMoved = lastFen.section(' ', 1, 1) == "w";
                    pendingEvalLine = addMoveToHistory(uci, whiteMoved);
                    bool weMoved = lastFen.section(' ', 1, 1) == getMyColor();
                    if (weMoved) {
                        lastOwnMove = uci;
                        lastPlayedFen = fen;
                        lastAutoMove.clear();
                    }
                }

                qDebug() << "[gui] Received FEN:" << fen;
                qDebug() << "[gui] Piece layout:" << pieceLayout;
                qDebug() << "[gui] Passing to board: flipped =" << (getMyColor() == "b");

                if (board) {
                    board->setPositionFromFen(pieceLayout, getMyColor() == "b");
                    if (!isMyTurn) {
                        board->setArrows({});
                    }
                }

                if (fenChanged && !isMyTurn) {
                    ui->bestMoveDisplay->clear();  // ✅ Only clear if FEN changed and it's not your turn
                }

                if (fenChanged) {
                    evaluatePosition(fen);
                }

                if (isMyTurn) {
                    statusBar()->showMessage("My turn — analyzing...");
                    updateStatusLabel("My turn — analyzing...");
                    setStatusLight("green");
                } else {
                    statusBar()->showMessage("Opponent's turn — analyzing...");
                    updateStatusLabel("Opponent's turn — analyzing...");
                    setStatusLight("red");
                }

                lastFen = fen;
                repetitionTable[fen] = repetitionTable.value(fen, 0) + 1;
                ui->fenDisplay->setPlainText(fen);
            }
        }
    });


}

MainWindow::~MainWindow()
{
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QFile::remove(QDir(tempDir).filePath("chessgui_last_screenshot.png"));
    if (screenshotTimer)
        screenshotTimer->stop();
    if (fenServer && fenServer->state() != QProcess::NotRunning) {
        restartFenServerOnCrash = false;
        fenServer->kill();
        fenServer->waitForFinished(3000);
    }
    if (engineProcess && engineProcess->state() != QProcess::NotRunning) {
        engineProcess->kill();
        engineProcess->waitForFinished(3000);
    }
    if (moveProcess && moveProcess->state() != QProcess::NotRunning) {
        moveProcess->kill();
        moveProcess->waitForFinished(3000);
    }
    delete ui;
}


void logToFile(const QString &msg) {
    QFile logFile(QCoreApplication::applicationDirPath() + "/log.txt");
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss] ") << msg << "\n";
    }
}


void MainWindow::on_setRegionButton_clicked() {
    QScreen* screen = QGuiApplication::primaryScreen();
    QPixmap screenPixmap = screen->grabWindow(0);
    QImage screenshot = screenPixmap.toImage();

    // Mask out this application's window so detection ignores it
    double dpr = screenshot.devicePixelRatio();
    QRect windowRect = frameGeometry();
    QRect scaledRect(windowRect.x() * dpr,
                     windowRect.y() * dpr,
                     windowRect.width() * dpr,
                     windowRect.height() * dpr);
    QPainter maskPainter(&screenshot);
    maskPainter.fillRect(scaledRect, Qt::black);
    maskPainter.end();

    QRect detected;
    if (useAutoBoardDetectionSetting && !forceManualRegionSetting)
        detected = detectChessboard(screenshot);
    qDebug() << "Detected chessboard region:" << detected;

    if (!detected.isNull()) {
        autoDetectedRegion = detected;

        // If a previous overlay exists, clean it up
        if (autoOverlay) {
            autoOverlay->close();
            autoOverlay->deleteLater();
            autoOverlay = nullptr;
        }

        // Define and show the modal overlay painter
        class OverlayPainter : public QDialog {
        public:
            QRect highlight;
            OverlayPainter(QRect rect)
                : QDialog(nullptr, Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::Window)
                , highlight(rect)
            {
                setAttribute(Qt::WA_TranslucentBackground);
                setFocusPolicy(Qt::StrongFocus);
                setModal(true);  // ✅ This grabs all keyboard input
                setGeometry(QGuiApplication::primaryScreen()->geometry());
            }

        protected:
            void paintEvent(QPaintEvent*) override {
                QPainter p(this);
                p.fillRect(rect(), QColor(0, 0, 0, 100));
                p.setPen(QPen(Qt::cyan, 3));
                p.drawRect(highlight);
                p.setPen(Qt::white);
                p.setFont(QFont("Arial", 14));
                p.drawText(highlight.bottomLeft() + QPoint(0, 30),
                           "Press Enter to Confirm, Esc to Cancel");
            }
        };

        // Instantiate overlay
        autoOverlay = new OverlayPainter(detected);
        autoOverlay->installEventFilter(this);

        // Force delayed focus grab after event loop returns
        QTimer::singleShot(0, this, [this]() {
            if (autoOverlay) {
                autoOverlay->show();
                autoOverlay->raise();
                autoOverlay->activateWindow();
                autoOverlay->setFocus();
            }
        });


    } else {
        // Fallback: Manual selection
        RegionSelector* selector = new RegionSelector();
        connect(selector, &RegionSelector::regionSelected, this, [=](const QRect& region) {
            captureRegion = region;
            statusBar()->showMessage("Manual region set.");
            updateStatusLabel("Manual region set.");
        });
        selector->show();
    }
}



void MainWindow::captureScreenshot() {
    if (automoveInProgress) {
        qDebug() << "[screenshot] Skipped: automove in progress";
        return;
    }

    if (captureRegion.isNull()) return;

    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    screenshotElapsed.restart();

    QPixmap fullShot = screen->grabWindow(0,
                                          captureRegion.x(),
                                          captureRegion.y(),
                                          captureRegion.width(),
                                          captureRegion.height());

    QPixmap resized = fullShot.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QImage image = resized.toImage().convertToFormat(QImage::Format_RGB888);

    statusBar()->showMessage("Board changed → ready to analyze");
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString imagePath = QDir(tempDir).filePath("chessgui_last_screenshot.png");
    image.save(imagePath);
    qDebug() << "[timing] Screenshot capture:" << screenshotElapsed.elapsed() << "ms";
    qDebug() << "[runFenPrediction] Sending image path:" << imagePath;
    runFenPrediction(imagePath);


}

void MainWindow::startEngine() {
    restartEngineOnCrash = true;
    engineProcess = new QProcess(this);

    QProcess* proc = engineProcess;

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc](int, QProcess::ExitStatus exitStatus) {
                bool shouldRestart = (proc == engineProcess) &&
                                     restartEngineOnCrash &&
                                     exitStatus == QProcess::CrashExit;
                proc->deleteLater();
                if (proc == engineProcess)
                    engineProcess = nullptr;
                if (shouldRestart) {
                    statusBar()->showMessage("Engine crashed - restarting");
                    updateStatusLabel("Engine crashed - restarting");
                    QTimer::singleShot(0, this, &MainWindow::startEngine);
                }
            });

    connect(proc, &QProcess::errorOccurred, this,
            [this, proc](QProcess::ProcessError error) {
                bool shouldRestart = (proc == engineProcess) &&
                                     restartEngineOnCrash &&
                                     error == QProcess::Crashed;
                if (shouldRestart) {
                    statusBar()->showMessage("Engine crashed - restarting");
                    updateStatusLabel("Engine crashed - restarting");
                    proc->deleteLater();
                    engineProcess = nullptr;
                    QTimer::singleShot(0, this, &MainWindow::startEngine);
                }
            });

    engineProcess->start(this->enginePath, QStringList{});

    if (!engineProcess->waitForStarted()) {
        qDebug() << "Failed to start engine";
        updateStatusLabel("Engine failed to start");
        return;
    }

    // ----- one-time UCI handshake & options -----
    engineProcess->write("uci\n");
    engineProcess->waitForReadyRead(200);
    QString backend = "blas";
#ifdef USE_GPU
    backend = "cudnn";
#endif
    QSettings settings("ChessGUI", "ChessGUI");
    weightsPath = settings.value("weightsPath",
        QCoreApplication::applicationDirPath() + "/maia1900.pb.gz").toString();
    QString strengthSetting = settings.value("engineStrength", "Unrestricted").toString();
    if (strengthSetting != "Unrestricted") {
        QString fname;
        if (strengthSetting == "Maia-1100") fname = "maia1100.pb.gz";
        else if (strengthSetting == "Maia-1500") fname = "maia1500.pb.gz";
        else if (strengthSetting == "Maia-1900") fname = "maia1900.pb.gz";
        QString candidate = QCoreApplication::applicationDirPath() + "/" + fname;
        if (QFile::exists(candidate))
            weightsPath = candidate;
    }
    engineProcess->write(QString("setoption name Backend value %1\n").arg(backend).toUtf8());
    engineProcess->write(QString("setoption name WeightsFile value %1\n")
        .arg(weightsPath).toUtf8());
    engineProcess->write("ucinewgame\n");

    connect(engineProcess, &QProcess::readyReadStandardOutput, this, [=]() {
        QStringList lines = QString::fromUtf8(engineProcess->readAllStandardOutput()).split("\n", Qt::SkipEmptyParts);

        QRegularExpression bestMovePattern("bestmove ([a-h][1-8][a-h][1-8])");
        QRegularExpression matePattern("score mate (-?\\d+)");
        QRegularExpression cpPattern("score cp (-?\\d+)");
        QRegularExpression pvPattern(
            "multipv\\s+(\\d+)\\s+.*score\\s+cp\\s+(-?\\d+)\\s+.*pv\\s+([a-h][1-8][a-h][1-8])");
        QRegularExpression policyPattern1(
            "info string\\s+([a-h][1-8][a-h][1-8]).*\\(P:\\s*([0-9.]+)%\\)");
        QRegularExpression policyPattern2(
            "info move\\s+([a-h][1-8][a-h][1-8]).*policy\\s+([0-9.]+)");

        for (const QString& line : lines) {
            QString trimmed = line.trimmed();

            QRegularExpressionMatch pvMatch = pvPattern.match(trimmed);
            if (pvMatch.hasMatch()) {
                int idx = pvMatch.captured(1).toInt();
                int cp = pvMatch.captured(2).toInt();
                QString mv = pvMatch.captured(3);
                multipvMoves[idx] = qMakePair(mv, cp);
            }

            QRegularExpressionMatch pol1 = policyPattern1.match(trimmed);
            if (pol1.hasMatch()) {
                QString mv = pol1.captured(1);
                double prob = pol1.captured(2).toDouble() / 100.0;
                policyProbMap[mv] = prob;
            } else {
                QRegularExpressionMatch pol2 = policyPattern2.match(trimmed);
                if (pol2.hasMatch()) {
                    QString mv = pol2.captured(1);
                    double prob = pol2.captured(2).toDouble();
                    if (prob > 1.0)
                        prob /= 100.0;
                    policyProbMap[mv] = prob;
                }
            }

            QRegularExpressionMatch bestMoveMatch = bestMovePattern.match(trimmed);
            if (bestMoveMatch.hasMatch()) {
                QString bestMove = bestMoveMatch.captured(1);
                qDebug() << "[timing] Stockfish evaluation:" << evalElapsed.elapsed() << "ms";

                QString reverseMove;
                if (lastOwnMove.length() >= 4)
                    reverseMove = lastOwnMove.mid(2, 2) + lastOwnMove.mid(0, 2);

                if (!reverseMove.isEmpty() && bestMove == reverseMove &&
                    repetitionTable.value(lastFen) >= 2 &&
                    engineProcess && engineProcess->state() == QProcess::Running) {

                    engineProcess->write(QString("position fen %1\n").arg(lastFen).toUtf8());
                    engineProcess->write("d\n");

                    QString buffer;
                    if (engineProcess->waitForReadyRead(150))
                        buffer += QString::fromUtf8(engineProcess->readAllStandardOutput());
                    int idx = buffer.indexOf("Legal moves:");
                    QStringList legalMoves;
                    if (idx != -1) {
                        QString line = buffer.mid(idx).section(QLatin1Char('\n'), 0, 0);
                        line = line.section(":", 1).trimmed();
                        legalMoves = line.split(' ', Qt::SkipEmptyParts);
                    }

                    legalMoves.removeAll(reverseMove);
                    if (legalMoves.isEmpty()) {
                        playMove(reverseMove);
                        updateRepetitionTable(lastFen, true);
                        return;
                    }

                    QString cmd = QStringLiteral("position fen %1\nsearchmoves %2\ngo depth %3\n")
                                       .arg(lastFen)
                                       .arg(legalMoves.join(' '))
                                       .arg(engineDepth);
                    engineProcess->write(cmd.toUtf8());
                    return;
                }

                QVector<MoveCandidate> mvList;
                int bestCp = 0;
                for (int i = 1; i <= multipvMoves.size(); ++i) {
                    if (multipvMoves.contains(i)) {
                        auto p = multipvMoves.value(i);
                        MoveCandidate c;
                        c.move = p.first;
                        c.rank = i;
                        c.score = p.second;
                        c.policyProb = policyProbMap.value(c.move, 0.0);
                        mvList.append(c);
                        if (i == 1)
                            bestCp = c.score;
                    }
                }

                lastBestScore = bestCp;
                lastMoveComplexity = computeMoveComplexity(mvList);

                StealthMoveSelector selector;
                selector.setStealthParams(stealthTemperature, 100);
                selector.setTargetACPL(60.0);
                selector.setInjectFrequency(15);

                int moveNum = lastFen.section(' ', 5, 5).toInt();
                MoveCandidate choice;
                if (ui->stealthCheck->isChecked())
                    choice = selector.pickMove(mvList, bestCp, moveNum, lastFen);
                else if (!mvList.isEmpty())
                    choice = mvList.first();

                if (choice.move.isEmpty()) {
                    choice.move = bestMove;
                    choice.rank = 1;
                }
                selectedBestMoveRank = choice.rank;
                multipvMoves.clear();

                if (ui->stealthCheck->isChecked())
                    qDebug() << "[stealth] Move" << choice.move << "score" << choice.score;

                pendingTelemetry.fen = lastEvaluatedFen;
                pendingTelemetry.move = choice.move;
                pendingTelemetry.policyProb = policyProbMap.value(choice.move, 0.0);
                pendingTelemetry.thinkTimeMs = 0;
                pendingTelemetry.rank = choice.rank;

                if (lastEvaluatedFen == lastFen) {  // ✅ Ensures best move matches current board
                    currentBestMove = choice.move;
                    QString label = choice.move;
                    if (choice.rank > 1) {
                        label += QString(" (Move: %1)").arg(choice.rank);
                    }
                    ui->bestMoveDisplay->setText(label);

                    if (choice.move.length() == 4) {
                        QString from = choice.move.mid(0, 2);
                        QString to = choice.move.mid(2, 2);
                        board->setArrows({ qMakePair(from, to) });

                        if (isMyTurn && ui->automoveCheck->isChecked() && lastEvaluatedFen == lastFen) {
                            playBestMove();  // ✅ Only play after fresh bestMove matches fresh FEN
                        }
                    }
                } else {
                    qDebug() << "[Engine] Ignoring best move for stale FEN";
                }
            }


            QRegularExpressionMatch mateMatch = matePattern.match(trimmed);
            QRegularExpressionMatch cpMatch = cpPattern.match(trimmed);

            // 0. Who is the engine talking about?  + = good for White
            int povSign = (boardTurnColor == "b") ? -1 : 1;

            /* ---------- mate in N ---------- */
            if (mateMatch.hasMatch()) {
                int mateMoves = mateMatch.captured(1).toInt();     // engine value
                int whiteMate = povSign * mateMoves;               // re-oriented
                QString txt   = QString("M%1").arg(whiteMate);

                ui->evalBar->setRange(-1000, 1000);
                ui->evalBar->setValue(whiteMate > 0 ? +1000 : -1000);

                evalScoreLabel->setText(txt);
                updateEvalLabel();
                statusBar()->showMessage("Eval: " + txt);
                updateStatusLabel("Eval: " + txt);
            }

            /* ---------- centipawn ---------- */
            else if (cpMatch.hasMatch()) {
                int rawCp   = cpMatch.captured(1).toInt();         // engine value
                int whiteCp = povSign * rawCp;                     // re-oriented
                QString txt = QString::number(whiteCp / 100.0, 'f', 2);

                ui->evalBar->setRange(-1000, 1000);
                ui->evalBar->setValue(std::clamp(whiteCp, -1000, 1000));

                evalScoreLabel->setText(txt);
                updateEvalLabel();
                statusBar()->showMessage("Eval: " + txt);
                updateStatusLabel("Eval: " + txt);
            }
        }
    });


}

void MainWindow::startFenServer() {
    restartFenServerOnCrash = true;

    if (fenServer) {
        fenServer->deleteLater();
    }

    fenServer = new QProcess(this);
    QProcess* proc = fenServer;

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc](int, QProcess::ExitStatus exitStatus) {
                bool shouldRestart = (proc == fenServer) &&
                                     restartFenServerOnCrash &&
                                     exitStatus == QProcess::CrashExit;
                proc->deleteLater();
                if (proc == fenServer)
                    fenServer = nullptr;
                if (shouldRestart) {
                    statusBar()->showMessage("FEN server crashed - restarting");
                    updateStatusLabel("FEN server crashed - restarting");
                    QTimer::singleShot(0, this, &MainWindow::startFenServer);
                }
            });

    connect(proc, &QProcess::errorOccurred, this,
            [this, proc](QProcess::ProcessError error) {
                bool shouldRestart = (proc == fenServer) &&
                                     restartFenServerOnCrash &&
                                     error == QProcess::Crashed;
                if (shouldRestart) {
                    statusBar()->showMessage("FEN server crashed - restarting");
                    updateStatusLabel("FEN server crashed - restarting");
                    proc->deleteLater();
                    fenServer = nullptr;
                    QTimer::singleShot(0, this, &MainWindow::startFenServer);
                }
            });

    QString scriptPath = QCoreApplication::applicationDirPath() + "/python/fen_tracker/main.py";
    QString color = getMyColor();  // This returns "w" or "b"
    QStringList arguments;
    arguments << scriptPath << "--color" << color;

    qDebug() << "[fenServer] Launching python with arguments:" << arguments;

    QString embeddedPython = QCoreApplication::applicationDirPath() + "/python/python.exe";
    proc->start(embeddedPython, arguments);

    if (!proc->waitForStarted()) {
        qDebug() << "[fenServer] Failed to start";
        return;
    }

    // ✅ Immediately send the color again in case user toggled it early
    proc->write(QString("[color] %1\n").arg(color).toUtf8());

    connect(fenServer, &QProcess::readyReadStandardError, this, [=]() {
        QString error = QString::fromUtf8(fenServer->readAllStandardError());
        qDebug() << "[fenServer stderr]" << error;
    });

}






void MainWindow::on_toggleAnalysisButton_clicked() {
    if (analysisRunning) {
        screenshotTimer->stop();
        ui->toggleAnalysisButton->setText("Start Analysis (Ctrl +A)");
        updateStatusLabel("Idle");
        setStatusLight("gray");
        analysisRunning = false;
        return;
    }

    if (captureRegion.isNull()) {
        on_setRegionButton_clicked();
        if (captureRegion.isNull()) {
            QMessageBox::warning(this, tr("No Region"), tr("Please select a board region first."));
            ui->toggleAnalysisButton->setChecked(false);
            return;
        }
    }

    screenshotTimer->start(analysisInterval);
    ui->toggleAnalysisButton->setText("Stop Analysis (Ctrl +A)");
    updateStatusLabel("Analyzing...");
    setStatusLight("yellow");
    analysisRunning = true;
}

void MainWindow::runFenPrediction(const QString& imagePath) {
    if (!fenServer || fenServer->state() != QProcess::Running) {
        qDebug() << "[fen_server] Not running";
        return;
    }

    fenElapsed.restart();
    QString toSend = imagePath + "\n";
    fenServer->write(toSend.toUtf8());
}

void MainWindow::evaluatePosition(const QString& fen) {
    lastEvaluatedFen = fen;

    if (!engineProcess || engineProcess->state() != QProcess::Running)
        return;

    evalElapsed.restart();

    QStringList commands;
    commands << "position fen " + fen;
    commands << "setoption name Multipv value 3";
    if (ui->stealthCheck->isChecked()) {
        commands << "go depth 6";
    } else {
        commands << QString("go depth %1").arg(engineDepth);
    }

    multipvMoves.clear();
    policyProbMap.clear();
    selectedBestMoveRank = 1;

    for (const QString& cmd : commands) {
        engineProcess->write((cmd + "\n").toUtf8());
    }
}

QString MainWindow::getMyColor() const {
    return ui->whiteRadioButton->isChecked() ? "w" : "b";
}



bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (obj == autoOverlay && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            captureRegion = autoDetectedRegion;
            autoOverlay->close();
            autoOverlay->deleteLater();
            autoOverlay = nullptr;
            statusBar()->showMessage("Auto-detected region set.");
            updateStatusLabel("Auto-detected region set.");
            return true;
        } else if (keyEvent->key() == Qt::Key_Escape) {
            autoOverlay->close();
            autoOverlay->deleteLater();
            autoOverlay = nullptr;
            statusBar()->showMessage("Cancelled — using manual selector.");
            updateStatusLabel("Cancelled — using manual selector.");

            RegionSelector* selector = new RegionSelector();
            connect(selector, &RegionSelector::regionSelected, this, [=](const QRect& region) {
                captureRegion = region;
                statusBar()->showMessage("Manual region set.");
                updateStatusLabel("Manual region set.");
            });
            selector->show();

            return true;
        }

    }
    return QMainWindow::eventFilter(obj, event);
}


void MainWindow::setStatusLight(const QString& color) {
    QString hex;
    if (color == "green") hex = "#4CAF50";
    else if (color == "yellow") hex = "#FFC107";
    else if (color == "red") hex = "#F44336";
    else hex = "gray";

    ui->statusLightLabel->setStyleSheet(QString(
                                            "border-radius: 10px; background-color: %1;").arg(hex));
}

void MainWindow::updateStatusLabel(const QString& text) {
    ui->statusLabelEdit->setText(text);
}

void MainWindow::playBestMove() {
    if (currentBestMove.length() != 4 || automoveInProgress)
        return;
    if (currentBestMove == lastAutoMove)
        return;
    automoveInProgress = true;
    lastAutoMove = currentBestMove;
    qDebug() << "[automove] Starting move execution";

    QString from = currentBestMove.mid(0, 2);
    QString to = currentBestMove.mid(2, 2);

    QString scriptPath =
        QDir::cleanPath(QCoreApplication::applicationDirPath() +
                        "/python/fen_tracker/play_move.py");

    int originX = captureRegion.x();
    int originY = captureRegion.y();
    int tileSize = captureRegion.width() / 8;
    bool flipped = (getMyColor() == "b");
    bool stealth = ui->stealthCheck->isChecked();

    QString phase = determinePhaseFromFEN(lastFen);
    double evalScore = lastBestScore;
    if (std::abs(evalScore) <= 1.5)
        evalScore *= 100.0;

    QStringList args;
    args << scriptPath
         << from
         << to
         << QString::number(originX)
         << QString::number(originY)
         << QString::number(tileSize)
         << (flipped ? "true" : "false")
         << (stealth ? "true" : "false")
         << "--phase" << phase
         << "--complexity" << QString::number(lastMoveComplexity)
         << "--eval" << QString::number(evalScore);

    qDebug() << "[play_move] args:" << args;

    auto execute = [=]() {
        if (moveProcess)
            moveProcess->deleteLater();
        moveProcess = new QProcess(this);
        connect(moveProcess,
                QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this](int, QProcess::ExitStatus) {
                    automoveInProgress = false;
                    QTimer::singleShot(1000, this, &MainWindow::captureScreenshot);
                    moveProcess->deleteLater();
                    moveProcess = nullptr;
                });

        QString embeddedPython = QCoreApplication::applicationDirPath() + "/python/python.exe";
        qDebug() << "[play_move] start" << embeddedPython << args;
        moveProcess->start(embeddedPython, args);
        if (!moveProcess->waitForStarted()) {
            qDebug() << "[automove] Failed to start move process";
            moveProcess->deleteLater();
            moveProcess = nullptr;
            automoveInProgress = false;
        }
    };

    int delay = autoMoveDelayMs;
    if (stealth) {
        const double mean = 1500.0;
        const double stddev = 500.0;
        double sigma = std::sqrt(std::log(1.0 + (stddev * stddev) / (mean * mean)));
        double mu = std::log(mean) - 0.5 * sigma * sigma;
        std::lognormal_distribution<double> dist(mu, sigma);
        delay += static_cast<int>(dist(randomGenerator));
    }

    if (ui->stealthCheck->isChecked()) {
        // If pendingTelemetry.move is empty (Lc0/Maia), fill with current move.
        if (pendingTelemetry.move.isEmpty() && !currentBestMove.isEmpty()) {
            pendingTelemetry.move = currentBestMove;
            pendingTelemetry.rank = 1;
            pendingTelemetry.evalPlayed = 0; // You could try to parse a value if you want
            pendingTelemetry.evalBest = 0;
            pendingTelemetry.cpDelta = 0;
        }
        pendingTelemetry.thinkTimeMs = delay;
        pendingTelemetry.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
        telemetryManager->logEntry(pendingTelemetry);
        telemetryDock->refresh(telemetryManager);
        pendingTelemetry = TelemetryEntry();
    }

    if (delay > 0)
        QTimer::singleShot(delay, this, execute);
    else
        execute();
}

void MainWindow::playMove(const QString &uci) {
    QString prev = currentBestMove;
    currentBestMove = uci;
    playBestMove();
    currentBestMove = prev;
}

void MainWindow::updateRepetitionTable(const QString &fen, bool afterMove) {
    if (afterMove)
        repetitionTable[fen] = repetitionTable.value(fen, 0) + 1;
    else if (repetitionTable.contains(fen) && repetitionTable.value(fen) > 0)
        repetitionTable[fen] -= 1;
}

void MainWindow::updateEvalLabel() {
    if (!evalScoreLabel || !ui->evalBar) return;

    int value = ui->evalBar->value();
    int min = ui->evalBar->minimum();
    int max = ui->evalBar->maximum();
    if (max == min) return;

    double ratio = double(value - min) / double(max - min);
    int barHeight = ui->evalBar->height();
    bool inverted = ui->evalBar->invertedAppearance();

    int y = inverted ? ratio * barHeight : barHeight - ratio * barHeight;
    y -= evalScoreLabel->height() / 2;
    y = qBound(0, y, barHeight - evalScoreLabel->height());
    int x = (ui->evalBar->width() - evalScoreLabel->width()) / 2;

    evalScoreLabel->move(x, y);
}

void MainWindow::setEvalBarValue(int value) {
    if (!evalAnimation || !ui->evalBar) {
        if (ui->evalBar)
            ui->evalBar->setValue(value);
        return;
    }
    if (evalAnimation->state() == QVariantAnimation::Running)
        evalAnimation->stop();
    evalAnimation->setStartValue(ui->evalBar->value());
    evalAnimation->setEndValue(value);
    evalAnimation->start();
}

QString MainWindow::detectUciMove(const QString& prevFen, const QString& currFen) const {
    auto parseBoard = [](const QString& layout) {
        QVector<QVector<QChar>> board(8, QVector<QChar>(8, '.'));
        QStringList ranks = layout.split('/');
        for (int r = 0; r < qMin(8, ranks.size()); ++r) {
            int c = 0;
            for (QChar ch : ranks[r]) {
                if (ch.isDigit()) {
                    c += ch.digitValue();
                } else {
                    if (c < 8) board[r][c] = ch;
                    ++c;
                }
            }
        }
        return board;
    };

    QString prevLayout = prevFen.section(' ', 0, 0);
    QString currLayout = currFen.section(' ', 0, 0);
    auto prevBoard = parseBoard(prevLayout);
    auto currBoard = parseBoard(currLayout);

    QString from, to;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (prevBoard[r][c] != currBoard[r][c]) {
                if (prevBoard[r][c] != '.' && currBoard[r][c] == '.') {
                    from = QString(QChar('a' + c)) + QString::number(8 - r);
                } else if (prevBoard[r][c] == '.' && currBoard[r][c] != '.') {
                    to = QString(QChar('a' + c)) + QString::number(8 - r);
                } else if (prevBoard[r][c] != currBoard[r][c] && currBoard[r][c] != '.') {
                    to = QString(QChar('a' + c)) + QString::number(8 - r);
                    if (from.isEmpty())
                        from = QString(QChar('a' + c)) + QString::number(8 - r);
                }
            }
        }
    }

    if (!from.isEmpty() && !to.isEmpty())
        return from + to;
    return {};
}

int MainWindow::addMoveToHistory(const QString& moveUci, bool whiteMove) {
    if (moveUci.isEmpty()) return -1;

    int index = 0;
    if (whiteMove || moveHistoryLines.isEmpty()) {
        moveHistoryLines.append(QString::number(moveHistoryLines.size() + 1) + ". " + moveUci);
        index = moveHistoryLines.size() - 1;
    } else {
        moveHistoryLines.last().append(" " + moveUci);
        index = moveHistoryLines.size() - 1;
    }

    ui->pgnDisplay->setPlainText(moveHistoryLines.join("\n"));
    ui->pgnDisplay->verticalScrollBar()->setValue(ui->pgnDisplay->verticalScrollBar()->maximum());
    return index;
}

void MainWindow::appendEvalChangeToHistory(int index, double delta) {
    if (index < 0 || index >= moveHistoryLines.size()) return;
    QString sign = delta >= 0 ? "+" : "";
    QString deltaStr = QString("%1%2").arg(sign).arg(QString::number(delta, 'f', 2));
    moveHistoryLines[index].append(QString(" (%1)").arg(deltaStr));
    ui->pgnDisplay->setPlainText(moveHistoryLines.join("\n"));
    ui->pgnDisplay->verticalScrollBar()->setValue(ui->pgnDisplay->verticalScrollBar()->maximum());
}

void MainWindow::openSettings()
{
    if (!settingsDialog)
        return;
    settingsDialog->setAnalysisInterval(analysisInterval);
    settingsDialog->setEngineDepth(engineDepth);
    settingsDialog->setStealthModeEnabled(ui->stealthCheck->isChecked());
    settingsDialog->setUseAutoBoardDetection(useAutoBoardDetectionSetting);
    settingsDialog->setForceManualRegion(forceManualRegionSetting);
    settingsDialog->setAutoMoveWhenReady(ui->automoveCheck->isChecked());
    settingsDialog->setAutoMoveDelay(autoMoveDelayMs);
    settingsDialog->setEnginePath(enginePath);
    settingsDialog->setFenModelPath(fenModelPath);
    settingsDialog->setWeightsPath(weightsPath);
    settingsDialog->setDefaultPlayerColor(ui->whiteRadioButton->isChecked() ? "White" : "Black");
    settingsDialog->setStealthTemperature(stealthTemperature);
    settingsDialog->setInjectPercent(stealthInjectPct);
    settingsDialog->setEngineStrength(engineStrength);
    if (settingsDialog->exec() == QDialog::Accepted) {
        analysisInterval = settingsDialog->analysisInterval();
        engineDepth = settingsDialog->engineDepth();
        ui->stealthCheck->setChecked(settingsDialog->stealthModeEnabled());
        useAutoBoardDetectionSetting = settingsDialog->useAutoBoardDetection();
        forceManualRegionSetting = settingsDialog->forceManualRegion();
        ui->automoveCheck->setChecked(settingsDialog->autoMoveWhenReady());
        autoMoveDelayMs = settingsDialog->autoMoveDelay();
        enginePath = settingsDialog->enginePath();
        fenModelPath = settingsDialog->fenModelPath();
        weightsPath = settingsDialog->weightsPath();
        stealthTemperature = settingsDialog->stealthTemperature();
        stealthInjectPct = settingsDialog->injectPercent();
        engineStrength = settingsDialog->engineStrength();
        if (settingsDialog->defaultPlayerColor() == "Black")
            ui->blackRadioButton->setChecked(true);
        else
            ui->whiteRadioButton->setChecked(true);
        if (analysisRunning) {
            screenshotTimer->stop();
            screenshotTimer->start(analysisInterval);
        }
        if (engineProcess) {
            restartEngineOnCrash = false;
            engineProcess->kill();
            engineProcess->deleteLater();
            engineProcess = nullptr;
            startEngine();
        }
    }
}

void MainWindow::on_resetGameButton_clicked()
{
    if (analysisRunning) {
        screenshotTimer->stop();
        analysisRunning = false;
        ui->toggleAnalysisButton->setChecked(false);
        ui->toggleAnalysisButton->setText("Start Analysis (Ctrl +A)");
    }

    if (moveProcess && moveProcess->state() != QProcess::NotRunning) {
        moveProcess->kill();
        moveProcess->waitForFinished(3000);
    }

    lastFen.clear();
    lastEvaluatedFen.clear();
    lastPlayedFen.clear();
    lastOwnMove.clear();
    lastAutoMove.clear();
    boardTurnColor.clear();
    repetitionTable.clear();
    multipvMoves.clear();
    currentBestMove.clear();
    pendingEvalLine = -1;
    lastEvalForMe = 0.0;
    lastEvalValid = false;
    moveHistoryLines.clear();

    if (board) {
        board->setPositionFromFen("", getMyColor() == "b");
        board->setArrows({});
    }

    ui->pgnDisplay->clear();
    ui->fenDisplay->setPlainText("Waiting for FEN...");
    ui->bestMoveDisplay->clear();
    evalScoreLabel->clear();
    setEvalBarValue(0);

    updateStatusLabel("Idle");
    setStatusLight("gray");
    statusBar()->showMessage("Game reset");
}

MoveCandidate MainWindow::pickBestMove(bool stealth, double temperature, int injectPct)
{
    MoveCandidate result;
    pendingTelemetry = TelemetryEntry();
    if (multipvMoves.isEmpty() || !multipvMoves.contains(1))
        return result;

    // Collect up to the top 3 moves from Stockfish
    QVector<MoveCandidate> all;
    for (int i = 1; i <= 3; ++i) {
        if (multipvMoves.contains(i)) {
            auto pair = multipvMoves.value(i);
            MoveCandidate cand;
            cand.move = pair.first;
            cand.rank = i;
            cand.score = pair.second;
            all.append(cand);
        }
    }
    if (all.isEmpty())
        return result;

    MoveCandidate best = all[0];
    int bestScore = best.score;

    if (!stealth) {
        return best;
    }

    // Filter moves more than 60cp worse than the best
    QVector<MoveCandidate> candidates;
    for (const MoveCandidate &c : all) {
        if (bestScore - c.score <= 60)
            candidates.append(c);
    }
    if (candidates.isEmpty())
        candidates.append(best);

    // Softmax selection using temperature
    double T = temperature;
    QVector<double> weights;
    for (const MoveCandidate &c : candidates) {
        double val = (c.score - bestScore) / 100.0; // pawn units relative to best
        weights.append(std::exp(val / T));
    }
    double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
    QVector<double> probs;
    for (double w : weights)
        probs.append(w / sum);

    double r = randomGenerator.generateDouble();
    int index = 0;
    while (index < probs.size() && r > probs[index]) {
        r -= probs[index];
        ++index;
    }
    if (index >= candidates.size())
        index = candidates.size() - 1;
    MoveCandidate choice = candidates[index];

    pendingTelemetry.fen = lastEvaluatedFen;
    pendingTelemetry.move = choice.move;
    pendingTelemetry.policyProb = policyProbMap.value(choice.move, 0.0);
    pendingTelemetry.thinkTimeMs = 0;

    if (candidates.size() >= 2 && bestScore - candidates[1].score <= 60) {
        if (randomGenerator.generateDouble() < injectPct / 100.0)
            choice = candidates[1];
    }

    return choice;
}

void MainWindow::clearTelemetryLog()
{
    if (telemetryManager)
        telemetryManager->clearLog();
    if (telemetryDock)
        telemetryDock->refresh(telemetryManager);
}

