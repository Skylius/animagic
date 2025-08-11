#include "DisassembleViewModel.hpp"
#include <QFileInfo>
#include <QDir>
#include <QDebug>

DisassembleViewModel::DisassembleViewModel(QObject* parent) : QObject(parent) {}

void DisassembleViewModel::setSourcePath(const QString& path) { sourcePath_ = path; }
void DisassembleViewModel::setOutFolder(const QString& folder) { outFolder_ = folder; }
void DisassembleViewModel::setPattern(const QString& pattern) { pattern_ = pattern; }
void DisassembleViewModel::setStillFormat(const QString& fmt) { stillFormat_ = fmt; }
void DisassembleViewModel::setCoalesce(bool value) { coalesce_ = value; }

DisassembleViewModel::ProbeResult DisassembleViewModel::analyze() {
    ProbeResult r;
    auto vr = Validators::checkReadableFile(sourcePath_, {"gif","webp"});
    if (!vr.ok) {
        emit logMessage(vr.errors.join('\n'));
        return r;
    }
    try {
        auto p = backend_.probeAnimation(sourcePath_);
        r.ok = true;
        r.format = p.format;
        r.width = p.width;
        r.height = p.height;
        r.frames = p.frames;
        r.loop = p.loop;
        r.perFrameDelayMs = QVector<int>::fromStdVector(p.perFrameDelayMs);
        emit logMessage(QString("Analyzed: %1 × %2, %3 frames, loop=%4, format=%5")
                        .arg(r.width).arg(r.height).arg(r.frames).arg(r.loop).arg(r.format));
    } catch (const std::exception& e) {
        emit logMessage(QString("Analyze failed: %1").arg(e.what()));
    }
    return r;
}

bool DisassembleViewModel::extract(QString* errorOut) {
    Validators::DisassembleParams params;
    params.sourcePath = sourcePath_;
    params.outFolder = outFolder_;
    params.pattern = pattern_;
    params.stillFormat = stillFormat_;
    params.coalesce = coalesce_;
    auto vr = Validators::validate(params);
    if (!vr.ok) {
        if (errorOut) *errorOut = vr.errors.join('\n');
        emit logMessage(*errorOut);
        return false;
    }
    try {
        backend_.extractFrames(sourcePath_, outFolder_, pattern_, stillFormat_, coalesce_,
            [this](int done, int total){ emit progress(done, total); });
        emit logMessage("Extraction complete.");
        return true;
    } catch (const std::exception& e) {
        if (errorOut) *errorOut = e.what();
        emit logMessage(QString("Extraction failed: %1").arg(e.what()));
        return false;
    }
}

bool DisassembleViewModel::writeMetadata(const ProjectMetadata& md, const QString& filePath, QString* errorOut) {
    if (!ProjectMetadata::saveToFile(filePath, md, errorOut)) {
        emit logMessage(QString("Failed to write metadata: %1").arg(errorOut ? *errorOut : QString()));
        return false;
    }
    emit logMessage(QString("Wrote metadata: %1").arg(filePath));
    return true;
}
