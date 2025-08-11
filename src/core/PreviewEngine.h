#pragma once
#include <QStringList>
#include <QImage>
#include <QSize>
#include <QVector>

class PreviewEngine {
private:
    int loopsRemaining_ = -1;
    int loopsTotal_ = 0;
public:
    bool loadFrames(const QStringList& paths, const QSize& targetSize = QSize(), QString* errorOut = nullptr);
    void setDelays(const QVector<int>& perFrameMs);
    void setLoopCount(int loops);
    void reset();
    bool advance(int deltaMs);
    bool nextFrame();

    void setIndex(int idx);
    void rescale(const QSize& targetSize);
    bool replaceFrame(int idx, const QImage& img);

    int currentIndex() const { return index_; }
    int frameCount() const { return frames_.size(); }
    bool finished() const { return finished_; }
    bool isLoaded() const { return !frames_.isEmpty(); }
    QImage currentImage() const { return (index_ >= 0 && index_ < frames_.size()) ? frames_[index_] : QImage(); }

private:
    void rebuildScaledCache();
    void ensureDelays();

private:
    QVector<QImage> frames_;
    QVector<QImage> originals_;
    QVector<int> delaysMs_;
    QVector<int> cumMs_;
    QSize targetSize_;
    int index_ = 0;
    int loopCount_ = 0;
    int loopsDone_ = 0;
    int elapsedInFrame_ = 0;
    bool finished_ = false;
};
