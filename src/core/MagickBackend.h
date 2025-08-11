#pragma once
#include <QString>
#include <QStringList>
#include <QProcess>
#include <functional>
#include <optional>
#include <vector>
#include "core/MagickConfigFix.hpp"

// Animagic - MagickBackend with CLI fallback (magick/convert/identify)

struct AnimationProbe {
    int width = 0;
    int height = 0;
    int frames = 0;
    int loop = 0; // 0 = infinite
    QString format; // "gif" or "webp" etc.
    std::vector<int> perFrameDelayMs; // per frame in ms
    std::vector<Magick::DisposeType> disposal;
};

class MagickBackend {
public:
    enum class Mode { Auto, MagickPP, CLI };

    explicit MagickBackend(Mode mode = Mode::Auto); // Initializes Magick++

    // Analyze an animated image and return metadata.
    AnimationProbe probeAnimation(const QString& path);

    // Disassemble an animation into still frames.
    void extractFrames(const QString& path,
                       const QString& outFolder,
                       const QString& pattern,
                       const QString& stillFormat,
                       bool coalesce,
                       std::function<void(int,int)> progress = {});

    // Assemble frames into an animated GIF.
    void assembleGif(const QStringList& frames,
                     const QString& outPath,
                     int loop,
                     std::optional<int> delayMs,
                     std::optional<double> fps,
                     bool optimize,
                     bool dither,
                     int colors,
                     std::function<void(int,int)> progress = {});

    // Assemble frames into an animated WebP.
    void assembleWebp(const QStringList& frames,
                      const QString& outPath,
                      int loop,
                      std::optional<int> delayMs,
                      std::optional<double> fps,
                      bool lossless,
                      int quality,
                      int method,
                      int nearLossless,
                      int alphaQuality,
                      std::function<void(int,int)> progress = {});

private:
    // helpers
    static int msToCentiseconds(int ms);
    static QString extLower(const QString& path);
    static QStringList frameGlobsFrom(const QStringList& frames);
    static QString quote(const QString& s);

    // CLI helpers
    bool ensureCli();
    int runCli(const QStringList& args, QString* stdoutStr = nullptr, QString* stderrStr = nullptr) const;

    // Implementations
    AnimationProbe probeAnimationPP(const QString& path);
    AnimationProbe probeAnimationCLI(const QString& path);

    void extractFramesPP(const QString& path, const QString& outFolder,
                         const QString& pattern, const QString& stillFormat, bool coalesce,
                         std::function<void(int,int)> progress);
    void extractFramesCLI(const QString& path, const QString& outFolder,
                          const QString& pattern, const QString& stillFormat, bool coalesce);

    void assembleGifPP(const QStringList& frames, const QString& outPath, int loop,
                       std::optional<int> delayMs, std::optional<double> fps,
                       bool optimize, bool dither, int colors,
                       std::function<void(int,int)> progress);
    void assembleGifCLI(const QStringList& frames, const QString& outPath, int loop,
                        std::optional<int> delayMs, std::optional<double> fps,
                        bool optimize, bool dither, int colors);

    void assembleWebpPP(const QStringList& frames, const QString& outPath, int loop,
                        std::optional<int> delayMs, std::optional<double> fps,
                        bool lossless, int quality, int method, int nearLossless, int alphaQuality,
                        std::function<void(int,int)> progress);
    void assembleWebpCLI(const QStringList& frames, const QString& outPath, int loop,
                         std::optional<int> delayMs, std::optional<double> fps,
                         bool lossless, int quality, int method, int nearLossless, int alphaQuality);

private:
    Mode mode_;
    QString cliCmd_; // "magick" preferred, otherwise "convert"/"identify" handled via cliCmd_ switch.
};
