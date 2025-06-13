#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H


#include <QDialog>
#include <QSettings>

class QTabWidget;
class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QComboBox;
class QDialogButtonBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override;

    // Core settings
    void setAnalysisInterval(int interval);
    int analysisInterval() const;
    void setEngineDepth(int depth);
    int engineDepth() const;
    void setStealthModeEnabled(bool enabled);
    bool stealthModeEnabled() const;
    void setStealthTemperature(double temp);
    double stealthTemperature() const;
    void setInjectPercent(int pct);
    int injectPercent() const;

    // Board detection
    void setUseAutoBoardDetection(bool use);
    bool useAutoBoardDetection() const;
    void setForceManualRegion(bool force);
    bool forceManualRegion() const;

    // Move automation
    void setAutoMoveWhenReady(bool enable);
    bool autoMoveWhenReady() const;
    void setAutoMoveDelay(int delay);
    int autoMoveDelay() const;

    // Misc
    void setEnginePath(const QString &path);
    QString enginePath() const;
    void setFenModelPath(const QString &path);
    QString fenModelPath() const;
    void setDefaultPlayerColor(const QString &color);
    QString defaultPlayerColor() const;
    void setEngineStrength(const QString &strength);
    QString engineStrength() const;

    QString weightsPath() const;
    void setWeightsPath(const QString &path);


signals:
    void resetPgnRequested();

private slots:
    void browseStockfish();
    void browseFenModel();
    void resetDefaults();
    void accept() override;

private:
    void loadSettings();
    void saveSettings();

    QLineEdit *weightsPathEdit;
    QPushButton *weightsBrowseButton;


    QTabWidget *tabs;
    QSpinBox *intervalSpinBox;
    QSpinBox *depthSpinBox;
    QCheckBox *stealthCheckBox;

    QCheckBox *autoBoardDetectCheckBox;
    QCheckBox *forceManualRegionCheckBox;
    QCheckBox *autoMoveCheckBox;
    QSpinBox *autoMoveDelaySpinBox;
    QLineEdit *enginePathEdit;
    QPushButton *stockfishBrowseButton;
    QLineEdit *fenModelPathEdit;
    QPushButton *fenModelBrowseButton;
    QComboBox *colorComboBox;
    QDoubleSpinBox *temperatureSpinBox;
    QSpinBox *injectSpinBox;
    QComboBox *strengthComboBox;

    QPushButton *resetButton;
    QDialogButtonBox *buttonBox;

    QSettings settings;
};

#endif // SETTINGSDIALOG_H
