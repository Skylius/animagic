#include <QPushButton>
#include "AssemblePage.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileInfo>
#include "config/AppSettings.hpp"
#include "platform/SpaceCheck.hpp"
#include <QDirIterator>
AssemblePage::AssemblePage(QWidget* parent) : QWidget(parent) { buildUi(); }
AssemblePage::AssemblePage(TaskRunner* sharedRunner, QWidget* parent) : QWidget(parent) { runner_ = sharedRunner; buildUi(); }
void AssemblePage::buildUi() {
    auto* split = new QSplitter(Qt::Horizontal, this);
    auto* left = new QWidget(split);
    auto* right = new QWidget(split);
    split->addWidget(left); split->addWidget(right);
    split->setStretchFactor(0, 1); split->setStretchFactor(1, 1);
    auto* leftLay = new QVBoxLayout(left);
    listToolbar_ = new QToolBar(left);
    auto* actAddFiles = listToolbar_->addAction("Add Files…");
    auto* actAddFolder = listToolbar_->addAction("Add Folder…");
    listToolbar_->addSeparator();
    auto* actRemove = listToolbar_->addAction("Remove");
    auto* actDuplicate = listToolbar_->addAction("Duplicate");
    listToolbar_->addSeparator();
    auto* actUp = listToolbar_->addAction("Up");
    auto* actDown = listToolbar_->addAction("Down");
    listToolbar_->addSeparator();
    auto* actClear = listToolbar_->addAction("Clear");
    listToolbar_->addSeparator();
    auto* actEditTiming = listToolbar_->addAction("Edit Timing…");
    leftLay->addWidget(listToolbar_);
    model_ = new FrameListModel(this);
    list_ = new CenteredListView(left);
    list_->setPlaceholderText("Drop frames here or use Add…");
    list_->setModel(model_);
    list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    leftLay->addWidget(list_, 1);
    auto* rightLay = new QVBoxLayout(right);
    preview_ = new PreviewWidget(right);
    preview_->setMinimumSize(320, 240);
    rightLay->addWidget(preview_, 1);
    auto* formBox = new QGroupBox("Assembly Settings", right);
    auto* form = new QFormLayout(formBox);
    targetFmt_ = new QComboBox(formBox);
    targetFmt_->addItems({"gif","webp"});
    loopSpin_ = new QSpinBox(formBox); loopSpin_->setRange(0, 1000); loopSpin_->setValue(0);
    delayMsSpin_ = new QSpinBox(formBox); delayMsSpin_->setRange(0, 60000); delayMsSpin_->setValue(100);
    fpsSpin_ = new QDoubleSpinBox(formBox); fpsSpin_->setRange(0.0, 120.0); fpsSpin_->setDecimals(2); fpsSpin_->setValue(0.0);
    form->addRow("Target format:", targetFmt_);
    form->addRow("Loop count (0=∞):", loopSpin_);
    form->addRow("Global delay (ms):", delayMsSpin_);
    form->addRow("FPS (alt to delay):", fpsSpin_);
    auto* gifBox = new QGroupBox("GIF Options", formBox);
    auto* gifForm = new QFormLayout(gifBox);
    gifOptimize_ = new QCheckBox("Optimize", gifBox); gifOptimize_->setChecked(true);
    gifDither_ = new QCheckBox("Dither", gifBox); gifDither_->setChecked(true);
    gifColors_ = new QSpinBox(gifBox); gifColors_->setRange(2, 256); gifColors_->setValue(256);
    gifForm->addRow(gifOptimize_); gifForm->addRow(gifDither_); gifForm->addRow("Colors:", gifColors_);
    auto* webpBox = new QGroupBox("WebP Options", formBox);
    auto* webpForm = new QFormLayout(webpBox);
    webpLossless_ = new QCheckBox("Lossless", webpBox);
    webpQuality_ = new QSpinBox(webpBox); webpQuality_->setRange(0, 100); webpQuality_->setValue(85);
    webpMethod_ = new QSpinBox(webpBox); webpMethod_->setRange(0, 6); webpMethod_->setValue(4);
    webpNearLossless_ = new QSpinBox(webpBox); webpNearLossless_->setRange(0, 100); webpNearLossless_->setValue(0);
    webpAlphaQuality_ = new QSpinBox(webpBox); webpAlphaQuality_->setRange(0, 100); webpAlphaQuality_->setValue(100);
    webpForm->addRow(webpLossless_); webpForm->addRow("Quality:", webpQuality_);
    webpForm->addRow("Method:", webpMethod_); webpForm->addRow("Near-lossless:", webpNearLossless_);
    webpForm->addRow("Alpha quality:", webpAlphaQuality_);
    form->addRow(gifBox); form->addRow(webpBox);
    rightLay->addWidget(formBox);
    auto* controls = new QHBoxLayout();
    auto* btnPlay = new QPushButton("Play", right);
    auto* btnPause = new QPushButton("Pause", right);
    btnAssemble_ = new QPushButton("Assemble…", right);
    auto* btnImportMeta = new QPushButton("Import Metadata…", right);
    auto* btnApplyProfile = new QPushButton("Apply Profile…", right);
    controls->addWidget(btnPlay); controls->addWidget(btnPause); controls->addStretch(1);
    controls->addWidget(btnImportMeta); controls->addWidget(btnApplyProfile); controls->addWidget(btnAssemble_);
    btnCancel_ = new QPushButton("Cancel", right);
    controls->addWidget(btnCancel_);
    rightLay->addLayout(controls);
    progressLog_ = new ProgressLogPane(right);
    progressLog_->setMinimumHeight(140);
    rightLay->addWidget(progressLog_);
    connect(actAddFiles, &QAction::triggered, this, &AssemblePage::onAddFiles);
    connect(actAddFolder, &QAction::triggered, this, &AssemblePage::onAddFolder);
    connect(actRemove, &QAction::triggered, this, &AssemblePage::onRemove);
    connect(actDuplicate, &QAction::triggered, this, &AssemblePage::onDuplicate);
    connect(actUp, &QAction::triggered, this, &AssemblePage::onMoveUp);
    connect(actDown, &QAction::triggered, this, &AssemblePage::onMoveDown);
    connect(actClear, &QAction::triggered, this, &AssemblePage::onClear);
    connect(actEditTiming, &QAction::triggered, this, &AssemblePage::onEditTiming);
    connect(btnPlay, &QPushButton::clicked, this, &AssemblePage::onPreviewPlay);
    connect(btnPause, &QPushButton::clicked, this, &AssemblePage::onPreviewPause);
    connect(btnAssemble_, &QPushButton::clicked, this, &AssemblePage::onAssemble);
    connect(btnImportMeta, &QPushButton::clicked, this, &AssemblePage::onImportMetadata);
    connect(btnApplyProfile, &QPushButton::clicked, this, &AssemblePage::onApplyProfile);
    // Apply last assemble settings
    {
        auto a = AppSettings::instance().lastAssemble();
        int tdx = targetFmt_->findText(a.targetFormat); if (tdx>=0) targetFmt_->setCurrentIndex(tdx);
        loopSpin_->setValue(a.loop);
        if (a.delayMs.has_value()) delayMsSpin_->setValue(*a.delayMs);
        if (a.fps.has_value()) fpsSpin_->setValue(*a.fps);
        gifOptimize_->setChecked(a.gif.optimize);
        gifDither_->setChecked(a.gif.dither);
        gifColors_->setValue(a.gif.colors);
        webpLossless_->setChecked(a.webp.lossless);
        webpQuality_->setValue(a.webp.quality);
        webpMethod_->setValue(a.webp.method);
        webpNearLossless_->setValue(a.webp.nearLossless);
        webpAlphaQuality_->setValue(a.webp.alphaQuality);
    }
    /* APPLY_LAST_ASSEMBLE */

    if (!runner_) runner_ = new TaskRunner(this);
    connect(btnCancel_, &QPushButton::clicked, runner_, &TaskRunner::cancelCurrent);
    connect(taskBar_, &TaskBar::cancelRequested, runner_, &TaskRunner::cancelCurrent);
    connect(runner_, &TaskRunner::taskProgress, this, [this](int d,int t){ if (progressLog_) progressLog_->setProgress(d,t); if (taskBar_) taskBar_->setProgress(d,t); });
    connect(runner_, &TaskRunner::taskLog, this, [this](const QString& s){ log(s); });
    connect(runner_, &TaskRunner::taskStarted, this, [this](const QString& name){ if (progressLog_) progressLog_->log("Started: " + name); if (taskBar_) taskBar_->setActive(name); });
    connect(runner_, &TaskRunner::taskSucceeded, this, [this](const QString& name){ if (progressLog_) progressLog_->log("Succeeded: " + name); if (taskBar_) taskBar_->setIdle(); });
    connect(runner_, &TaskRunner::taskFailed, this, [this](const QString& name, const QString& err){ if (progressLog_) progressLog_->log("Failed: " + name + ": " + err); if (taskBar_) taskBar_->setIdle(); });
    connect(runner_, &TaskRunner::taskCancelled, this, [this](const QString& name){ if (progressLog_) progressLog_->log("Cancelled: " + name); if (taskBar_) taskBar_->setIdle(); });
    auto* outer = new QVBoxLayout(this); outer->setContentsMargins(0,0,0,0); outer->addWidget(split);
}
void AssemblePage::log(const QString& line) { if (progressLog_) progressLog_->log(line); }
void AssemblePage::onAddFiles() {
    QStringList files = QFileDialog::getOpenFileNames(this, "Add frames", QString(), "Images (*.png *.jpg *.jpeg *.tif *.tiff *.bmp *.webp)");
    if (files.isEmpty()) return;
    QVector<FrameOps::FrameItem> items = model_->items();
    for (const auto& f : files) items.push_back({f, true, std::nullopt});
    model_->setItems(items);
}
void AssemblePage::onAddFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, "Add folder of frames");
    if (dir.isEmpty()) return;
    auto loaded = FrameOps::loadFromFolder(dir);
    QVector<FrameOps::FrameItem> items = model_->items();
    items += loaded;
    model_->setItems(items);
}
void AssemblePage::onRemove() {
    auto sel = list_->selectionModel()->selectedIndexes();
    if (sel.isEmpty()) return;
    QVector<int> idx; idx.reserve(sel.size());
    for (const auto& mi : sel) idx.push_back(mi.row());
    model_->removeRowsByIndices(idx);
}
void AssemblePage::onDuplicate() {
    auto sel = list_->selectionModel()->selectedIndexes();
    if (sel.isEmpty()) return;
    QVector<int> idx; idx.reserve(sel.size());
    for (const auto& mi : sel) idx.push_back(mi.row());
    model_->duplicateRowsByIndices(idx);
}
void AssemblePage::onMoveUp() {
    auto sel = list_->selectionModel()->selectedIndexes();
    if (sel.isEmpty()) return;
    int r = sel.first().row();
    if (r > 0) model_->moveRow(r, r-1);
}
void AssemblePage::onMoveDown() {
    auto sel = list_->selectionModel()->selectedIndexes();
    if (sel.isEmpty()) return;
    int r = sel.last().row();
    if (r >= 0 && r < model_->rowCount()-1) model_->moveRow(r, r+1);
}
void AssemblePage::onClear() { model_->clear(); }
void AssemblePage::onEditTiming() {
    auto sel = list_->selectionModel()->selectedIndexes();
    if (sel.isEmpty()) return;
    std::optional<int> init;
    const auto& items = model_->items();
    if (items[sel.first().row()].delayMs.has_value()) init = items[sel.first().row()].delayMs;
    FrameTimingEditor dlg(this);
    dlg.setInitialDelay(init);
    if (dlg.exec() == QDialog::Accepted) {
        std::optional<int> val = dlg.delayMs();
        QVector<int> idx; idx.reserve(sel.size());
        for (const auto& mi : sel) idx.push_back(mi.row());
        model_->setDelayMs(idx, val);
    }
}
void AssemblePage::onPreviewPlay() {
    preview_->stop();
    QStringList paths = model_->includedPaths();
    if (paths.isEmpty()) { log("No frames to preview."); return; }
    QVector<std::optional<int>> overrides; overrides.reserve(model_->items().size());
    for (const auto& it : model_->items()) overrides.push_back(it.delayMs);
    auto delays = Timing::buildDelays(paths.size(),
        delayMsSpin_->value() > 0 ? std::optional<int>(delayMsSpin_->value()) : std::nullopt,
        fpsSpin_->value() > 0 ? std::optional<double>(fpsSpin_->value()) : std::nullopt,
        overrides);
    QString err;
    if (!preview_->loadFrames(paths, delays, loopSpin_->value(), &err)) { log(QString("Preview load failed: %1").arg(err)); return; }
    preview_->play();
}
void AssemblePage::onPreviewPause() { preview_->pause(); }
QVector<std::optional<int>> AssemblePage::collectOverrides() const {
    QVector<std::optional<int>> out; out.reserve(model_->items().size());
    for (const auto& it : model_->items()) out.push_back(it.delayMs);
    return out;
}
void AssemblePage::onImportMetadata() {
    QString f = QFileDialog::getOpenFileName(this, "Open metadata", QString(), "JSON (*.json)");
    if (f.isEmpty()) return;
    QString err;
    auto md = ProjectMetadata::loadFromFile(f, &err);
    if (!md.has_value()) { log(QString("Failed to load metadata: %1").arg(err)); return; }
    targetFmt_->setCurrentText(md->assembly.targetFormat);
    loopSpin_->setValue(md->assembly.loop);
    if (md->assembly.delayMs.has_value()) delayMsSpin_->setValue(*md->assembly.delayMs);
    if (md->assembly.fps.has_value()) fpsSpin_->setValue(*md->assembly.fps);
    gifOptimize_->setChecked(md->assembly.gif.optimize);
    gifDither_->setChecked(md->assembly.gif.dither);
    gifColors_->setValue(md->assembly.gif.colors);
    webpLossless_->setChecked(md->assembly.webp.lossless);
    webpQuality_->setValue(md->assembly.webp.quality);
    webpMethod_->setValue(md->assembly.webp.method);
    webpNearLossless_->setValue(md->assembly.webp.nearLossless);
    webpAlphaQuality_->setValue(md->assembly.webp.alphaQuality);
    log(QString("Applied assembly prefs from: %1").arg(f));
}
void AssemblePage::onApplyProfile() {
    QMenu menu(this);
    auto names = Profiles::Store::list();
    if (names.isEmpty()) { log("No profiles found."); return; }
    QAction* chosen = nullptr;
    for (const auto& n : names) menu.addAction(n);
    chosen = menu.exec(QCursor::pos());
    if (!chosen) return;
    QString err;
    auto prof = Profiles::Store::load(chosen->text(), &err);
    if (!prof.has_value()) { log(QString("Failed to load profile: %1").arg(err)); return; }
    targetFmt_->setCurrentText(prof->prefs.targetFormat);
    loopSpin_->setValue(prof->prefs.loop);
    delayMsSpin_->setValue(prof->prefs.delayMs.value_or(100));
    fpsSpin_->setValue(prof->prefs.fps.value_or(0.0));
    gifOptimize_->setChecked(prof->prefs.gif.optimize);
    gifDither_->setChecked(prof->prefs.gif.dither);
    gifColors_->setValue(prof->prefs.gif.colors);
    webpLossless_->setChecked(prof->prefs.webp.lossless);
    webpQuality_->setValue(prof->prefs.webp.quality);
    webpMethod_->setValue(prof->prefs.webp.method);
    webpNearLossless_->setValue(prof->prefs.webp.nearLossless);
    webpAlphaQuality_->setValue(prof->prefs.webp.alphaQuality);
    log(QString("Applied profile: %1").arg(prof->name));
}
void AssemblePage::onAssemble() {
    QString out = QFileDialog::getSaveFileName(this, "Save animation", QString(),
        targetFmt_->currentText() == "gif" ? "GIF (*.gif)" : "WebP (*.webp)");
    if (out.isEmpty()) return;

    Validators::AssembleParams p;
    p.frames = model_->includedPaths();
    p.outPath = out;
    p.loop = loopSpin_->value();
    if (delayMsSpin_->value() > 0) p.delayMs = delayMsSpin_->value();
    if (fpsSpin_->value() > 0) p.fps = fpsSpin_->value();
    p.targetFormat = targetFmt_->currentText();
    p.gif.optimize = gifOptimize_->isChecked();
    p.gif.dither = gifDither_->isChecked();
    p.gif.colors = gifColors_->value();
    p.webp.lossless = webpLossless_->isChecked();
    p.webp.quality = webpQuality_->value();
    p.webp.method = webpMethod_->value();
    p.webp.nearLossless = webpNearLossless_->value();
    p.webp.alphaQuality = webpAlphaQuality_->value();

    auto vr = Validators::validate(p);
    if (!vr.ok) { QMessageBox::warning(this, "Assemble", vr.errors.join('\n')); return; }
    for (const auto& w : vr.warnings) log(QString("Warning: %1").arg(w));

    const auto targetFmt = p.targetFormat;
    const auto frames = p.frames;
    const QString outPath = p.outPath;
    const int loop = p.loop;
    const auto delayMs = p.delayMs;
    const auto fps = p.fps;
    const auto gif = p.gif;
    const auto webp = p.webp;

    Task* t = new Task("Assemble animation", [targetFmt, frames, outPath, loop, delayMs, fps, gif, webp](CancellationToken tok, Task::ProgressFn prog, Task::LogFn log){
        // Free space check: warn if less than 100 MB free at destination
        const QString destDir = QFileInfo(outPath).absolutePath();
        quint64 avail = 0;
        if (SpaceCheck::hasAtLeast(destDir, 100ull * 1024ull * 1024ull, &avail)) {
            log(QString("Free space: %1 MB").arg(static_cast<qulonglong>(avail / (1024ull*1024ull))));
        } else {
            log(QString("Warning: low free space (~%1 MB) at destination").arg(static_cast<qulonglong>(avail / (1024ull*1024ull))));
        }

        if (frames.isEmpty()) throw std::runtime_error("No frames selected");
        log(QString("Assembling %1 frames → %2").arg(frames.size()).arg(outPath));
        MagickBackend backend;
        if (targetFmt == "gif") {
            backend.assembleGif(frames, outPath, loop, delayMs, fps,
                                gif.optimize, gif.dither, gif.colors,
                                [prog](int d,int t){ prog(d,t); });
        } else {
            \1
// Persist last assemble settings after queueing (so even if it fails, user's intent is saved)
{
    AppSettings::Assemble a;
    a.targetFormat = targetFmt_->currentText();
    a.loop = loopSpin_->value();
    if (delayMsSpin_->value() > 0) a.delayMs = delayMsSpin_->value();
    if (fpsSpin_->value() > 0) a.fps = fpsSpin_->value();
    a.gif.optimize = gifOptimize_->isChecked();
    a.gif.dither = gifDither_->isChecked();
    a.gif.colors = gifColors_->value();
    a.webp.lossless = webpLossless_->isChecked();
    a.webp.quality = webpQuality_->value();
    a.webp.method = webpMethod_->value();
    a.webp.nearLossless = webpNearLossless_->value();
    a.webp.alphaQuality = webpAlphaQuality_->value();
    AppSettings::instance().setLastAssemble(a);
    QString err; AppSettings::instance().save(&err);
}
