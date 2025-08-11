#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <optional>

#include "core/MagickBackend.hpp"
#include "core/Validators.hpp"
#include "core/ProjectMetadata.hpp"

// DisassembleViewModel: Thin mediator between UI and backend for disassembly.
class DisassembleViewModel : public QObject {
    Q_OBJECT
public:
    explicit DisassembleViewModel(QObject* parent = nullptr);

    struct ProbeResult {
        bool ok = false;
        QString format;
        int width = 0;
        int height = 0;
        int frames = 0;
        int loop = 0;
        QVector<int> perFrameDelayMs;
    };

    // Parameters
    void setSourcePath(const QString& path);
    void setOutFolder(const QString& folder);
    void setPattern(const QString& pattern);
    void setStillFormat(const QString& fmt);
    void setCoalesce(bool value);
    QString sourcePath() const { return sourcePath_; }
    QString outFolder() const { return outFolder_; }
    QString pattern() const { return pattern_; }
    QString stillFormat() const { return stillFormat_; }
    bool coalesce() const { return coalesce_; }

    // Actions
    Q_INVOKABLE ProbeResult analyze();
    Q_INVOKABLE bool extract(QString* errorOut = nullptr);
    Q_INVOKABLE bool writeMetadata(const ProjectMetadata& md, const QString& filePath, QString* errorOut = nullptr);

signals:
    void progress(int done, int total);
    void logMessage(const QString& line);

private:
    MagickBackend backend_;
    QString sourcePath_;
    QString outFolder_;
    QString pattern_ = "frame_%04d.png";
    QString stillFormat_ = "png";
    bool coalesce_ = true;
};
