#include "PreviewEngine.h"
#include <QImageReader>
#include <QtMath>
#include <algorithm>

bool PreviewEngine::loadFrames(const QStringList& paths, const QSize& targetSize, QString* errorOut) {
    frames_.clear();
    originals_.clear();
    delaysMs_.clear();
    cumMs_.clear();
    index_ = 0;
    elapsedInFrame_ = 0;
    finished_ = false;

    if (paths.isEmpty()) {
        if (errorOut) *errorOut = "No frame paths provided.";
        return false;
    }

    targetSize_ = targetSize;

    // Load originals via QImageReader for robustness
    originals_.reserve(paths.size());
    for (const auto& p : paths) {
        QImageReader r(p);
        QImage img = r.read();
        if (img.isNull()) {
            if (errorOut) *errorOut = QString("Failed to read frame: %1").arg(p);
            frames_.clear(); originals_.clear();
            return false;
        }
        originals_.push_back(img.convertToFormat(QImage::Format_RGBA8888));
    }

    rebuildScaledCache();
    ensureDelays();
    return true;
}

void PreviewEngine::rebuildScaledCache() {
    frames_.clear();
    frames_.reserve(originals_.size());
    if (!targetSize_.isValid() || targetSize_.isEmpty()) {
        for (const auto& img : originals_) frames_.push_back(img);
        return;
    }
    for (const auto& img : originals_) {
        QImage scaled = img.scaled(targetSize_, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        frames_.push_back(scaled);
    }
}

void PreviewEngine::ensureDelays() {
    const int n = frames_.size();
    if (delaysMs_.size() != n) {
        delaysMs_.assign(n, 100);
    } else {
        for (int& d : delaysMs_) { if (d <= 0) d = 100; }
    }
    cumMs_.assign(n, 0);
}

void PreviewEngine::setDelays(const QVector<int>& delaysMs) {
    delaysMs_ = delaysMs;
    ensureDelays();
}

void PreviewEngine::setLoopCount(int loops) {
    if (loops <= 0) {
        loopsRemaining_ = -1; // infinite
        loopsTotal_ = 0;
    } else {
        loopsRemaining_ = loops;
        loopsTotal_ = loops;
    }
    finished_ = false;
}

void PreviewEngine::reset() {
    index_ = 0;
    elapsedInFrame_ = 0;
    finished_ = false;
    // Reset loop counter (if finite)
    if (loopsTotal_ > 0) loopsRemaining_ = loopsTotal_;
}

bool PreviewEngine::advance(int deltaMs) {
    if (!isLoaded() || finished_) return false;
    if (deltaMs <= 0) return false;

    elapsedInFrame_ += deltaMs;
    bool changed = false;
    while (elapsedInFrame_ >= delaysMs_[index_]) {
        elapsedInFrame_ -= delaysMs_[index_];
        nextFrame();
        changed = true;
        if (finished_) break;
    }
    return changed;
}

bool PreviewEngine::nextFrame() {
    if (index_ + 1 < frames_.size()) { ++index_; return true; }
    if (loopsRemaining_ < 0) { index_ = 0; return true; }
    if (loopsRemaining_ > 1) { --loopsRemaining_; index_ = 0; return true; }
    finished_ = true; return false;
}

    // end of sequence: handle looping
    if (loopsRemaining_ < 0) {
        index_ = 0; // infinite loop
        return;
    }
    if (loopsRemaining_ > 1) {
        --loopsRemaining_;
        index_ = 0;
        return;
    }
    // loopsRemaining_ == 1 or 0: finish
    index_ = frames_.size() - 1;
    finished_ = true;
}

void PreviewEngine::setIndex(int idx) {
    if (!isLoaded()) return;
    if (idx < 0) idx = 0;
    if (idx >= frames_.size()) idx = frames_.size() - 1;
    index_ = idx;
    elapsedInFrame_ = 0;
}

void PreviewEngine::rescale(const QSize& targetSize) {
    targetSize_ = targetSize;
    if (!originals_.isEmpty()) {
        rebuildScaledCache();
    } else {
        // fallback: rescale current cache
        QVector<QImage> newFrames; newFrames.reserve(frames_.size());
        for (const auto& img : frames_) {
            newFrames.push_back(img.scaled(targetSize_, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        frames_.swap(newFrames);
    }
}

bool PreviewEngine::replaceFrame(int idx, const QImage& img) {
    if (idx < 0 || idx >= frames_.size()) return false;
    QImage src = img.convertToFormat(QImage::Format_RGBA8888);
    if (originals_.size() == frames_.size()) originals_[idx] = src;
    if (targetSize_.isValid() && !targetSize_.isEmpty()) {
        frames_[idx] = src.scaled(targetSize_, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        frames_[idx] = src;
    }
    return true;
}
