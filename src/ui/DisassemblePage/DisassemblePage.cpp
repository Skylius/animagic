#include "DisassemblePage.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QLabel>
#include <QThread>
#include "DisassembleWorker.h"
#include "core/MagickBackend.hpp"

DisassemblePage::DisassemblePage(QWidget* parent): QWidget(parent){
    inEdit_ = new QLineEdit(this);
    outEdit_ = new QLineEdit(this);
    auto *btnIn = new QPushButton(tr("Browse…"), this);
    auto *btnOut = new QPushButton(tr("Browse…"), this);
    auto *runBtn = new QPushButton(tr("Disassemble"), this);
    log_ = new QListWidget(this);
    auto *row1 = new QHBoxLayout; row1->addWidget(new QLabel(tr("Input file:"))); row1->addWidget(inEdit_); row1->addWidget(btnIn);
    auto *row2 = new QHBoxLayout; row2->addWidget(new QLabel(tr("Output folder:"))); row2->addWidget(outEdit_); row2->addWidget(btnOut);
    auto *lay = new QVBoxLayout(this); lay->addLayout(row1); lay->addLayout(row2); lay->addWidget(runBtn); lay->addWidget(log_);
    connect(btnIn, &QPushButton::clicked, this, &DisassemblePage::chooseInput);
    connect(btnOut, &QPushButton::clicked, this, &DisassemblePage::chooseOutput);
    connect(runBtn, &QPushButton::clicked, this, &DisassemblePage::run);
}

void DisassemblePage::chooseInput(){
    auto f = QFileDialog::getOpenFileName(this, tr("Open Animation"), QString(), tr("Animations (*.gif *.webp)"));
    if(!f.isEmpty()) inEdit_->setText(f);
}
void DisassemblePage::chooseOutput(){
    auto d = QFileDialog::getExistingDirectory(this, tr("Choose output folder"));
    if(!d.isEmpty()) outEdit_->setText(d);
}
void DisassemblePage::run(){
    QString in = inEdit_->text();
    QString out = outEdit_->text();
    if(in.isEmpty() || out.isEmpty()){ QMessageBox::warning(this, tr("Missing info"), tr("Pick input and output")); return; }
    if(workerThread_){ workerThread_->quit(); workerThread_->wait(); delete workerThread_; workerThread_=nullptr; }
    auto worker = new DisassembleWorker;
    worker->input = in;
    worker->outDir = out;
    workerThread_ = new QThread(this);
    worker->moveToThread(workerThread_);
    connect(workerThread_, &QThread::started, worker, &DisassembleWorker::start);
    connect(worker, &DisassembleWorker::progress, this, &DisassemblePage::onWorkerProgress);
    connect(worker, &DisassembleWorker::done, this, &DisassemblePage::onWorkerDone);
    connect(worker, &DisassembleWorker::done, workerThread_, &QThread::quit);
    connect(workerThread_, &QThread::finished, worker, &QObject::deleteLater);
    emit taskStarted(tr("Disassembling..."));
    workerThread_->start();
}

void DisassemblePage::onWorkerProgress(int d, int t){ emit taskProgress(d, t); }
void DisassemblePage::onWorkerDone(bool ok, const QString& msg){
    if(ok){
        log_->addItem(msg);
        QMessageBox::information(this, tr("Done"), msg);
        emit taskSucceeded(msg);
    }else{
        QMessageBox::critical(this, tr("Error"), msg);
        emit taskFailed(msg);
    }
}
