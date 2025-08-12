#include "TaskBar.h"
#include <QHBoxLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QLabel>

TaskBar::TaskBar(QWidget* parent): QWidget(parent){
    label_ = new QLabel(tr("Idle"), this);
    bar_ = new QProgressBar(this);
    bar_->setRange(0,100);
    bar_->setValue(0);
    start_ = new QPushButton(tr("Assemble"), this);
    cancel_ = new QPushButton(tr("Cancel"), this);
    auto *lay = new QHBoxLayout(this);
    lay->addWidget(label_);
    lay->addWidget(bar_, 1);
    lay->addWidget(start_);
    lay->addWidget(cancel_);
    connect(start_, &QPushButton::clicked, this, &TaskBar::startRequested);
    connect(cancel_, &QPushButton::clicked, this, &TaskBar::cancelRequested);
}
void TaskBar::setActive(const QString& name){ label_->setText(name); }
void TaskBar::setIdle(){ label_->setText(tr("Idle")); bar_->setValue(0); }
void TaskBar::setProgress(int d, int t){ if(t>0) bar_->setValue(int(double(d)*100.0/double(t))); }
