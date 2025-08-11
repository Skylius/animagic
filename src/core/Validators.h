#pragma once
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <optional>

// Validators: sanity checks for user inputs and assembly/disassembly parameters.
// These helpers are GUI-agnostic and return structured results suitable for UI display.

namespace Validators {

struct Result {
    bool ok = true;
    QStringList warnings;
    QStringList errors;

    void warn(const QString& w) { warnings << w; }
    void err(const QString& e)  { errors << e; ok = false; }
};

// -------- Filesystem --------
Result checkReadableFile(const QString& path, const QStringList& requiredExts = {});
Result checkWritableDir(const QString& dirPath);
Result ensureOrCreateDir(const QString& dirPath);             // attempts to create if missing

// Validate a printf-like frame filename pattern, e.g., "frame_%04d.png"
// Ensures it contains exactly one "%d" (optionally with width), and an image extension.
Result validateFilenamePattern(const QString& pattern);

// Validate still format (png/jpeg/jpg/tiff/bmp/webp/...); returns normalized lowercase ext
std::optional<QString> normalizeStillFormat(const QString& fmt);

// Validate target animation format ("gif" or "webp"); returns normalized lowercase
std::optional<QString> normalizeTargetFormat(const QString& fmt);

// -------- Params: Disassemble --------
struct DisassembleParams {
    QString sourcePath;
    QString outFolder;
    QString pattern;      // e.g., "frame_%04d.png"
    QString stillFormat;  // may be empty to infer from pattern
    bool coalesce = true;
};

Result validate(const DisassembleParams& p);

// -------- Params: Assemble --------
struct GifOptions {
    bool optimize = true;
    bool dither = true;
    int colors = 256; // 2..256
};

struct WebpOptions {
    bool lossless = false;
    int quality = 85;        // 0..100 (ignored if lossless)
    int method = 4;          // 0..6
    int nearLossless = 0;    // 0..100 (ignored if lossless)
    int alphaQuality = 100;  // 0..100
};

struct AssembleParams {
    QStringList frames;         // ordered full paths
    QString outPath;            // final animation path (.gif or .webp)
    int loop = 0;               // 0=infinite
    std::optional<int> delayMs; // global delay
    std::optional<double> fps;  // alternative to delay
    QString targetFormat;       // "gif" or "webp"
    GifOptions gif;
    WebpOptions webp;
};

Result validate(const AssembleParams& p);

// -------- Environment --------
struct BackendAvailability {
    bool magickpp = true;   // Assume available if we linked; caller can override
    bool cli = false;
    QString cliCommand;     // "magick" or "legacy" (convert/identify)
};

BackendAvailability detectImageMagickCLI();

} // namespace Validators
