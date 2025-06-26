#include "settingsdialog.h"
#include <QTabWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QCoreApplication>

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
    coreLayout->addRow(tr("Engine Depth"), depthSpinBox);

    stealthCheckBox = new QCheckBox(tr("Enable Stealth Mode"), coreTab);
    coreLayout->addRow(stealthCheckBox);
    temperatureSpinBox = new QDoubleSpinBox(coreTab);
    temperatureSpinBox->setRange(0.01, 0.10);
    temperatureSpinBox->setSingleStep(0.005);
    coreLayout->addRow(tr("Softmax Temperature"), temperatureSpinBox);
    injectSpinBox = new QSpinBox(coreTab);
    injectSpinBox->setRange(0, 30);
    coreLayout->addRow(tr("Inject 2nd Line (%)"), injectSpinBox);
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
    enginePathEdit = new QLineEdit(miscTab);
    stockfishBrowseButton = new QPushButton(tr("Browse"), miscTab);
    stockfishLayout->addWidget(enginePathEdit);
    stockfishLayout->addWidget(stockfishBrowseButton);
    QWidget *stockfishWidget = new QWidget(miscTab);
    stockfishWidget->setLayout(stockfishLayout);
    miscLayout->addRow(tr("Path to Engine Executable"), stockfishWidget);

    QHBoxLayout *fenLayout = new QHBoxLayout();
    fenModelPathEdit = new QLineEdit(miscTab);
    fenModelBrowseButton = new QPushButton(tr("Browse"), miscTab);
    fenLayout->addWidget(fenModelPathEdit);
    fenLayout->addWidget(fenModelBrowseButton);
    QWidget *fenWidget = new QWidget(miscTab);
    fenWidget->setLayout(fenLayout);
    miscLayout->addRow(tr("FEN Prediction Model Path"), fenWidget);

    QHBoxLayout *weightsLayout = new QHBoxLayout();
    weightsPathEdit = new QLineEdit(miscTab);
    weightsBrowseButton = new QPushButton(tr("Browse"), miscTab);
    weightsLayout->addWidget(weightsPathEdit);
    weightsLayout->addWidget(weightsBrowseButton);
    QWidget *weightsWidget = new QWidget(miscTab);
    weightsWidget->setLayout(weightsLayout);
    miscLayout->addRow(tr("SABLE Model File (.pth)"), weightsWidget);

    strengthComboBox = new QComboBox(miscTab);
    strengthComboBox->addItems({tr("Default")});
    miscLayout->addRow(tr("Engine Profile"), strengthComboBox);


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
    connect(weightsBrowseButton, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(this, tr("Select SABLE Model (.pth)"));
        if (!file.isEmpty())
            weightsPathEdit->setText(file);
    });


    loadSettings();
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::loadSettings()
{
    setAnalysisInterval(settings.value("analysisInterval", 1000).toInt());
    setEngineDepth(settings.value("engineDepth", 15).toInt());
    setStealthModeEnabled(settings.value("stealthMode", false).toBool());

    setUseAutoBoardDetection(settings.value("autoBoardDetection", true).toBool());
    setForceManualRegion(settings.value("forceManualRegion", false).toBool());


    setAutoMoveWhenReady(settings.value("autoMoveWhenReady", false).toBool());
    setAutoMoveDelay(settings.value("autoMoveDelay", 0).toInt());

    setStealthTemperature(settings.value("stealthTemperature", 0.035).toDouble());
    setInjectPercent(settings.value("stealthInjectPct", 10).toInt());


    QString defaultStockfish = QCoreApplication::applicationDirPath() + "/python/sable/engine.py";
    QString defaultFenModel = QCoreApplication::applicationDirPath() + "/python/fen_tracker/ccn_model_default.pth";

    QString defaultWeights = QCoreApplication::applicationDirPath() + "/python/sable/sable_model.pth";
    weightsPathEdit->setText(settings.value("weightsPath", defaultWeights).toString());


    setEnginePath(settings.value("enginePath", defaultStockfish).toString());
    setFenModelPath(settings.value("fenModelPath", defaultFenModel).toString());
    setDefaultPlayerColor(settings.value("defaultColor", "White").toString());
    setEngineStrength(settings.value("engineStrength", "Unrestricted").toString());
}

void SettingsDialog::saveSettings()
{
    settings.setValue("analysisInterval", analysisInterval());
    settings.setValue("engineDepth", engineDepth());
    settings.setValue("stealthMode", stealthModeEnabled());
    settings.setValue("autoBoardDetection", useAutoBoardDetection());
    settings.setValue("forceManualRegion", forceManualRegion());
    settings.setValue("autoMoveWhenReady", autoMoveWhenReady());
    settings.setValue("autoMoveDelay", autoMoveDelay());
    settings.setValue("enginePath", enginePath());
    settings.setValue("fenModelPath", fenModelPath());
    settings.setValue("defaultColor", defaultPlayerColor());
    settings.setValue("weightsPath", weightsPathEdit->text());
    settings.setValue("stealthTemperature", stealthTemperature());
    settings.setValue("stealthInjectPct", injectPercent());
    settings.setValue("engineStrength", engineStrength());

}

void SettingsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}

void SettingsDialog::browseStockfish()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select Engine Executable"));
    if (!file.isEmpty())
        enginePathEdit->setText(file);
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
    setEngineDepth(15);
    setStealthModeEnabled(false);
    setUseAutoBoardDetection(true);
    setForceManualRegion(false);
    setAutoMoveWhenReady(false);
    setAutoMoveDelay(0);
    setEnginePath(QCoreApplication::applicationDirPath() + "/python/sable/engine.py");
    setFenModelPath(QCoreApplication::applicationDirPath() + "/python/fen_tracker/ccn_model_default.pth");
    setDefaultPlayerColor("White");
    weightsPathEdit->setText(QCoreApplication::applicationDirPath() + "/python/sable/sable_model.pth");
    setStealthTemperature(0.035);
    setInjectPercent(10);
    setEngineStrength("Unrestricted");

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

void SettingsDialog::setEngineDepth(int depth)
{
    depthSpinBox->setValue(depth);
}

int SettingsDialog::engineDepth() const
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


void SettingsDialog::setEnginePath(const QString &path)
{
    enginePathEdit->setText(path);
}

QString SettingsDialog::enginePath() const
{
    return enginePathEdit->text();
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

void SettingsDialog::setStealthTemperature(double temp)
{
    temperatureSpinBox->setValue(temp);
}

double SettingsDialog::stealthTemperature() const
{
    return temperatureSpinBox->value();
}

void SettingsDialog::setInjectPercent(int pct)
{
    injectSpinBox->setValue(pct);
}

int SettingsDialog::injectPercent() const
{
    return injectSpinBox->value();
}

void SettingsDialog::setEngineStrength(const QString &strength)
{
    int idx = strengthComboBox->findText(strength);
    if (idx >= 0)
        strengthComboBox->setCurrentIndex(idx);
}

QString SettingsDialog::engineStrength() const
{
    return strengthComboBox->currentText();
}

void SettingsDialog::setWeightsPath(const QString &path)
{
    weightsPathEdit->setText(path);
}

QString SettingsDialog::weightsPath() const
{
    return weightsPathEdit->text();
}

