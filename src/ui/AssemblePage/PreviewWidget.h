#pragma once
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>
#include <optional>
#include "core/PreviewEngine.h"
#include "core/Timing.hpp"

// PreviewWidget: a small, self-contained widget for playing frame previews.
// - Drop this into AssemblePage.
// - Call loadFrames(paths) then play().
// - It scales to fit; on resize it requests the engine to rescale.

class PreviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget* parent = nullptr);

    // Load frames and optional delays; target size is the widget's current size.
    bool loadFrames(const QStringList& paths,
                    const QVector<int>& delaysMs = {},
                    int loopCount = 0, // 0=infinite
                    QString* errorOut = nullptr);

    // Controls
    void play();
    void pause();
    void stop();
    bool isPlaying() const { return playing_; }

    // Speed scalar (0.5, 1.0, 2.0...)
    void setSpeed(double s);

    // Jump and accessors
    void setIndex(int idx);
    int currentIndex() const { return engine_.currentIndex(); }
    int frameCount() const { return engine_.frameCount(); }

signals:
    void frameChanged(int index);

protected:
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void onTick();

private:
    void render();

private:
    QLabel* label_ = nullptr;
    QTimer timer_;
    QElapsedTimer elapsed_;
    double speed_ = 1.0;
    bool playing_ = false;

    PreviewEngine engine_;
};
