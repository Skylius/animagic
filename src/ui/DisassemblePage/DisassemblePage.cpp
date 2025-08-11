#include <QPushButton>
#include "DisassemblePage.hpp"
#include <QSpacerItem>

DisassemblePage::DisassemblePage(QWidget* parent) : QWidget(parent) {
    buildUi();
}

void DisassemblePage::buildUi() {
    auto* lay = new QVBoxLayout(this);
    auto* form = new QFormLayout();

    srcEdit_ = new QLineEdit(this);
    auto* btnSrc = new QPushButton("Browse…", this);
    auto* srcRow = new QWidget(this);
    auto* srcRowLay = new QHBoxLayout(srcRow);
    srcRowLay->setContentsMargins(0,0,0,0);
    srcRowLay->addWidget(srcEdit_, 1);
    srcRowLay->addWidget(btnSrc, 0);
    form->addRow("Source GIF/WebP:", srcRow);

    outEdit_ = new QLineEdit(this);
    auto* btnOut = new QPushButton("Browse…", this);
    auto* outRow = new QWidget(this);
    auto* outRowLay = new QHBoxLayout(outRow);
    outRowLay->setContentsMargins(0,0,0,0);
    outRowLay->addWidget(outEdit_, 1);
    outRowLay->addWidget(btnOut, 0);
    form->addRow("Output folder:", outRow);

    patternEdit_ = new QLineEdit("frame_%04d.png", this);
    form->addRow("Filename pattern:", patternEdit_);

    stillFmt_ = new QComboBox(this);
    stillFmt_->addItems({"png","jpg","jpeg","tif","tiff","bmp","webp"});
    form->addRow("Still format:", stillFmt_);

    coalesceBox_ = new QCheckBox("Coalesce frames (recommended)", this);
    coalesceBox_->setChecked(true);
    form->addRow("", coalesceBox_);

    lay->addLayout(form);

    infoLabel_ = new QLabel("—", this);
    lay->addWidget(infoLabel_);

    auto* btnRow = new QHBoxLayout();
    btnAnalyze_ = new QPushButton("Analyze", this);
    btnExtract_ = new QPushButton("Extract Frames", this);
    btnCancel_ = new QPushButton("Cancel", this);
    btnRow->addWidget(btnAnalyze_);
    btnRow->addWidget(btnExtract_);
    btnRow->addWidget(btnCancel_);
    btnRow->addStretch(1);
    lay->addLayout(btnRow);

    taskBar_ = new TaskBar(this);
    taskBar_->setVisible(false);
    lay->addWidget(taskBar_);

    progressLog_ = new ProgressLogPane(this);
    lay->addWidget(progressLog_, 1);

    connect(btnSrc, &QPushButton::clicked, this, &DisassemblePage::onBrowseSource);
    connect(btnOut, &QPushButton::clicked, this, &DisassemblePage::onBrowseOutFolder);
    connect(btnAnalyze_, &QPushButton::clicked, this, &DisassemblePage::onAnalyze);
    connect(btnExtract_, &QPushButton::clicked, this, &DisassemblePage::onExtract);

    connect(&vm_, &DisassembleViewModel::progress, this, [this](int done, int total){ progressLog_->setProgress(done, total); });
    connect(&vm_, &DisassembleViewModel::logMessage, this, &DisassemblePage::log);

    if (!runner_) runner_ = new TaskRunner(this);
    connect(btnCancel_, &QPushButton::clicked, runner_, &TaskRunner::cancelCurrent);
    connect(taskBar_, &TaskBar::cancelRequested, runner_, &TaskRunner::cancelCurrent);
    connect(runner_, &TaskRunner::taskProgress, this, [this](int d,int t){ if (progressLog_) progressLog_->setProgress(d,t); if (taskBar_) taskBar_->setProgress(d,t); });
    connect(runner_, &TaskRunner::taskLog, this, [this](const QString& s){ log(s); });
    connect(runner_, &TaskRunner::taskStarted, this, [this](const QString& name){ if (progressLog_) progressLog_->log("Started: " + name); if (taskBar_) taskBar_->setActive(name); });
    connect(runner_, &TaskRunner::taskSucceeded, this, [this](const QString& name){ if (progressLog_) progressLog_->log("Succeeded: " + name); if (taskBar_) taskBar_->setIdle(); });
    connect(runner_, &TaskRunner::taskFailed, this, [this](const QString& name, const QString& err){ if (progressLog_) progressLog_->log("Failed: " + name + ": " + err); if (taskBar_) taskBar_->setIdle(); });
    connect(runner_, &TaskRunner::taskCancelled, this, [this](const QString& name){ if (progressLog_) progressLog_->log("Cancelled: " + name); if (taskBar_) taskBar_->setIdle(); });
}

void DisassemblePage::log(const QString& line) {
    if (progressLog_) progressLog_->log(line);
}

void DisassemblePage::onBrowseSource() {
    QString f = QFileDialog::getOpenFileName(this, "Choose source animation", QString(), "Animations (*.gif *.webp)");
    if (f.isEmpty()) return;
    srcEdit_->setText(f);
}

void DisassemblePage::onBrowseOutFolder() {
    QString d = QFileDialog::getExistingDirectory(this, "Choose output folder");
    if (d.isEmpty()) return;
    outEdit_->setText(d);
}

void DisassemblePage::onAnalyze() {
    vm_.setSourcePath(srcEdit_->text());
    auto r = vm_.analyze();
    if (r.ok) {
        infoLabel_->setText(QString("Format: %1 • %2x%3 • %4 frames • loop=%5")
                            .arg(r.format).arg(r.width).arg(r.height).arg(r.frames).arg(r.loop));
        emit analyzed();
    } else {
        infoLabel_->setText("Analyze failed. See log.");
    }
}

void DisassemblePage::onExtract() {
    vm_.setSourcePath(srcEdit_->text());
    vm_.setOutFolder(outEdit_->text());
    vm_.setPattern(patternEdit_->text());
    vm_.setStillFormat(stillFmt_->currentText());
    vm_.setCoalesce(coalesceBox_->isChecked());

    const QString source = srcEdit_->text();
    const QString outDir = outEdit_->text();
    const QString pattern = patternEdit_->text();
    const QString stillFmt = stillFmt_->currentText();
    const bool coalesce = coalesceBox_->isChecked();

    Task* t = new Task("Extract frames", [source, outDir, pattern, stillFmt, coalesce](CancellationToken tok, Task::ProgressFn prog, Task::LogFn log){
        // Free space check: warn if less than 200 MB free in output folder
        quint64 avail = 0;
        if (SpaceCheck::hasAtLeast(outDir, 200ull * 1024ull * 1024ull, &avail)) {
            log(QString("Free space: %1 MB").arg(static_cast<qulonglong>(avail / (1024ull*1024ull))));
        } else {
            log(QString("Warning: low free space (~%1 MB) in output folder").arg(static_cast<qulonglong>(avail / (1024ull*1024ull))));
        }

        auto vr = Validators::validate(Validators::DisassembleParams{source, outDir, pattern, stillFmt, coalesce});
        if (!vr.ok) { throw std::runtime_error(vr.errors.join('\n').toStdString()); }
        log(QString("Extracting from %1 → %2 (%3)").arg(source, outDir, pattern));

        MagickBackend backend;
        backend.extractFrames(source, outDir, pattern, stillFmt, coalesce,
            [prog](int done, int total){ prog(done, total); });
        log("Extraction complete.");

        auto p = backend.probeAnimation(source);
        ProjectMetadata md;
        md.source.path = source;
        md.source.format = p.format;
        md.source.width = p.width;
        md.source.height = p.height;
        md.source.frames = p.frames;
        md.source.loop = p.loop;
        md.source.perFrameDelayMs = p.perFrameDelayMs;
        md.stills.folder = outDir;
        md.stills.pattern = pattern;
        md.stills.format = stillFmt;
        md.assembly.targetFormat = "gif";
        const QString metaPath = QDir(outDir).filePath("animagic.meta.json");
        QString err;
        if (!ProjectMetadata::saveToFile(metaPath, md, &err)) {
            log(QString("Metadata write failed: %1").arg(err));
        } else {
            log(QString("Wrote metadata: %1").arg(metaPath));
        }
    }, this);

    runner_->queue(t);
}
