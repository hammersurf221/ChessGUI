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
#include <QSettings>
#include <QStandardPaths>
#include <cmath>
#include <algorithm>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSettings settings("ChessGUI", "ChessGUI");
    analysisInterval = settings.value("analysisInterval", 1000).toInt();
    stockfishDepth = settings.value("stockfishDepth", 15).toInt();
    autoMoveDelayMs = settings.value("autoMoveDelay", 0).toInt();
    autoMoveWhenReady = settings.value("autoMoveWhenReady", false).toBool();
    useAutoBoardDetectionSetting = settings.value("autoBoardDetection", true).toBool();
    forceManualRegionSetting = settings.value("forceManualRegion", false).toBool();
    stockfishPath = settings.value("stockfishPath",
        QCoreApplication::applicationDirPath() + "/stockfish.exe").toString();
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
    updateEvalLabel();
    connect(ui->evalBar, &QProgressBar::valueChanged, this, &MainWindow::updateEvalLabel);
    ui->fenDisplay->setPlainText("Waiting for FEN...");
    screenshotTimer = new QTimer(this);
    connect(screenshotTimer, &QTimer::timeout, this, &MainWindow::captureScreenshot);

    settingsDialog = new SettingsDialog(this);
    settingsDialog->setAnalysisInterval(analysisInterval);
    settingsDialog->setStockfishDepth(stockfishDepth);
    connect(settingsDialog, &SettingsDialog::resetPgnRequested, this, [=]() {
        moveHistoryLines.clear();
        ui->pgnDisplay->clear();
    });
    connect(ui->actionOpen_Settings, &QAction::triggered, this, &MainWindow::openSettings);

    startStockfish();  // Launch Stockfish engine


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
                qDebug() << "[timing] FEN processing:" << fenElapsed.elapsed() << "ms";

                isMyTurn = (getMyColor() == turnColor);
                bool fenChanged = (lastFen != fen);

                if (!lastFen.isEmpty() && fenChanged) {
                    QString uci = detectUciMove(lastFen, fen);
                    bool whiteMoved = lastFen.section(' ', 1, 1) == "w";
                    pendingEvalLine = addMoveToHistory(uci, whiteMoved);
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
        fenServer->kill();
        fenServer->waitForFinished(3000);
    }
    if (stockfishProcess && stockfishProcess->state() != QProcess::NotRunning) {
        stockfishProcess->kill();
        stockfishProcess->waitForFinished(3000);
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

void MainWindow::startStockfish() {
    restartStockfishOnCrash = true;
    stockfishProcess = new QProcess(this);

    QProcess* proc = stockfishProcess;

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc](int, QProcess::ExitStatus exitStatus) {
                bool shouldRestart = (proc == stockfishProcess) &&
                                     restartStockfishOnCrash &&
                                     exitStatus == QProcess::CrashExit;
                proc->deleteLater();
                if (proc == stockfishProcess)
                    stockfishProcess = nullptr;
                if (shouldRestart) {
                    statusBar()->showMessage("Stockfish crashed - restarting");
                    updateStatusLabel("Stockfish crashed - restarting");
                    QTimer::singleShot(0, this, &MainWindow::startStockfish);
                }
            });

    connect(proc, &QProcess::errorOccurred, this,
            [this, proc](QProcess::ProcessError error) {
                bool shouldRestart = (proc == stockfishProcess) &&
                                     restartStockfishOnCrash &&
                                     error == QProcess::Crashed;
                if (shouldRestart) {
                    statusBar()->showMessage("Stockfish crashed - restarting");
                    updateStatusLabel("Stockfish crashed - restarting");
                    proc->deleteLater();
                    stockfishProcess = nullptr;
                    QTimer::singleShot(0, this, &MainWindow::startStockfish);
                }
            });

    stockfishProcess->start(this->stockfishPath, QStringList{});


    if (!stockfishProcess->waitForStarted()) {
        qDebug() << "Failed to start Stockfish";
        return;
    }

    connect(stockfishProcess, &QProcess::readyReadStandardOutput, this, [=]() {
        QStringList lines = QString::fromUtf8(stockfishProcess->readAllStandardOutput()).split("\n", Qt::SkipEmptyParts);

        QRegularExpression bestMovePattern("bestmove ([a-h][1-8][a-h][1-8])");
        QRegularExpression matePattern("score mate (-?\\d+)");
        QRegularExpression cpPattern("score cp (-?\\d+)");
        QRegularExpression pvPattern(
            "multipv\\s+(\\d+)\\s+.*score\\s+cp\\s+(-?\\d+)\\s+.*pv\\s+([a-h][1-8][a-h][1-8])");

        for (const QString& line : lines) {
            QString trimmed = line.trimmed();

            QRegularExpressionMatch pvMatch = pvPattern.match(trimmed);
            if (pvMatch.hasMatch()) {
                int idx = pvMatch.captured(1).toInt();
                int cp = pvMatch.captured(2).toInt();
                QString mv = pvMatch.captured(3);
                multipvMoves[idx] = qMakePair(mv, cp);
            }

            QRegularExpressionMatch bestMoveMatch = bestMovePattern.match(trimmed);
            if (bestMoveMatch.hasMatch()) {
                QString bestMove = bestMoveMatch.captured(1);
                QString chosenMove = bestMove;
                int chosenScore = multipvMoves.value(1).second;
                selectedBestMoveRank = 1;
                qDebug() << "[timing] Stockfish evaluation:" << evalElapsed.elapsed() << "ms";

                if (ui->stealthCheck->isChecked() && !multipvMoves.isEmpty()) {
                    auto first = multipvMoves.value(1);
                    auto second = multipvMoves.value(2);
                    auto third = multipvMoves.value(3);
                    chosenMove = first.first.isEmpty() ? bestMove : first.first;
                    chosenScore = first.second;

                    if (!second.first.isEmpty() && std::abs(first.second - second.second) < 30) {
                        bool pickSecond = QRandomGenerator::global()->bounded(2);
                        if (pickSecond) {
                            selectedBestMoveRank = 2;
                            chosenMove = second.first;
                            chosenScore = second.second;
                        }
                    }

                    double roll = QRandomGenerator::global()->generateDouble();
                    if (roll > accuracy) {
                        QList<int> alts;
                        if (!second.first.isEmpty()) alts.append(2);
                        if (!third.first.isEmpty()) alts.append(3);
                        if (!alts.isEmpty()) {
                            int idx = alts[QRandomGenerator::global()->bounded(alts.size())];
                            if (idx == 2) {
                                selectedBestMoveRank = 2;
                                chosenMove = second.first;
                                chosenScore = second.second;
                            } else {
                                selectedBestMoveRank = 3;
                                chosenMove = third.first;
                                chosenScore = third.second;
                            }
                        }
                    }
                    qDebug() << "[stealth] Move" << chosenMove << "score" << chosenScore;
                }


                multipvMoves.clear();

                if (lastEvaluatedFen == lastFen) {  // ✅ Ensures best move matches current board
                    currentBestMove = chosenMove;
                    QString label = chosenMove;
                    if (selectedBestMoveRank > 1) {
                        label += QString(" (Move: %1)").arg(selectedBestMoveRank);
                    }
                    ui->bestMoveDisplay->setText(label);

                    if (chosenMove.length() == 4) {
                        QString from = chosenMove.mid(0, 2);
                        QString to = chosenMove.mid(2, 2);
                        board->setArrows({ qMakePair(from, to) });

                        if (isMyTurn && ui->automoveCheck->isChecked() && lastEvaluatedFen == lastFen) {
                            playBestMove();  // ✅ Only play after fresh bestMove matches fresh FEN
                        }
                    }
                } else {
                    qDebug() << "[Stockfish] Ignoring best move for stale FEN";
                }
            }


            QRegularExpressionMatch mateMatch = matePattern.match(trimmed);
            QRegularExpressionMatch cpMatch = cpPattern.match(trimmed);

            QString eval;
            double numericEval = 0.0;
            bool hasNumeric = false;
            int signForMe = (lastEvaluatedFen.section(' ', 1, 1) == getMyColor()) ? 1 : -1;
            if (mateMatch.hasMatch()) {
                int mateMoves = mateMatch.captured(1).toInt();
                int signedMate = signForMe >= 0 ? mateMoves : -mateMoves;
                eval = QString("M%1").arg(signedMate);
                numericEval = signForMe * (mateMoves > 0 ? 1000.0 : -1000.0);
                hasNumeric = true;
            } else if (cpMatch.hasMatch()) {
                int scoreCp = cpMatch.captured(1).toInt();
                int signedCp = signForMe * scoreCp;
                eval = QString::number(signedCp / 100.0, 'f', 2);
                numericEval = signForMe * (scoreCp / 100.0);
                numericEval = std::clamp(numericEval, -1000.0, 1000.0);
                hasNumeric = true;
            }

            if (!eval.isEmpty()) {
                statusBar()->showMessage("Eval: " + eval);
                updateStatusLabel("Eval: " + eval);

                if (ui->evalBar) {
                    if (mateMatch.hasMatch()) {
                        int mateScore = mateMatch.captured(1).toInt();
                        int barValue = signForMe * (mateScore > 0 ? 1000 : -1000);
                        ui->evalBar->setMaximum(1000);
                        ui->evalBar->setMinimum(-1000);
                        ui->evalBar->setValue(barValue);
                    } else if (cpMatch.hasMatch()) {
                        int score = cpMatch.captured(1).toInt();
                        score = std::clamp(score, -1000, 1000);
                        ui->evalBar->setValue(signForMe * score);
                    }
                    if (evalScoreLabel) {
                        evalScoreLabel->setText(eval);
                        updateEvalLabel();
                    }
                }

                if (hasNumeric) {
                    double delta = 0.0;
                    if (lastEvalValid) {
                        delta = numericEval - lastEvalForMe;
                    }
                    if (pendingEvalLine != -1) {
                        appendEvalChangeToHistory(pendingEvalLine, delta);
                        pendingEvalLine = -1;
                    }
                    lastEvalForMe = numericEval;
                    lastEvalValid = true;
                }
            }
        }
    });


}

void MainWindow::startFenServer() {
    QString scriptPath = QCoreApplication::applicationDirPath() + "/python/fen_tracker/main.py";
    QString color = getMyColor();  // This returns "w" or "b"
    QStringList arguments;
    arguments << scriptPath << "--color" << color;

    qDebug() << "[fenServer] Launching python with arguments:" << arguments;

    QString embeddedPython = QCoreApplication::applicationDirPath() + "/python/python.exe";
    fenServer->start(embeddedPython, arguments);


    if (!fenServer->waitForStarted()) {
        qDebug() << "[fenServer] Failed to start";
        return;
    }

    // ✅ Immediately send the color again in case user toggled it early
    fenServer->write(QString("[color] %1\n").arg(color).toUtf8());

    connect(fenServer, &QProcess::readyReadStandardOutput, this, [=]() {
        QStringList lines = QString::fromUtf8(fenServer->readAllStandardOutput()).split("\n", Qt::SkipEmptyParts);
        for (const QString& rawLine : lines) {
            QString output = rawLine.trimmed();
            qDebug() << "[fenServer stdout]" << output;
        }
    });

    connect(fenServer, &QProcess::readyReadStandardError, this, [=]() {
        QString error = QString::fromUtf8(fenServer->readAllStandardError());
        qDebug() << "[fenServer stderr]" << error;
    });

}






void MainWindow::on_toggleAnalysisButton_clicked() {
    QString pythonExe = QCoreApplication::applicationDirPath() + "/python/python.exe";
    QString pythonScript = QCoreApplication::applicationDirPath() + "/python/fen_tracker/main.py";
    QSettings settings("ChessGUI", "ChessGUI");
    QString modelPath = settings.value("fenModelPath").toString();

    if (analysisRunning) {
        screenshotTimer->stop();
        if (fenServer && fenServer->state() != QProcess::NotRunning) {
            fenServer->kill();
            fenServer->waitForFinished(3000);
        }
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

    if (!QFile::exists(pythonExe) || !QFile::exists(pythonScript) || !QFile::exists(modelPath)) {
        QMessageBox::warning(this, tr("Missing Files"), tr("Python executable or model not found."));
        ui->toggleAnalysisButton->setChecked(false);
        return;
    }

    startFenServer();
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

    if (!stockfishProcess || stockfishProcess->state() != QProcess::Running)
        return;

    evalElapsed.restart();

    QStringList commands = {
        "uci",
        "ucinewgame",
        QString("setoption name MultiPV value %1").arg(ui->stealthCheck->isChecked() ? 3 : 1),
        "position fen " + fen,
        QString("go depth %1").arg(stockfishDepth)
    };

    multipvMoves.clear();
    selectedBestMoveRank = 1;

    for (const QString& cmd : commands) {
        stockfishProcess->write((cmd + "\n").toUtf8());
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
    automoveInProgress = true;
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

    QStringList args;
    args << scriptPath
         << from
         << to
         << QString::number(originX)
         << QString::number(originY)
         << QString::number(tileSize)
         << (flipped ? "true" : "false")
         << (stealth ? "true" : "false");

    auto execute = [=]() {
        QProcess* moveProcess = new QProcess(this);
        connect(moveProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
                    qDebug() << "[automove] Move script finished with code" << exitCode;
                    automoveInProgress = false;
                    moveProcess->deleteLater();
                    captureScreenshot();
                });

        QString embeddedPython = QCoreApplication::applicationDirPath() + "/python/python.exe";
        moveProcess->start(embeddedPython, args);


        if (!moveProcess->waitForStarted()) {
            qDebug() << "[automove] Failed to start move process";
            automoveInProgress = false;
            moveProcess->deleteLater();
        }
    };

    if (autoMoveDelayMs > 0)
        QTimer::singleShot(autoMoveDelayMs, this, execute);
    else
        execute();
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
    settingsDialog->setStockfishDepth(stockfishDepth);
    settingsDialog->setStealthModeEnabled(ui->stealthCheck->isChecked());
    settingsDialog->setUseAutoBoardDetection(useAutoBoardDetectionSetting);
    settingsDialog->setForceManualRegion(forceManualRegionSetting);
    settingsDialog->setAutoMoveWhenReady(ui->automoveCheck->isChecked());
    settingsDialog->setAutoMoveDelay(autoMoveDelayMs);
    settingsDialog->setStockfishPath(stockfishPath);
    settingsDialog->setFenModelPath(fenModelPath);
    settingsDialog->setDefaultPlayerColor(ui->whiteRadioButton->isChecked() ? "White" : "Black");
    if (settingsDialog->exec() == QDialog::Accepted) {
        analysisInterval = settingsDialog->analysisInterval();
        stockfishDepth = settingsDialog->stockfishDepth();
        ui->stealthCheck->setChecked(settingsDialog->stealthModeEnabled());
        useAutoBoardDetectionSetting = settingsDialog->useAutoBoardDetection();
        forceManualRegionSetting = settingsDialog->forceManualRegion();
        ui->automoveCheck->setChecked(settingsDialog->autoMoveWhenReady());
        autoMoveDelayMs = settingsDialog->autoMoveDelay();
        stockfishPath = settingsDialog->stockfishPath();
        fenModelPath = settingsDialog->fenModelPath();
        if (settingsDialog->defaultPlayerColor() == "Black")
            ui->blackRadioButton->setChecked(true);
        else
            ui->whiteRadioButton->setChecked(true);
        if (analysisRunning) {
            screenshotTimer->stop();
            screenshotTimer->start(analysisInterval);
        }
        if (stockfishProcess) {
            restartStockfishOnCrash = false;
            stockfishProcess->kill();
            stockfishProcess->deleteLater();
            stockfishProcess = nullptr;
            startStockfish();
        }
    }
}



