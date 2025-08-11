#include <QPushButton>
#include "ProgressLogPane.hpp"
ProgressLogPane::ProgressLogPane(QWidget* parent) : QWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    bar_ = new QProgressBar(this);
    bar_->setRange(0, 100);
    bar_->setValue(0);
    lay->addWidget(bar_);
    log_ = new QPlainTextEdit(this);
    log_->setReadOnly(true);
    log_->setMinimumHeight(100);
    lay->addWidget(log_, 1);
    auto* btns = new QHBoxLayout();
    auto* btnClear = new QPushButton("Clear Log", this);
    btns->addStretch(1);
    btns->addWidget(btnClear);
    lay->addLayout(btns);
    connect(btnClear, &QPushButton::clicked, this, &ProgressLogPane::clearLog);
}
void ProgressLogPane::setIndeterminate(bool on) { if (on) bar_->setRange(0, 0); else bar_->setRange(0, 100); }
void ProgressLogPane::setProgress(int done, int total) {
    if (total <= 0) { setIndeterminate(true); return; }
    setIndeterminate(false);
    bar_->setRange(0, total);
    bar_->setValue(done);
}
void ProgressLogPane::log(const QString& line) { log_->appendPlainText(line); }
void ProgressLogPane::clearLog() { log_->clear(); }
