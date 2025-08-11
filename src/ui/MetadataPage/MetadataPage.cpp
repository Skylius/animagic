#include <QPushButton>
#include "MetadataPage.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "ui/Common/FilePickers.hpp"
MetadataPage::MetadataPage(QWidget* parent) : QWidget(parent) { buildUi(); }
void MetadataPage::buildUi() {
    auto* lay = new QVBoxLayout(this);
    auto* srcBox = new QGroupBox("Source (read-only info)", this);
    auto* srcForm = new QFormLayout(srcBox);
    srcPath_ = new QLineEdit(this); srcPath_->setReadOnly(true);
    srcFormat_ = new QLineEdit(this); srcFormat_->setReadOnly(true);
    srcWidth_ = new QSpinBox(this); srcWidth_->setRange(0, 100000); srcWidth_->setReadOnly(true); srcWidth_->setButtonSymbols(QAbstractSpinBox::NoButtons);
    srcHeight_ = new QSpinBox(this); srcHeight_->setRange(0, 100000); srcHeight_->setReadOnly(true); srcHeight_->setButtonSymbols(QAbstractSpinBox::NoButtons);
    srcFrames_ = new QSpinBox(this); srcFrames_->setRange(0, 100000); srcFrames_->setReadOnly(true); srcFrames_->setButtonSymbols(QAbstractSpinBox::NoButtons);
    srcLoop_ = new QSpinBox(this); srcLoop_->setRange(0, 1000); srcLoop_->setReadOnly(true); srcLoop_->setButtonSymbols(QAbstractSpinBox::NoButtons);
    srcDelays_ = new QPlainTextEdit(this); srcDelays_->setReadOnly(true); srcDelays_->setMaximumHeight(60);
    srcForm->addRow("Path:", srcPath_);
    srcForm->addRow("Format:", srcFormat_);
    srcForm->addRow("Width:", srcWidth_);
    srcForm->addRow("Height:", srcHeight_);
    srcForm->addRow("Frames:", srcFrames_);
    srcForm->addRow("Loop:", srcLoop_);
    srcForm->addRow("Per-frame delays (ms):", srcDelays_);
    auto* stillBox = new QGroupBox("Stills", this);
    auto* stillForm = new QFormLayout(stillBox);
    stillFolder_ = new QLineEdit(this);
    auto* btnStillFolder = new QPushButton("Browse…", this);
    auto* stillFolderRow = new QWidget(this);
    auto* stillFolderLay = new QHBoxLayout(stillFolderRow); stillFolderLay->setContentsMargins(0,0,0,0);
    stillFolderLay->addWidget(stillFolder_, 1); stillFolderLay->addWidget(btnStillFolder, 0);
    stillPattern_ = new QLineEdit("frame_%04d.png", this);
    stillFormat_ = new QComboBox(this); stillFormat_->addItems({"png","jpg","jpeg","tif","tiff","bmp","webp"});
    stillForm->addRow("Folder:", stillFolderRow);
    stillForm->addRow("Pattern:", stillPattern_);
    stillForm->addRow("Format:", stillFormat_);
    auto* asmBox = new QGroupBox("Assembly Preferences", this);
    auto* asmForm = new QFormLayout(asmBox);
    targetFmt_ = new QComboBox(this); targetFmt_->addItems({"gif","webp"});
    loop_ = new QSpinBox(this); loop_->setRange(0, 1000); loop_->setValue(0);
    delayMs_ = new QSpinBox(this); delayMs_->setRange(0, 60000); delayMs_->setValue(100);
    fps_ = new QDoubleSpinBox(this); fps_->setRange(0.0, 120.0); fps_->setDecimals(2);
    gifOptimize_ = new QCheckBox("GIF Optimize", this); gifOptimize_->setChecked(true);
    gifDither_ = new QCheckBox("GIF Dither", this); gifDither_->setChecked(true);
    gifColors_ = new QSpinBox(this); gifColors_->setRange(2, 256); gifColors_->setValue(256);
    webpLossless_ = new QCheckBox("WebP Lossless", this);
    webpQuality_ = new QSpinBox(this); webpQuality_->setRange(0, 100); webpQuality_->setValue(85);
    webpMethod_ = new QSpinBox(this); webpMethod_->setRange(0, 6); webpMethod_->setValue(4);
    webpNearLossless_ = new QSpinBox(this); webpNearLossless_->setRange(0, 100); webpNearLossless_->setValue(0);
    webpAlphaQuality_ = new QSpinBox(this); webpAlphaQuality_->setRange(0, 100); webpAlphaQuality_->setValue(100);
    asmForm->addRow("Target format:", targetFmt_);
    asmForm->addRow("Loop count:", loop_);
    asmForm->addRow("Global delay (ms):", delayMs_);
    asmForm->addRow("FPS (alt):", fps_);
    asmForm->addRow(gifOptimize_);
    asmForm->addRow(gifDither_);
    asmForm->addRow("GIF colors:", gifColors_);
    asmForm->addRow(webpLossless_);
    asmForm->addRow("WebP quality:", webpQuality_);
    asmForm->addRow("WebP method:", webpMethod_);
    asmForm->addRow("WebP near-lossless:", webpNearLossless_);
    asmForm->addRow("WebP alpha-quality:", webpAlphaQuality_);
    auto* btnRow = new QHBoxLayout();
    auto* btnLoad = new QPushButton("Load JSON…", this);
    auto* btnSave = new QPushButton("Save JSON As…", this);
    btnRow->addWidget(btnLoad); btnRow->addWidget(btnSave); btnRow->addStretch(1);
    logEdit_ = new QPlainTextEdit(this); logEdit_->setReadOnly(true); logEdit_->setMinimumHeight(100);
    lay->addWidget(srcBox); lay->addWidget(stillBox); lay->addWidget(asmBox);
    lay->addLayout(btnRow); lay->addWidget(logEdit_, 1);
    connect(btnLoad, &QPushButton::clicked, this, &MetadataPage::onLoad);
    connect(btnSave, &QPushButton::clicked, this, &MetadataPage::onSaveAs);
    connect(btnStillFolder, &QPushButton::clicked, this, [this](){
        QString d = FilePickers::getOpenFolder(this, "Choose Stills Folder");
        if (!d.isEmpty()) stillFolder_->setText(d);
    });
}
void MetadataPage::log(const QString& s) { if (progressLog_) progressLog_->log(s); }
void MetadataPage::onApplyToForm(const ProjectMetadata& md) {
    srcPath_->setText(md.source.path);
    srcFormat_->setText(md.source.format);
    srcWidth_->setValue(md.source.width);
    srcHeight_->setValue(md.source.height);
    srcFrames_->setValue(md.source.frames);
    srcLoop_->setValue(md.source.loop);
    QString delays; for (size_t i=0;i<md.source.perFrameDelayMs.size();++i) { delays += QString::number(md.source.perFrameDelayMs[i]); if (i+1<md.source.perFrameDelayMs.size()) delays += ","; }
    srcDelays_->setPlainText(delays);
    stillFolder_->setText(md.stills.folder);
    stillPattern_->setText(md.stills.pattern);
    int sidx = stillFormat_->findText(md.stills.format); if (sidx >= 0) stillFormat_->setCurrentIndex(sidx);
    int tidx = targetFmt_->findText(md.assembly.targetFormat); if (tidx >= 0) targetFmt_->setCurrentIndex(tidx);
    loop_->setValue(md.assembly.loop);
    if (md.assembly.delayMs.has_value()) delayMs_->setValue(*md.assembly.delayMs);
    if (md.assembly.fps.has_value()) fps_->setValue(*md.assembly.fps);
    gifOptimize_->setChecked(md.assembly.gif.optimize);
    gifDither_->setChecked(md.assembly.gif.dither);
    gifColors_->setValue(md.assembly.gif.colors);
    webpLossless_->setChecked(md.assembly.webp.lossless);
    webpQuality_->setValue(md.assembly.webp.quality);
    webpMethod_->setValue(md.assembly.webp.method);
    webpNearLossless_->setValue(md.assembly.webp.nearLossless);
    webpAlphaQuality_->setValue(md.assembly.webp.alphaQuality);
}
void MetadataPage::onCollectFromForm(ProjectMetadata& md) {
    md.stills.folder = stillFolder_->text();
    md.stills.pattern = stillPattern_->text();
    md.stills.format = stillFormat_->currentText();
    md.assembly.targetFormat = targetFmt_->currentText();
    md.assembly.loop = loop_->value();
    if (delayMs_->value() > 0) md.assembly.delayMs = delayMs_->value(); else md.assembly.delayMs.reset();
    if (fps_->value() > 0.0) md.assembly.fps = fps_->value(); else md.assembly.fps.reset();
    md.assembly.gif.optimize = gifOptimize_->isChecked();
    md.assembly.gif.dither = gifDither_->isChecked();
    md.assembly.gif.colors = gifColors_->value();
    md.assembly.webp.lossless = webpLossless_->isChecked();
    md.assembly.webp.quality = webpQuality_->value();
    md.assembly.webp.method = webpMethod_->value();
    md.assembly.webp.nearLossless = webpNearLossless_->value();
    md.assembly.webp.alphaQuality = webpAlphaQuality_->value();
}
void MetadataPage::onLoad() {
    QString f = FilePickers::getOpenJson(this);
    if (f.isEmpty()) return;
    QString err;
    auto md = ProjectMetadata::loadFromFile(f, &err);
    if (!md.has_value()) { log(QString("Load failed: %1").arg(err)); return; }
    onApplyToForm(*md);
    emit loaded(f);
    log(QString("Loaded: %1").arg(f));
}
void MetadataPage::onSaveAs() {
    QString f = FilePickers::getSaveJson(this);
    if (f.isEmpty()) return;
    ProjectMetadata md; onCollectFromForm(md);
    QString err;
    if (!ProjectMetadata::saveToFile(f, md, &err)) { log(QString("Save failed: %1").arg(err)); return; }
    emit saved(f);
    log(QString("Saved: %1").arg(f));
}
