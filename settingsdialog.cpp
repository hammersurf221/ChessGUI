#include "settingsdialog.h"
#include <QTabWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent), settings("ChessGUI", "ChessGUI")
{
    tabs = new QTabWidget(this);

    // Core tab
    QWidget *coreTab = new QWidget(this);
    QFormLayout *coreLayout = new QFormLayout(coreTab);
    intervalSpinBox = new QSpinBox(coreTab);
    intervalSpinBox->setRange(100, 5000);
    coreLayout->addRow(tr("Analysis Interval (ms)"), intervalSpinBox);

    depthSpinBox = new QSpinBox(coreTab);
    depthSpinBox->setRange(1, 30);
    coreLayout->addRow(tr("Stockfish Depth"), depthSpinBox);

    stealthCheckBox = new QCheckBox(tr("Enable Stealth Mode"), coreTab);
    coreLayout->addRow(stealthCheckBox);
    coreTab->setLayout(coreLayout);
    tabs->addTab(coreTab, tr("Core"));

    // Board detection tab
    QWidget *boardTab = new QWidget(this);
    QVBoxLayout *boardLayout = new QVBoxLayout(boardTab);
    autoBoardDetectCheckBox = new QCheckBox(tr("Use Automatic Board Detection"), boardTab);
    forceManualRegionCheckBox = new QCheckBox(tr("Force Manual Region on Startup"), boardTab);
    boardLayout->addWidget(autoBoardDetectCheckBox);
    boardLayout->addWidget(forceManualRegionCheckBox);
    boardTab->setLayout(boardLayout);
    tabs->addTab(boardTab, tr("Board Detection"));


    // Move automation tab
    QWidget *moveTab = new QWidget(this);
    QFormLayout *moveLayout = new QFormLayout(moveTab);
    autoMoveCheckBox = new QCheckBox(tr("Auto-move When Ready"), moveTab);
    moveLayout->addRow(autoMoveCheckBox);
    autoMoveDelaySpinBox = new QSpinBox(moveTab);
    autoMoveDelaySpinBox->setRange(0, 3000);
    moveLayout->addRow(tr("Delay Before Auto-move (ms)"), autoMoveDelaySpinBox);
    moveTab->setLayout(moveLayout);
    tabs->addTab(moveTab, tr("Move Automation"));


    // Misc tab
    QWidget *miscTab = new QWidget(this);
    QFormLayout *miscLayout = new QFormLayout(miscTab);
    QHBoxLayout *stockfishLayout = new QHBoxLayout();
    stockfishPathEdit = new QLineEdit(miscTab);
    stockfishBrowseButton = new QPushButton(tr("Browse"), miscTab);
    stockfishLayout->addWidget(stockfishPathEdit);
    stockfishLayout->addWidget(stockfishBrowseButton);
    QWidget *stockfishWidget = new QWidget(miscTab);
    stockfishWidget->setLayout(stockfishLayout);
    miscLayout->addRow(tr("Path to Stockfish Executable"), stockfishWidget);

    QHBoxLayout *fenLayout = new QHBoxLayout();
    fenModelPathEdit = new QLineEdit(miscTab);
    fenModelBrowseButton = new QPushButton(tr("Browse"), miscTab);
    fenLayout->addWidget(fenModelPathEdit);
    fenLayout->addWidget(fenModelBrowseButton);
    QWidget *fenWidget = new QWidget(miscTab);
    fenWidget->setLayout(fenLayout);
    miscLayout->addRow(tr("FEN Prediction Model Path"), fenWidget);

    colorComboBox = new QComboBox(miscTab);
    colorComboBox->addItems({tr("White"), tr("Black")});
    miscLayout->addRow(tr("Default Player Color"), colorComboBox);
    miscTab->setLayout(miscLayout);
    tabs->addTab(miscTab, tr("Misc"));

    // Buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    resetButton = new QPushButton(tr("Reset to Defaults"), this);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(resetButton);
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(buttonBox);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabs);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
    connect(resetButton, &QPushButton::clicked, this, &SettingsDialog::resetDefaults);
    connect(stockfishBrowseButton, &QPushButton::clicked, this, &SettingsDialog::browseStockfish);
    connect(fenModelBrowseButton, &QPushButton::clicked, this, &SettingsDialog::browseFenModel);

    loadSettings();
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::loadSettings()
{
    setAnalysisInterval(settings.value("analysisInterval", 1000).toInt());
    setStockfishDepth(settings.value("stockfishDepth", 15).toInt());
    setStealthModeEnabled(settings.value("stealthMode", false).toBool());

    setUseAutoBoardDetection(settings.value("autoBoardDetection", true).toBool());
    setForceManualRegion(settings.value("forceManualRegion", false).toBool());


    setAutoMoveWhenReady(settings.value("autoMoveWhenReady", false).toBool());
    setAutoMoveDelay(settings.value("autoMoveDelay", 0).toInt());


    setStockfishPath(settings.value("stockfishPath").toString());
    setFenModelPath(settings.value("fenModelPath").toString());
    setDefaultPlayerColor(settings.value("defaultColor", "White").toString());
}

void SettingsDialog::saveSettings()
{
    settings.setValue("analysisInterval", analysisInterval());
    settings.setValue("stockfishDepth", stockfishDepth());
    settings.setValue("stealthMode", stealthModeEnabled());
    settings.setValue("autoBoardDetection", useAutoBoardDetection());
    settings.setValue("forceManualRegion", forceManualRegion());
    settings.setValue("autoMoveWhenReady", autoMoveWhenReady());
    settings.setValue("autoMoveDelay", autoMoveDelay());
    settings.setValue("stockfishPath", stockfishPath());
    settings.setValue("fenModelPath", fenModelPath());
    settings.setValue("defaultColor", defaultPlayerColor());
}

void SettingsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}

void SettingsDialog::browseStockfish()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select Stockfish Executable"));
    if (!file.isEmpty())
        stockfishPathEdit->setText(file);
}

void SettingsDialog::browseFenModel()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select FEN Model"));
    if (!file.isEmpty())
        fenModelPathEdit->setText(file);
}

void SettingsDialog::resetDefaults()
{
    setAnalysisInterval(1000);
    setStockfishDepth(15);
    setStealthModeEnabled(false);
    setUseAutoBoardDetection(true);
    setForceManualRegion(false);
    setAutoMoveWhenReady(false);
    setAutoMoveDelay(0);
    setStockfishPath(QString());
    setFenModelPath(QString());
    setDefaultPlayerColor("White");
}

// Getter and setter implementations
void SettingsDialog::setAnalysisInterval(int interval)
{
    intervalSpinBox->setValue(interval);
}

int SettingsDialog::analysisInterval() const
{
    return intervalSpinBox->value();
}

void SettingsDialog::setStockfishDepth(int depth)
{
    depthSpinBox->setValue(depth);
}

int SettingsDialog::stockfishDepth() const
{
    return depthSpinBox->value();
}

void SettingsDialog::setStealthModeEnabled(bool enabled)
{
    stealthCheckBox->setChecked(enabled);
}

bool SettingsDialog::stealthModeEnabled() const
{
    return stealthCheckBox->isChecked();
}

void SettingsDialog::setUseAutoBoardDetection(bool use)
{
    autoBoardDetectCheckBox->setChecked(use);
}

bool SettingsDialog::useAutoBoardDetection() const
{
    return autoBoardDetectCheckBox->isChecked();
}

void SettingsDialog::setForceManualRegion(bool force)
{
    forceManualRegionCheckBox->setChecked(force);
}

bool SettingsDialog::forceManualRegion() const
{
    return forceManualRegionCheckBox->isChecked();
}


void SettingsDialog::setAutoMoveWhenReady(bool enable)
{
    autoMoveCheckBox->setChecked(enable);
}

bool SettingsDialog::autoMoveWhenReady() const
{
    return autoMoveCheckBox->isChecked();
}

void SettingsDialog::setAutoMoveDelay(int delay)
{
    autoMoveDelaySpinBox->setValue(delay);
}

int SettingsDialog::autoMoveDelay() const
{
    return autoMoveDelaySpinBox->value();
}


void SettingsDialog::setStockfishPath(const QString &path)
{
    stockfishPathEdit->setText(path);
}

QString SettingsDialog::stockfishPath() const
{
    return stockfishPathEdit->text();
}

void SettingsDialog::setFenModelPath(const QString &path)
{
    fenModelPathEdit->setText(path);
}

QString SettingsDialog::fenModelPath() const
{
    return fenModelPathEdit->text();
}

void SettingsDialog::setDefaultPlayerColor(const QString &color)
{
    int index = colorComboBox->findText(color);
    if (index >= 0)
        colorComboBox->setCurrentIndex(index);
}

QString SettingsDialog::defaultPlayerColor() const
{
    return colorComboBox->currentText();
}

