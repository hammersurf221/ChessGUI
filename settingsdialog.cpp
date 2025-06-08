#include "settingsdialog.h"
#include "ui_SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->intervalSpinBox->setRange(100, 5000);
    ui->depthSpinBox->setRange(1, 30);
    connect(ui->closeSettingsButton, &QPushButton::clicked,
            this, &SettingsDialog::on_closeSettingsButton_clicked);
    connect(ui->closeButton, &QPushButton::clicked,
            this, &SettingsDialog::accept);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setInterval(int interval)
{
    ui->intervalSpinBox->setValue(interval);
}

int SettingsDialog::interval() const
{
    return ui->intervalSpinBox->value();
}

void SettingsDialog::setDepth(int depth)
{
    ui->depthSpinBox->setValue(depth);
}

int SettingsDialog::depth() const
{
    return ui->depthSpinBox->value();
}

void SettingsDialog::on_closeSettingsButton_clicked()
{
    emit resetPgnRequested();
}
