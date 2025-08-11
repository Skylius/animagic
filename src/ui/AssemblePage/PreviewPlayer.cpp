#include "PreviewPlayer.hpp"
PreviewPlayer::PreviewPlayer(QObject* parent) : QObject(parent) {
    timer_.setInterval(16);
    connect(&timer_, &QTimer::timeout, this, &PreviewPlayer::onTick);
}
bool PreviewPlayer::loadFrames(const QStringList& paths, const QVector<int>& delaysMs, int loopCount, QString* errorOut) {
    if (!engine_.loadFrames(paths, QSize(), errorOut)) return false;
    if (!delaysMs.isEmpty()) engine_.setDelays(delaysMs);
    engine_.setLoopCount(loopCount);
    engine_.reset();
    return true;
}
void PreviewPlayer::setSpeed(double s) { if (s <= 0.0) s = 1.0; speed_ = s; }
void PreviewPlayer::play() { if (playing_) return; playing_ = true; elapsed_.restart(); timer_.start(); }
void PreviewPlayer::pause() { if (!playing_) return; playing_ = false; timer_.stop(); }
void PreviewPlayer::stop() { pause(); engine_.reset(); }
void PreviewPlayer::onTick() {
    int delta = static_cast<int>(elapsed_.restart());
    if (delta < 0) delta = timer_.interval();
    int scaled = static_cast<int>(delta * speed_);
    bool changed = engine_.advance(scaled);
    if (changed) emit frameChanged(engine_.currentIndex(), engine_.currentImage());
    if (engine_.finished()) { pause(); emit finished(); }
}
