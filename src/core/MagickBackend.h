#pragma once
#include <QString>
#include <QStringList>
#include <optional>
#include <functional>

struct AnimationProbe{
    int width=0;
    int height=0;
    int frames=0;
    int loop=0; // 0=infinite
};

class MagickBackend{
public:
    enum class Mode { MagickPP };
    explicit MagickBackend(Mode = Mode::MagickPP);
    AnimationProbe probeAnimation(const QString& path);

    void extractFrames(const QString& path,
                       const QString& outFolder,
                       const QString& pattern = "frame_%05d.png",
                       const QString& stillFormat = "PNG",
                       bool coalesce=true,
                       std::function<void(int,int)> progress = {});

    // Backward compatible signature
    void extractFrames(const QString& path,
                       const QString& outFolder,
                       const QString& pattern = "frame_%05d.png",
                       const QString& stillFormat = "PNG",
                       bool coalesce=true);

    void assembleGif(const QStringList& frames, const QString& outPath, int loop=0,
                     std::optional<int> delayMs=std::nullopt,
                     std::optional<double> fps=std::nullopt,
                     bool optimize=true, bool dither=true, int colors=256,
                     std::function<void(int,int)> progress = {});

    // Extended: with FPS and resize
    void assembleGif(const QStringList& frames, const QString& outPath, int loop,
                     std::optional<int> delayMs, std::optional<double> fps,
                     bool optimize, bool dither, int colors,
                     int width, int height,
                     std::function<void(int,int)> progress = {});


    // Backward compatible signature
    void assembleGif(const QStringList& frames, const QString& outPath, int loop=0,
                     std::optional<int> delayMs=std::nullopt,
                     std::optional<double> fps=std::nullopt,
                     bool optimize=true, bool dither=true, int colors=256);

    void assembleWebp(const QStringList& frames, const QString& outPath, int loop=0,
                      std::optional<int> delayMs=std::nullopt,
                      std::optional<double> fps=std::nullopt,
                      bool lossless=false, int quality=90, int method=4, int nearLossless=0, int alphaQuality=100,
                      std::function<void(int,int)> progress = {});

    // Extended: with FPS and resize
    void assembleWebp(const QStringList& frames, const QString& outPath, int loop,
                      std::optional<int> delayMs, std::optional<double> fps,
                      bool lossless, int quality, int method, int nearLossless, int alphaQuality,
                      int width, int height,
                      std::function<void(int,int)> progress = {});


    // Backward compatible signature
    void assembleWebp(const QStringList& frames, const QString& outPath, int loop=0,
                      std::optional<int> delayMs=std::nullopt,
                      std::optional<double> fps=std::nullopt,
                      bool lossless=false, int quality=90, int method=4, int nearLossless=0, int alphaQuality=100);
};
