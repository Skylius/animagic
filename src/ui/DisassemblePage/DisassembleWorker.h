#pragma once
#include <QObject>
#include <QString>
#include "core/MagickBackend.hpp"

class DisassembleWorker : public QObject {
    Q_OBJECT
public:
    QString input;
    QString outDir;
signals:
    void progress(int done, int total);
    void done(bool ok, const QString& msg);
public slots:
    void start();
};
