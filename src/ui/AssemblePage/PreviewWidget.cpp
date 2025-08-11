#include "PreviewWidget.hpp"
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QPixmap>

PreviewWidget::PreviewWidget(QWidget* parent) : QWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    label_ = new QLabel(this);
    label_->setAlignment(Qt::AlignCenter);
    lay->addWidget(label_);

    timer_.setInterval(16); // ~60Hz
    connect(&timer_, &QTimer::timeout, this, &PreviewWidget::onTick);
}

bool PreviewWidget::loadFrames(const QStringList& paths,
                               const QVector<int>& delaysMs,
                               int loopCount,
                               QString* errorOut) {
    if (!engine_.loadFrames(paths, size(), errorOut)) return false;
    if (!delaysMs.isEmpty()) engine_.setDelays(delaysMs);
    engine_.setLoopCount(loopCount);
    engine_.reset();
    render();
    return true;
}

void PreviewWidget::play() {
    if (!engine_.isLoaded()) return;
    if (playing_) return;
    playing_ = true;
    elapsed_.restart();
    timer_.start();
}

void PreviewWidget::pause() {
    if (!playing_) return;
    playing_ = false;
    timer_.stop();
}

void PreviewWidget::stop() {
    pause();
    engine_.reset();
    render();
}

void PreviewWidget::setSpeed(double s) {
    if (s <= 0.0) s = 1.0;
    speed_ = s;
}

void PreviewWidget::setIndex(int idx) {
    engine_.setIndex(idx);
    render();
}

void PreviewWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    engine_.rescale(e->size());
    render();
}

void PreviewWidget::onTick() {
    if (!playing_) return;
    int delta = static_cast<int>(elapsed_.restart());
    if (delta < 0) delta = timer_.interval();
    // apply scalar
    int scaled = static_cast<int>(delta * speed_);
    bool changed = engine_.advance(scaled);
    if (changed) {
        emit frameChanged(engine_.currentIndex());
        render();
    }
    if (engine_.finished()) {
        pause();
    }
}

void PreviewWidget::render() {
    if (!engine_.isLoaded()) {
        label_->setPixmap(QPixmap());
        return;
    }
    QImage img = engine_.currentImage();
    label_->setPixmap(QPixmap::fromImage(img));
}
