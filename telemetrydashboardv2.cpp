#include "telemetrydashboardv2.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QProgressBar>
#include <QHBoxLayout>
#include "telemetrymanager.h"

TelemetryDashboardV2::TelemetryDashboardV2(QWidget *parent)
    : QDockWidget(parent)
{
    QWidget *container = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(container);

    averageLabel = new QLabel(tr("Average Human-likeness: N/A"), container);

    QStringList categories;
    categories << "<0.2" << "0.2–0.4" << "0.4–0.6" << "0.6–0.8" << ">=0.8";

    QWidget *histWidget = new QWidget(container);
    QHBoxLayout *histLayout = new QHBoxLayout(histWidget);
    histLayout->setSpacing(4);

    for (const QString &cat : categories) {
        QVBoxLayout *binLayout = new QVBoxLayout();
        QProgressBar *bar = new QProgressBar(histWidget);
        bar->setOrientation(Qt::Vertical);
        bar->setRange(0, 1);
        bar->setValue(0);
        bar->setTextVisible(false);
        QLabel *label = new QLabel(cat, histWidget);
        label->setAlignment(Qt::AlignHCenter);
        binLayout->addWidget(bar, 1);
        binLayout->addWidget(label);
        histLayout->addLayout(binLayout);
        bars.append(bar);
    }

    histWidget->setLayout(histLayout);

    clearButton = new QPushButton(tr("Clear Telemetry"), container);

    layout->addWidget(averageLabel);
    layout->addWidget(histWidget);
    layout->addStretch();
    layout->addWidget(clearButton);

    container->setLayout(layout);
    setWidget(container);

    connect(clearButton, &QPushButton::clicked,
            this, &TelemetryDashboardV2::clearTelemetryRequested);
}

void TelemetryDashboardV2::refresh(TelemetryManager *manager)
{
    if (!manager)
        return;

    double avg = manager->averagePolicyProb() * 100.0;
    averageLabel->setText(tr("Average Human-likeness: %1%")
                          .arg(avg, 0, 'f', 1));

    QVector<int> counts = manager->policyHistogram();
    int maxVal = 1;
    for (int i = 0; i < counts.size() && i < bars.size(); ++i) {
        bars[i]->setValue(counts[i]);
        if (counts[i] > maxVal)
            maxVal = counts[i];
    }
    for (QProgressBar *bar : bars)
        bar->setMaximum(maxVal);
}
