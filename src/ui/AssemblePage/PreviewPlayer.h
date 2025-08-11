#pragma once
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QImage>
#include <QVector>
#include <QStringList>
#include "core/PreviewEngine.h"
class PreviewPlayer : public QObject {
    Q_OBJECT
public:
    explicit PreviewPlayer(QObject* parent = nullptr);
    bool loadFrames(const QStringList& paths, const QVector<int>& delaysMs = {}, int loopCount = 0, QString* errorOut = nullptr);
    void setSpeed(double s);
    void play();
    void pause();
    void stop();
    int currentIndex() const { return engine_.currentIndex(); }
    int frameCount() const { return engine_.frameCount(); }
    bool isPlaying() const { return playing_; }
signals:
    void frameChanged(int index, QImage image);
    void finished();
private slots:
    void onTick();
private:
    PreviewEngine engine_;
    QTimer timer_;
    QElapsedTimer elapsed_;
    bool playing_ = false;
    double speed_ = 1.0;
};
