#include "AssemblePage.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QGroupBox>
#include <QThread>
#include "AssembleWorker.h"
#include "core/MagickBackend.hpp"

AssemblePage::AssemblePage(QWidget* parent): QWidget(parent){
    framesDir_ = new QLineEdit(this);
    outFile_ = new QLineEdit(this);
    delayMs_ = new QSpinBox(this); delayMs_->setRange(1,10000); delayMs_->setValue(100);
    loopCount_ = new QSpinBox(this); loopCount_->setRange(0,1000); loopCount_->setValue(0);
    optimize_ = new QCheckBox(tr("Optimize"), this);
    fps_ = new QDoubleSpinBox(this); fps_->setRange(0.0, 240.0); fps_->setDecimals(2); fps_->setValue(0.0); // 0 means use Delay
    lossless_ = new QCheckBox(tr("Lossless (WebP)"), this);
    quality_ = new QSpinBox(this); quality_->setRange(0,100); quality_->setValue(90);
    method_ = new QSpinBox(this); method_->setRange(0,6); method_->setValue(4);
    dither_ = new QCheckBox(tr("Dither (GIF)"), this); dither_->setChecked(true);
    colors_ = new QSpinBox(this); colors_->setRange(2,256); colors_->setValue(256);
    width_ = new QSpinBox(this); width_->setRange(0,16384); width_->setValue(0);
    height_ = new QSpinBox(this); height_->setRange(0,16384); height_->setValue(0);
    auto *btnFrames = new QPushButton(tr("Browse…"), this);
    auto *btnOut = new QPushButton(tr("Browse…"), this);
    gifRadio_ = new QRadioButton(tr("GIF"), this);
    webpRadio_ = new QRadioButton(tr("WebP"), this);
    gifRadio_->setChecked(true);
    log_ = new QListWidget(this);

    auto *row1 = new QHBoxLayout; row1->addWidget(new QLabel(tr("Frames dir:"))); row1->addWidget(framesDir_); row1->addWidget(btnFrames);
    auto *row2 = new QHBoxLayout; row2->addWidget(new QLabel(tr("Output file:"))); row2->addWidget(outFile_); row2->addWidget(btnOut);
    useDelay_ = new QCheckBox(tr("Use Delay"), this); useDelay_->setChecked(true);
    useLoop_ = new QCheckBox(tr("Use Loop"), this); useLoop_->setChecked(true);
    auto *row3 = new QHBoxLayout; row3->addWidget(useDelay_); row3->addWidget(new QLabel(tr("Delay (ms):"))); row3->addWidget(delayMs_); row3->addSpacing(16); row3->addWidget(useLoop_); row3->addWidget(new QLabel(tr("Loop:"))); row3->addWidget(loopCount_); row3->addStretch();
    auto *row0 = new QHBoxLayout; row0->addWidget(new QLabel(tr("Format:"))); row0->addWidget(gifRadio_); row0->addWidget(webpRadio_); row0->addStretch();
    auto *row5 = new QHBoxLayout; row5->addWidget(new QLabel(tr("FPS (0=use delay):"))); row5->addWidget(fps_); row5->addSpacing(16);
    gifGroup_ = new QGroupBox(tr("GIF")); auto *gifLay = new QHBoxLayout; gifLay->addWidget(optimize_); gifLay->addWidget(dither_); gifLay->addWidget(new QLabel(tr("Colors:"))); gifLay->addWidget(colors_); gifLay->addStretch(); gifGroup_->setLayout(gifLay);
    webpGroup_ = new QGroupBox(tr("WebP")); auto *webpLay = new QHBoxLayout; webpLay->addWidget(lossless_); webpLay->addWidget(new QLabel(tr("Quality:"))); webpLay->addWidget(quality_); webpLay->addWidget(new QLabel(tr("Method:"))); webpLay->addWidget(method_); webpLay->addStretch(); webpGroup_->setLayout(webpLay);
    auto *row8 = new QHBoxLayout; row8->addWidget(new QLabel(tr("Resize W×H (0=keep):"))); row8->addWidget(width_); row8->addWidget(height_); row8->addStretch();
    effectiveTiming_ = new QLabel(this); effectiveTiming_->setText(tr("Effective delay: 100 ms (10.00 FPS)"));
    auto *lay = new QVBoxLayout(this); lay->addLayout(row0); lay->addLayout(row1); lay->addLayout(row2); lay->addLayout(row3);
    lay->addLayout(row5); lay->addWidget(gifGroup_); lay->addWidget(webpGroup_); lay->addLayout(row8);
    lay->addWidget(effectiveTiming_);
    lay->addWidget(log_);

    connect(btnFrames, &QPushButton::clicked, this, &AssemblePage::chooseFrames);
    connect(btnOut, &QPushButton::clicked, this, &AssemblePage::chooseOutput);
            connect(delayMs_, qOverload<int>(&QSpinBox::valueChanged), this, &AssemblePage::updateEffectiveTiming);
    connect(fps_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &AssemblePage::updateEffectiveTiming);
    connect(gifRadio_, &QRadioButton::toggled, this, [this](bool on){ if(on) updateEnablements(false); });
    connect(webpRadio_, &QRadioButton::toggled, this, [this](bool on){ if(on) updateEnablements(true); });
    connect(useDelay_, &QCheckBox::toggled, this, &AssemblePage::updateEffectiveTiming);
    updateEnablements(false);
    updateEffectiveTiming();
}

void AssemblePage::chooseFrames(){
    auto d = QFileDialog::getExistingDirectory(this, tr("Choose frames directory"));
    if(!d.isEmpty()) framesDir_->setText(d);
}
void AssemblePage::chooseOutput(){
    auto f = QFileDialog::getSaveFileName(this, tr("Save animation"), QString(), tr("GIF (*.gif);;WebP (*.webp)"));
    if(!f.isEmpty()) outFile_->setText(f);
}
void AssemblePage::runGif(){ updateEnablements(false); assembleCommon(false); }
void AssemblePage::runWebp(){ updateEnablements(true); assembleCommon(true); }

void AssemblePage::assembleCommon(bool webp){
    QString dir = framesDir_->text();
    QString out = outFile_->text();
    if(dir.isEmpty() || out.isEmpty()){ QMessageBox::warning(this, tr("Missing info"), tr("Pick frames directory and output file.")); return; }
    QDir qdir(dir);
    QStringList filters; filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp";
    auto files = qdir.entryList(filters, QDir::Files, QDir::Name);
    if(files.isEmpty()){ QMessageBox::warning(this, tr("No frames"), tr("No frame files in the directory.")); return; }
    QStringList paths; for(const auto& f : files) paths << qdir.filePath(f);
    if(workerThread_){ workerThread_->quit(); workerThread_->wait(); delete workerThread_; workerThread_ = nullptr; }
    auto worker = new AssembleWorker;
    worker->toWebp = webp;
    worker->frames = paths;
    worker->outPath = out;
    worker->loop = useLoop_ && useLoop_->isChecked() ? loopCount_->value() : 0;
    worker->delay = useDelay_ && useDelay_->isChecked() ? delayMs_->value() : 0;
    worker->optimize = optimize_->isChecked();
    worker->fps = fps_->value();
    worker->lossless = lossless_->isChecked();
    worker->quality = quality_->value();
    worker->method = method_->value();
    worker->dither = dither_->isChecked();
    worker->colors = colors_->value();
    worker->width = width_->value();
    worker->height = height_->value();
    workerThread_ = new QThread(this);
    worker->moveToThread(workerThread_);
    connect(workerThread_, &QThread::started, worker, &AssembleWorker::start);
    connect(worker, &AssembleWorker::progress, this, &AssemblePage::onWorkerProgress);
    connect(worker, &AssembleWorker::done, this, &AssemblePage::onWorkerDone);
    connect(worker, &AssembleWorker::done, workerThread_, &QThread::quit);
    connect(workerThread_, &QThread::finished, worker, &QObject::deleteLater);
    emit taskStarted(webp ? tr("Assembling WebP...") : tr("Assembling GIF..."));
    workerThread_->start();
}

void AssemblePage::onWorkerProgress(int d, int t){ emit taskProgress(d, t); }
void AssemblePage::onWorkerDone(bool ok, const QString& msg){
    if(ok){ log_->addItem(msg); QMessageBox::information(this, tr("Done"), msg); emit taskSucceeded(msg); }
    else { QMessageBox::critical(this, tr("Error"), msg); emit taskFailed(msg); }
}


void AssemblePage::updateEnablements(bool webpTarget){
    // Group enable/disable (greys out visually)
    if(gifGroup_) gifGroup_->setEnabled(!webpTarget);
    if(webpGroup_) webpGroup_->setEnabled(webpTarget);
    // Also guard individual widgets (in case)
    optimize_->setEnabled(!webpTarget);
    dither_->setEnabled(!webpTarget);
    colors_->setEnabled(!webpTarget);
    lossless_->setEnabled(webpTarget);
    quality_->setEnabled(webpTarget);
    method_->setEnabled(webpTarget);
}


void AssemblePage::updateEffectiveTiming(){
    double fpsVal = fps_->value();
    int delayVal = delayMs_->value();
    double effDelayMs = (fpsVal > 0.0) ? (1000.0 / fpsVal) : double(delayVal);
    double effFps = (effDelayMs > 0.0) ? (1000.0 / effDelayMs) : 0.0;
    if(effectiveTiming_){
        effectiveTiming_->setText(tr("Effective delay: %1 ms (%2 FPS)")
            .arg(QString::number(effDelayMs, 'f', 2))
            .arg(QString::number(effFps, 'f', 2)));
    }
}


bool AssemblePage::currentModeIsWebp() const {
    return webpRadio_ && webpRadio_->isChecked();
}

void AssemblePage::assembleNow(){
    bool webp = currentModeIsWebp();
    if(webp) runWebp(); else runGif();
}
