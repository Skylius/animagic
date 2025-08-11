#include "MagickBackend.hpp"
#include <list>
#include <stdexcept>
#include <filesystem>
#include <QFileInfo>
#include <QDir>

using std::list;
using std::string;

static inline std::string q2s(const QString& q) { return q.toStdString(); }

MagickBackend::MagickBackend(Mode mode) : mode_(mode) {
    try {
        Magick::InitializeMagick(nullptr);
    } catch (...) {
        // ignore, CLI fallback may be used
    }
}

QString MagickBackend::extLower(const QString& path) {
    auto s = path;
    int dot = s.lastIndexOf('.');
    if (dot >= 0) s = s.mid(dot + 1);
    return s.toLower();
}

int MagickBackend::msToCentiseconds(int ms) {
    if (ms <= 0) return 0;
    int cs = (ms + 5) / 10;
    if (cs < 1) cs = 1;
    return cs;
}

QString MagickBackend::quote(const QString& s) {
    QString t = s;
    t.replace('"', "\\\"");
    return "\"" + t + "\"";
}

QStringList MagickBackend::frameGlobsFrom(const QStringList& frames) {
    // Attempt to condense sequential frames into a glob if possible; otherwise return full list
    if (frames.isEmpty()) return frames;
    QFileInfo first(frames.first());
    QDir dir = first.dir();
    QString base = first.fileName();
    // Simple case: if all frames share same dir and extension and a common prefix
    QString ext = first.suffix();
    bool same = true;
    for (const auto& f : frames) {
        QFileInfo fi(f);
        if (fi.dir() != dir || fi.suffix() != ext) { same = false; break; }
    }
    if (!same) return frames;
    // Fallback to explicit list; CLI can accept multiple inputs
    return frames;
}

// -------------------- Public API dispatch --------------------
AnimationProbe MagickBackend::probeAnimation(const QString& path) {
    if (mode_ == Mode::MagickPP) return probeAnimationPP(path);
    if (mode_ == Mode::CLI) return probeAnimationCLI(path);
    // Auto: try PP first, then CLI
    try { return probeAnimationPP(path); }
    catch (...) { /* fall through */ }
    return probeAnimationCLI(path);
}

void MagickBackend::extractFrames(const QString& path,
                                  const QString& outFolder,
                                  const QString& pattern,
                                  const QString& stillFormat,
                                  bool coalesce,
                                  std::function<void(int,int)> progress) {
    if (mode_ == Mode::MagickPP) return extractFramesPP(path, outFolder, pattern, stillFormat, coalesce, progress);
    if (mode_ == Mode::CLI) return extractFramesCLI(path, outFolder, pattern, stillFormat, coalesce);
    try { return extractFramesPP(path, outFolder, pattern, stillFormat, coalesce, progress); }
    catch (...) { return extractFramesCLI(path, outFolder, pattern, stillFormat, coalesce); }
}

void MagickBackend::assembleGif(const QStringList& frames,
                                const QString& outPath,
                                int loop,
                                std::optional<int> delayMs,
                                std::optional<double> fps,
                                bool optimize,
                                bool dither,
                                int colors,
                                std::function<void(int,int)> progress) {
    if (mode_ == Mode::MagickPP) return assembleGifPP(frames, outPath, loop, delayMs, fps, optimize, dither, colors, progress);
    if (mode_ == Mode::CLI) return assembleGifCLI(frames, outPath, loop, delayMs, fps, optimize, dither, colors);
    try { return assembleGifPP(frames, outPath, loop, delayMs, fps, optimize, dither, colors, progress); }
    catch (...) { return assembleGifCLI(frames, outPath, loop, delayMs, fps, optimize, dither, colors); }
}

void MagickBackend::assembleWebp(const QStringList& frames,
                                 const QString& outPath,
                                 int loop,
                                 std::optional<int> delayMs,
                                 std::optional<double> fps,
                                 bool lossless,
                                 int quality,
                                 int method,
                                 int nearLossless,
                                 int alphaQuality,
                                 std::function<void(int,int)> progress) {
    if (mode_ == Mode::MagickPP) return assembleWebpPP(frames, outPath, loop, delayMs, fps, lossless, quality, method, nearLossless, alphaQuality, progress);
    if (mode_ == Mode::CLI) return assembleWebpCLI(frames, outPath, loop, delayMs, fps, lossless, quality, method, nearLossless, alphaQuality);
    try { return assembleWebpPP(frames, outPath, loop, delayMs, fps, lossless, quality, method, nearLossless, alphaQuality, progress); }
    catch (...) { return assembleWebpCLI(frames, outPath, loop, delayMs, fps, lossless, quality, method, nearLossless, alphaQuality); }
}

// -------------------- Magick++ implementations --------------------
AnimationProbe MagickBackend::probeAnimationPP(const QString& path) {
    list<Magick::Image> frames;
    readImages(&frames, q2s(path));

    AnimationProbe p;
    if (frames.empty()) return p;

    auto& first = frames.front();
    p.width  = static_cast<int>(first.columns());
    p.height = static_cast<int>(first.rows());
    p.frames = static_cast<int>(frames.size());
    p.format = QString::fromStdString(first.magick()).toLower();

    try { p.loop = first.animationIterations(); } catch (...) { p.loop = 0; }
    try {
        auto loopAttr = first.attribute("Loop");
        if (!loopAttr.empty()) p.loop = std::stoi(loopAttr);
        auto webpLoop = first.attribute("webp:loop");
        if (!webpLoop.empty()) p.loop = std::stoi(webpLoop);
    } catch (...) {}

    p.perFrameDelayMs.reserve(frames.size());
    p.disposal.reserve(frames.size());
    for (auto& f : frames) {
        int cs = 0;
        try { cs = f.animationDelay(); } catch (...) { cs = 0; }
        p.perFrameDelayMs.push_back(cs * 10);
        p.disposal.push_back(static_cast<Magick::DisposeType>(f.gifDisposeMethod()));
    }
    return p;
}

void MagickBackend::extractFramesPP(const QString& path,
                                    const QString& outFolder,
                                    const QString& pattern,
                                    const QString& stillFormatIn,
                                    bool coalesce,
                                    std::function<void(int,int)> progress) {
    list<Magick::Image> inFrames;
    readImages(&inFrames, q2s(path));
    if (inFrames.empty()) throw std::runtime_error("No frames read from source.");

    list<Magick::Image> frames;
    if (coalesce) {
        list<Magick::Image> tmp;
        coalesceImages(&tmp, inFrames.begin(), inFrames.end());
        frames.swap(tmp);
    } else {
        frames = inFrames;
    }

    QString stillFormat = stillFormatIn;
    if (stillFormat.isEmpty()) {
        stillFormat = extLower(pattern);
        if (stillFormat.isEmpty()) stillFormat = "png";
    }

    for (auto& f : frames) {
        f.magick(q2s(stillFormat.toUpper()));
    }

    std::filesystem::create_directories(q2s(outFolder));
    std::filesystem::path base = std::filesystem::path(q2s(outFolder)) / q2s(pattern);
    writeImages(frames.begin(), frames.end(), base.string());

    if (progress) progress(static_cast<int>(frames.size()), static_cast<int>(frames.size()));
}

void MagickBackend::assembleGifPP(const QStringList& framePaths,
                                  const QString& outPath,
                                  int loop,
                                  std::optional<int> delayMs,
                                  std::optional<double> fps,
                                  bool optimize,
                                  bool dither,
                                  int colors,
                                  std::function<void(int,int)> progress) {
    if (framePaths.isEmpty()) throw std::runtime_error("No frames provided.");
    list<Magick::Image> imgs;
    imgs.resize(framePaths.size());
    int idx = 0;
    for (auto it = imgs.begin(); it != imgs.end(); ++it, ++idx) {
        it->read(q2s(framePaths[idx]));
        it->magick("GIF");
        if (delayMs && *delayMs > 0) it->animationDelay(msToCentiseconds(*delayMs));
        if (dither) it->quantizeDither(true);
        if (progress) progress(idx + 1, framePaths.size());
    }
    if (fps && !delayMs) {
        int cs = msToCentiseconds(static_cast<int>(1000.0 / *fps));
        for (auto& i : imgs) i.animationDelay(cs);
    }
    try { imgs.front().animationIterations(loop); } catch (...) {}
    try { imgs.front().attribute("Loop", std::to_string(loop)); } catch (...) {}
    if (colors > 0) {
        for (auto& i : imgs) { i.quantizeColors(colors); i.quantize(); }
    }
    if (optimize) { std::list<Magick::Image> optimized; optimizeImageLayers(&optimized, imgs.begin(), imgs.end()); imgs.swap(optimized); }
    writeImages(imgs.begin(), imgs.end(), q2s(outPath));
}

void MagickBackend::assembleWebpPP(const QStringList& framePaths,
                                   const QString& outPath,
                                   int loop,
                                   std::optional<int> delayMs,
                                   std::optional<double> fps,
                                   bool lossless,
                                   int quality,
                                   int method,
                                   int nearLossless,
                                   int alphaQuality,
                                   std::function<void(int,int)> progress) {
    if (framePaths.isEmpty()) throw std::runtime_error("No frames provided.");
    list<Magick::Image> imgs;
    imgs.resize(framePaths.size());
    int idx = 0;
    for (auto it = imgs.begin(); it != imgs.end(); ++it, ++idx) {
        it->read(q2s(framePaths[idx]));
        it->magick("WEBP");
        if (delayMs && *delayMs > 0) it->animationDelay(msToCentiseconds(*delayMs));
        it->defineValue("webp", "lossless", lossless ? "true" : "false");
        if (!lossless) {
            if (quality < 0) quality = 75;
            if (quality > 100) quality = 100;
            it->quality(quality);
            it->defineValue("webp", "near-lossless", std::to_string(std::max(0, std::min(nearLossless, 100))));
        }
        it->defineValue("webp", "method", std::to_string(std::max(0, std::min(method, 6))));
        it->defineValue("webp", "alpha-quality", std::to_string(std::max(0, std::min(alphaQuality, 100))));
        if (progress) progress(idx + 1, framePaths.size());
    }
    if (fps && !delayMs) {
        int cs = msToCentiseconds(static_cast<int>(1000.0 / *fps));
        for (auto& i : imgs) i.animationDelay(cs);
    }
    try { imgs.front().animationIterations(loop); } catch (...) {}
    try { imgs.front().attribute("webp:loop", std::to_string(loop)); } catch (...) {}
    writeImages(imgs.begin(), imgs.end(), q2s(outPath));
}

// -------------------- CLI implementations --------------------
bool MagickBackend::ensureCli() {
    if (!cliCmd_.isEmpty()) return true;
    // Prefer "magick" (IM7+); fall back to legacy "convert"/"identify" pairs.
    // We'll store "magick" and build commands like: magick [subcommands/args]
    auto which = [&](const char* cmd)->bool{
        QProcess p; p.start("bash", {"-lc", QString("command -v %1").arg(cmd)});
        p.waitForFinished();
        return p.exitCode() == 0 && !QString(p.readAllStandardOutput()).trimmed().isEmpty();
    };
    if (which("magick")) { cliCmd_ = "magick"; return true; }
    // If no "magick", try legacy "convert" for ops and "identify" for probing.
    if (which("convert") && which("identify")) { cliCmd_ = "legacy"; return true; }
    return false;
}

int MagickBackend::runCli(const QStringList& args, QString* stdoutStr, QString* stderrStr) const {
    QProcess p;
    QString prog;
    QStringList finalArgs;
    if (cliCmd_ == "magick") {
        prog = "magick";
        finalArgs = args;
    } else if (cliCmd_ == "legacy") {
        // For legacy, args[0] should be either "convert" or "identify" to signal tool
        if (args.isEmpty()) throw std::runtime_error("Empty CLI args.");
        prog = args.first();
        finalArgs = args.mid(1);
    } else {
        throw std::runtime_error("CLI not available.");
    }
    p.start(prog, finalArgs);
    p.waitForFinished(-1);
    if (stdoutStr) *stdoutStr = QString::fromUtf8(p.readAllStandardOutput());
    if (stderrStr) *stderrStr = QString::fromUtf8(p.readAllStandardError());
    return p.exitCode();
}

AnimationProbe MagickBackend::probeAnimationCLI(const QString& path) {
    if (!ensureCli()) throw std::runtime_error("ImageMagick CLI not found.");

    QString out;
    QString err;
    QStringList args;
    if (cliCmd_ == "magick") {
        // identify-like output per frame
        args << "identify" << "-format" << "%m %w %h %[scene] %[delay] %[dispose] %[gamma]\\n" << path;
    } else {
        args << "identify" << "-format" << "%m %w %h %[scene] %[delay] %[dispose] %[gamma]\\n" << path;
    }
    if (runCli(args, &out, &err) != 0) {
        throw std::runtime_error(("identify failed: " + err).toStdString());
    }

    AnimationProbe p;
    QStringList lines = out.split('\n', Qt::SkipEmptyParts);
    p.frames = lines.size();
    for (int i = 0; i < lines.size(); ++i) {
        const auto parts = lines[i].split(' ', Qt::SkipEmptyParts);
        if (parts.size() >= 6) {
            if (i == 0) {
                p.format = parts[0].toLower();
                p.width = parts[1].toInt();
                p.height = parts[2].toInt();
            }
            int delay_cs = parts[4].toInt(); // delay reported in centiseconds
            p.perFrameDelayMs.push_back(delay_cs * 10);
            // dispose parsing is best-effort; default to None
            p.disposal.push_back(Magick::UndefinedDispose);
        }
    }

    // Loop count
    QString loopOut;
    QStringList loopArgs;
    if (cliCmd_ == "magick") {
        loopArgs << "identify" << "-format" << "%[gif:iterations]%[webp:loop]" << path;
    } else {
        loopArgs << "identify" << "-format" << "%[gif:iterations]%[webp:loop]" << path;
    }
    if (runCli(loopArgs, &loopOut, &err) == 0) {
        bool ok=false; int v = loopOut.trimmed().toInt(&ok);
        if (ok) p.loop = v;
    }
    return p;
}

void MagickBackend::extractFramesCLI(const QString& path, const QString& outFolder,
                                     const QString& pattern, const QString& stillFormat,
                                     bool coalesce) {
    if (!ensureCli()) throw std::runtime_error("ImageMagick CLI not found.");
    QDir().mkpath(outFolder);

    QString fmt = stillFormat.isEmpty() ? extLower(pattern) : stillFormat;
    if (fmt.isEmpty()) fmt = "png";

    QString outPattern = QDir(outFolder).filePath(pattern);
    QStringList args;
    if (cliCmd_ == "magick") {
        args << path;
        if (coalesce) args << "-coalesce";
        args << outPattern;
        args.push_front("convert"); // "magick convert input ... outPattern"
    } else {
        args << "convert" << path;
        if (coalesce) args << "-coalesce";
        args << outPattern;
    }
    QString out, err;
    if (runCli(args, &out, &err) != 0) {
        throw std::runtime_error(("extract (convert) failed: " + err).toStdString());
    }
}

void MagickBackend::assembleGifCLI(const QStringList& frames, const QString& outPath, int loop,
                                   std::optional<int> delayMs, std::optional<double> fps,
                                   bool optimize, bool dither, int colors) {
    if (!ensureCli()) throw std::runtime_error("ImageMagick CLI not found.");
    if (frames.isEmpty()) throw std::runtime_error("No frames provided.");
    QStringList args;
    int cs = 0;
    if (delayMs && *delayMs > 0) cs = msToCentiseconds(*delayMs);
    else if (fps && *fps > 0.0) cs = msToCentiseconds(static_cast<int>(1000.0 / *fps));

    if (cliCmd_ == "magick") {
        args << "convert";
    } else {
        args << "convert";
    }
    if (cs > 0) args << "-delay" << QString::number(cs);
    args << "-loop" << QString::number(loop);
    if (!dither) args << "-dither" << "None";
    if (colors > 0) args << "-colors" << QString::number(colors);
    if (optimize) args << "-layers" << "Optimize";
    for (const auto& f : frames) args << f;
    args << outPath;

    QString out, err;
    if (runCli(args, &out, &err) != 0) {
        throw std::runtime_error(("assembleGif (convert) failed: " + err).toStdString());
    }
}

void MagickBackend::assembleWebpCLI(const QStringList& frames, const QString& outPath, int loop,
                                    std::optional<int> delayMs, std::optional<double> fps,
                                    bool lossless, int quality, int method, int nearLossless, int alphaQuality) {
    if (!ensureCli()) throw std::runtime_error("ImageMagick CLI not found.");
    if (frames.isEmpty()) throw std::runtime_error("No frames provided.");
    QStringList args;
    int cs = 0;
    if (delayMs && *delayMs > 0) cs = msToCentiseconds(*delayMs);
    else if (fps && *fps > 0.0) cs = msToCentiseconds(static_cast<int>(1000.0 / *fps));

    if (cliCmd_ == "magick") {
        args << "convert";
    } else {
        args << "convert";
    }
    if (cs > 0) args << "-delay" << QString::number(cs);
    // Loop for webp via define
    args << "-define" << QString("webp:loop=%1").arg(loop);
    args << "-define" << QString("webp:method=%1").arg(std::max(0, std::min(method, 6)));
    if (lossless) {
        args << "-define" << "webp:lossless=true";
    } else {
        quality = std::max(0, std::min(quality, 100));
        nearLossless = std::max(0, std::min(nearLossless, 100));
        alphaQuality = std::max(0, std::min(alphaQuality, 100));
        args << "-quality" << QString::number(quality);
        args << "-define" << QString("webp:near-lossless=%1").arg(nearLossless);
        args << "-define" << QString("webp:alpha-quality=%1").arg(alphaQuality);
    }
    for (const auto& f : frames) args << f;
    args << outPath;

    QString out, err;
    if (runCli(args, &out, &err) != 0) {
        throw std::runtime_error(("assembleWebp (convert) failed: " + err).toStdString());
    }
}
