/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen_Settings;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_3;
    QVBoxLayout *verticalLayout_3;
    QSpacerItem *verticalSpacer_4;
    QProgressBar *evalBar;
    QSpacerItem *verticalSpacer_2;
    QWidget *leftPanel;
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer_5;
    QHBoxLayout *horizontalLayout_3;
    QLabel *statusLightLabel;
    QLabel *label;
    QSpacerItem *horizontalSpacer;
    QFrame *chessBoardFrame;
    QSpacerItem *verticalSpacer_3;
    QSpacerItem *horizontalSpacer_2;
    QWidget *rightPanel;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *setRegionButton;
    QPushButton *toggleAnalysisButton;
    QGroupBox *selectSide;
    QHBoxLayout *horizontalLayout_2;
    QRadioButton *whiteRadioButton;
    QRadioButton *blackRadioButton;
    QHBoxLayout *horizontalLayout_6;
    QCheckBox *automoveCheck;
    QCheckBox *stealthCheck;
    QSpacerItem *verticalSpacer;
    QLabel *bestMoveLabel;
    QLabel *bestMoveDisplay;
    QLabel *evalScoreRightLabel;
    QLabel *evalScoreRightDisplay;
    QLabel *pgnLabel;
    QTextEdit *pgnDisplay;
    QLabel *fenLabel;
    QPlainTextEdit *fenDisplay;
    QMenuBar *menubar;
    QMenu *menusettingsTab;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1030, 887);
        actionOpen_Settings = new QAction(MainWindow);
        actionOpen_Settings->setObjectName("actionOpen_Settings");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_4);

        evalBar = new QProgressBar(centralwidget);
        evalBar->setObjectName("evalBar");
        evalBar->setMinimum(-1000);
        evalBar->setMaximum(1000);
        evalBar->setValue(0);
        evalBar->setTextVisible(false);
        evalBar->setOrientation(Qt::Orientation::Vertical);
        evalBar->setInvertedAppearance(false);

        verticalLayout_3->addWidget(evalBar);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_2);


        horizontalLayout->addLayout(verticalLayout_3);

        leftPanel = new QWidget(centralwidget);
        leftPanel->setObjectName("leftPanel");
        verticalLayout = new QVBoxLayout(leftPanel);
        verticalLayout->setObjectName("verticalLayout");
        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_5);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        statusLightLabel = new QLabel(leftPanel);
        statusLightLabel->setObjectName("statusLightLabel");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(statusLightLabel->sizePolicy().hasHeightForWidth());
        statusLightLabel->setSizePolicy(sizePolicy);
        statusLightLabel->setMinimumSize(QSize(25, 25));
        statusLightLabel->setMaximumSize(QSize(25, 25));
        statusLightLabel->setStyleSheet(QString::fromUtf8("background-color: gray;\n"
"border-radius: 8px;\n"
"border: 1px solid #444;\n"
""));

        horizontalLayout_3->addWidget(statusLightLabel);

        label = new QLabel(leftPanel);
        label->setObjectName("label");

        horizontalLayout_3->addWidget(label);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout_3);

        chessBoardFrame = new QFrame(leftPanel);
        chessBoardFrame->setObjectName("chessBoardFrame");
        chessBoardFrame->setMinimumSize(QSize(512, 512));
        chessBoardFrame->setFrameShape(QFrame::Shape::StyledPanel);
        chessBoardFrame->setFrameShadow(QFrame::Shadow::Raised);

        verticalLayout->addWidget(chessBoardFrame);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_3);


        horizontalLayout->addWidget(leftPanel);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        rightPanel = new QWidget(centralwidget);
        rightPanel->setObjectName("rightPanel");
        verticalLayout_2 = new QVBoxLayout(rightPanel);
        verticalLayout_2->setObjectName("verticalLayout_2");
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        setRegionButton = new QPushButton(rightPanel);
        setRegionButton->setObjectName("setRegionButton");

        horizontalLayout_5->addWidget(setRegionButton);

        toggleAnalysisButton = new QPushButton(rightPanel);
        toggleAnalysisButton->setObjectName("toggleAnalysisButton");

        horizontalLayout_5->addWidget(toggleAnalysisButton);


        verticalLayout_2->addLayout(horizontalLayout_5);

        selectSide = new QGroupBox(rightPanel);
        selectSide->setObjectName("selectSide");
        horizontalLayout_2 = new QHBoxLayout(selectSide);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        whiteRadioButton = new QRadioButton(selectSide);
        whiteRadioButton->setObjectName("whiteRadioButton");
        whiteRadioButton->setChecked(true);

        horizontalLayout_2->addWidget(whiteRadioButton);

        blackRadioButton = new QRadioButton(selectSide);
        blackRadioButton->setObjectName("blackRadioButton");

        horizontalLayout_2->addWidget(blackRadioButton);


        verticalLayout_2->addWidget(selectSide);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName("horizontalLayout_6");
        automoveCheck = new QCheckBox(rightPanel);
        automoveCheck->setObjectName("automoveCheck");

        horizontalLayout_6->addWidget(automoveCheck);

        stealthCheck = new QCheckBox(rightPanel);
        stealthCheck->setObjectName("stealthCheck");

        horizontalLayout_6->addWidget(stealthCheck);


        verticalLayout_2->addLayout(horizontalLayout_6);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        bestMoveLabel = new QLabel(rightPanel);
        bestMoveLabel->setObjectName("bestMoveLabel");

        verticalLayout_2->addWidget(bestMoveLabel);

        bestMoveDisplay = new QLabel(rightPanel);
        bestMoveDisplay->setObjectName("bestMoveDisplay");

        verticalLayout_2->addWidget(bestMoveDisplay);

        evalScoreRightLabel = new QLabel(rightPanel);
        evalScoreRightLabel->setObjectName("evalScoreRightLabel");

        verticalLayout_2->addWidget(evalScoreRightLabel);

        evalScoreRightDisplay = new QLabel(rightPanel);
        evalScoreRightDisplay->setObjectName("evalScoreRightDisplay");

        verticalLayout_2->addWidget(evalScoreRightDisplay);

        pgnLabel = new QLabel(rightPanel);
        pgnLabel->setObjectName("pgnLabel");

        verticalLayout_2->addWidget(pgnLabel);

        pgnDisplay = new QTextEdit(rightPanel);
        pgnDisplay->setObjectName("pgnDisplay");
        pgnDisplay->setReadOnly(true);

        verticalLayout_2->addWidget(pgnDisplay);

        fenLabel = new QLabel(rightPanel);
        fenLabel->setObjectName("fenLabel");

        verticalLayout_2->addWidget(fenLabel);

        fenDisplay = new QPlainTextEdit(rightPanel);
        fenDisplay->setObjectName("fenDisplay");
        fenDisplay->setReadOnly(true);

        verticalLayout_2->addWidget(fenDisplay);


        horizontalLayout->addWidget(rightPanel);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1030, 21));
        menusettingsTab = new QMenu(menubar);
        menusettingsTab->setObjectName("menusettingsTab");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menusettingsTab->menuAction());
        menusettingsTab->addAction(actionOpen_Settings);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actionOpen_Settings->setText(QCoreApplication::translate("MainWindow", "Open Settings...", nullptr));
        statusLightLabel->setText(QString());
        label->setText(QCoreApplication::translate("MainWindow", "Status: ", nullptr));
        setRegionButton->setText(QCoreApplication::translate("MainWindow", "Set Region", nullptr));
        toggleAnalysisButton->setText(QCoreApplication::translate("MainWindow", "Start Analysis", nullptr));
        selectSide->setTitle(QCoreApplication::translate("MainWindow", "Choose Side:", nullptr));
        whiteRadioButton->setText(QCoreApplication::translate("MainWindow", "White", nullptr));
        blackRadioButton->setText(QCoreApplication::translate("MainWindow", "Black", nullptr));
        automoveCheck->setText(QCoreApplication::translate("MainWindow", "Enable Automove", nullptr));
        stealthCheck->setText(QCoreApplication::translate("MainWindow", "Enable Stealth Mode", nullptr));
        bestMoveLabel->setText(QCoreApplication::translate("MainWindow", "Best Move:", nullptr));
        bestMoveDisplay->setText(QString());
        evalScoreRightLabel->setText(QCoreApplication::translate("MainWindow", "Evaluation Score: ", nullptr));
        evalScoreRightDisplay->setText(QString());
        pgnLabel->setText(QCoreApplication::translate("MainWindow", "Move History:", nullptr));
        fenLabel->setText(QCoreApplication::translate("MainWindow", "FEN:", nullptr));
        menusettingsTab->setTitle(QCoreApplication::translate("MainWindow", "Settings", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
