#pragma once
#include <QObject>
#include <QStringList>
#include "core/MagickBackend.hpp"

class AssembleWorker : public QObject {
    Q_OBJECT
public:
    bool toWebp=false;
    QStringList frames;
    QString outPath;
    int loop=0;
    int delay=100;
    double fps=0.0; // if >0, overrides delay
    bool optimize=true;
    bool lossless=false; // webp
    int quality=90;      // webp
    int method=4;        // webp (0-6)
    bool dither=true;    // gif
    int colors=256;      // gif (2-256)
    int width=0;         // resize, 0=keep
    int height=0;
signals:
    void progress(int done, int total);
    void done(bool ok, const QString& message);
public slots:
    void start();
};
