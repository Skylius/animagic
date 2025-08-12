#include "MainWindow.hpp"
#include <QTabWidget>
#include <QVBoxLayout>
#include "ui/Common/TaskBar.h"
#include "ui/DisassemblePage/DisassemblePage.h"
#include "ui/AssemblePage/AssemblePage.h"

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent){
    auto *tabs = new QTabWidget(this);
    auto dis = new DisassemblePage(this);
    auto asmbl = new AssemblePage(this);
    tabs->addTab(dis, tr("Disassemble"));
    tabs->addTab(asmbl, tr("Assemble"));

    // Wire pages to task bar
    connect(asmbl, &AssemblePage::taskStarted, taskBar_, &TaskBar::setActive);
    connect(asmbl, &AssemblePage::taskProgress, taskBar_, &TaskBar::setProgress);
    connect(asmbl, &AssemblePage::taskSucceeded, taskBar_, [this](const QString&){ taskBar_->setIdle(); });
    connect(asmbl, &AssemblePage::taskFailed, taskBar_, [this](const QString&){ taskBar_->setIdle(); });
    connect(taskBar_, &TaskBar::startRequested, asmbl, &AssemblePage::assembleNow);

    connect(dis, &DisassemblePage::taskStarted, taskBar_, &TaskBar::setActive);
    connect(dis, &DisassemblePage::taskProgress, taskBar_, &TaskBar::setProgress);
    connect(dis, &DisassemblePage::taskSucceeded, taskBar_, [this](const QString&){ taskBar_->setIdle(); });
    connect(dis, &DisassemblePage::taskFailed, taskBar_, [this](const QString&){ taskBar_->setIdle(); });
    taskBar_ = new TaskBar(this);
    auto *central = new QWidget(this);
    auto *lay = new QVBoxLayout(central);
    lay->setContentsMargins(0,0,0,0);
    lay->addWidget(tabs, 1);
    lay->addWidget(taskBar_);
    setCentralWidget(central);
    setWindowTitle(tr("Animagic"));
    resize(1000, 700);
}
