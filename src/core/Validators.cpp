#include "Validators.hpp"
#include <QRegularExpression>
#include <QStandardPaths>

using namespace Validators;

// ------------- Helpers -------------
static bool extMatch(const QString& path, const QStringList& extsLower) {
    QString ext = QFileInfo(path).suffix().toLower();
    return extsLower.isEmpty() || extsLower.contains(ext);
}

// ------------- Filesystem -------------
Result Validators::checkReadableFile(const QString& path, const QStringList& requiredExts) {
    Result r;
    QFileInfo fi(path);
    if (!fi.exists()) r.err(QString("File does not exist: %1").arg(path));
    else if (!fi.isFile()) r.err(QString("Not a file: %1").arg(path));
    else if (!fi.isReadable()) r.err(QString("File is not readable: %1").arg(path));
    if (r.ok && !requiredExts.isEmpty() && !extMatch(path, requiredExts)) {
        r.err(QString("Unexpected file extension (expected one of %1): %2")
              .arg(requiredExts.join(", ")).arg(QFileInfo(path).suffix()));
    }
    return r;
}

Result Validators::checkWritableDir(const QString& dirPath) {
    Result r;
    QDir d(dirPath);
    if (!d.exists()) {
        r.err(QString("Directory does not exist: %1").arg(dirPath));
        return r;
    }
    QString probe = d.filePath(".animagic_write_probe");
    QFile f(probe);
    if (!f.open(QIODevice::WriteOnly)) {
        r.err(QString("Directory is not writable: %1").arg(dirPath));
        return r;
    }
    f.write("ok"); f.close();
    QFile::remove(probe);
    return r;
}

Result Validators::ensureOrCreateDir(const QString& dirPath) {
    Result r;
    QDir d(dirPath);
    if (!d.exists()) {
        if (!d.mkpath(".")) {
            r.err(QString("Failed to create directory: %1").arg(dirPath));
            return r;
        } else {
            r.warn(QString("Created directory: %1").arg(dirPath));
        }
    }
    return checkWritableDir(dirPath);
}

// ------------- Pattern -------------
Result Validators::validateFilenamePattern(const QString& pattern) {
    Result r;
    if (pattern.isEmpty()) { r.err("Filename pattern is empty."); return r; }
    // Must contain exactly one "%d" with optional width (e.g., %04d)
    QRegularExpression re(R"(^(.*)%0?\d*d(.*)$)");
    auto m = re.match(pattern);
    if (!m.hasMatch()) {
        r.err("Pattern must contain a numeric placeholder like %d or %04d.");
        return r;
    }
    // Require an image extension
    QString ext = QFileInfo(pattern).suffix().toLower();
    if (ext.isEmpty()) {
        r.err("Pattern must include an image file extension (e.g., .png).");
    }
    return r;
}

// ------------- Format normalization -------------
std::optional<QString> Validators::normalizeStillFormat(const QString& fmt) {
    QString f = fmt.trimmed().toLower();
    if (f.isEmpty()) return std::nullopt;
    static const QStringList ok = {"png","jpg","jpeg","tif","tiff","bmp","webp"};
    if (!ok.contains(f)) return std::nullopt;
    return f;
}

std::optional<QString> Validators::normalizeTargetFormat(const QString& fmt) {
    QString f = fmt.trimmed().toLower();
    if (f == "gif" || f == "webp") return f;
    return std::nullopt;
}

// ------------- Disassemble params -------------
Result Validators::validate(const DisassembleParams& p) {
    Result r;
    // source
    {
        auto rr = checkReadableFile(p.sourcePath, {"gif","webp"});
        if (!rr.ok) r.err(rr.errors.join('\n'));
        r.warnings.append(rr.warnings);
    }
    // out folder
    {
        auto rr = ensureOrCreateDir(p.outFolder);
        if (!rr.ok) r.err(rr.errors.join('\n'));
        r.warnings.append(rr.warnings);
    }
    // pattern
    {
        auto rr = validateFilenamePattern(p.pattern);
        if (!rr.ok) r.err(rr.errors.join('\n'));
        r.warnings.append(rr.warnings);
    }
    // still format (optional: if empty, infer from pattern)
    if (!p.stillFormat.isEmpty()) {
        if (!normalizeStillFormat(p.stillFormat).has_value()) {
            r.err(QString("Unsupported still format: %1").arg(p.stillFormat));
        }
    }
    return r;
}

// ------------- Assemble params -------------
Result Validators::validate(const AssembleParams& p) {
    Result r;
    // target format
    auto tf = normalizeTargetFormat(p.targetFormat);
    if (!tf.has_value()) r.err(QString("Unsupported target format: %1 (use gif or webp)").arg(p.targetFormat));

    // frames
    if (p.frames.isEmpty()) r.err("No frames provided.");
    for (const auto& f : p.frames) {
        QFileInfo fi(f);
        if (!fi.exists() || !fi.isFile() || !fi.isReadable()) {
            r.err(QString("Frame not readable: %1").arg(f));
            break;
        }
    }

    // out path
    if (p.outPath.isEmpty()) {
        r.err("Output path is empty.");
    } else {
        QDir outd = QFileInfo(p.outPath).dir();
        if (!outd.exists()) {
            if (!outd.mkpath(".")) r.err(QString("Failed to create output directory: %1").arg(outd.path()));
            else r.warn(QString("Created directory: %1").arg(outd.path()));
        }
        // Extension matches target
        QString ext = QFileInfo(p.outPath).suffix().toLower();
        if (tf.has_value() && ext != *tf) {
            r.warn(QString("Output extension .%1 does not match target format %2; proceeding.")
                   .arg(ext, *tf));
        }
    }

    // Timing consistency
    if (p.delayMs.has_value() && p.fps.has_value()) {
        r.warn("Both delay and fps provided; delay will take precedence.");
    }
    if (p.delayMs.has_value() && *p.delayMs < 0) {
        r.err("delayMs cannot be negative.");
    }
    if (p.fps.has_value() && *p.fps <= 0.0) {
        r.err("fps must be > 0.");
    }

    // GIF options
    if (tf == std::optional<QString>("gif")) {
        if (p.gif.colors < 2 || p.gif.colors > 256) {
            r.err("GIF palette size (colors) must be between 2 and 256.");
        }
    }

    // WebP options
    if (tf == std::optional<QString>("webp")) {
        if (!p.webp.lossless) {
            if (p.webp.quality < 0 || p.webp.quality > 100) r.err("WebP quality must be 0..100.");
            if (p.webp.nearLossless < 0 || p.webp.nearLossless > 100) r.err("WebP near-lossless must be 0..100.");
            if (p.webp.alphaQuality < 0 || p.webp.alphaQuality > 100) r.err("WebP alpha-quality must be 0..100.");
        }
        if (p.webp.method < 0 || p.webp.method > 6) r.err("WebP method must be 0..6.");
    }

    return r;
}

// ------------- Environment -------------
Validators::BackendAvailability Validators::detectImageMagickCLI() {
    BackendAvailability b;
    // Check magick
    {
        QProcess p;
        p.start("bash", {"-lc", "command -v magick"});
        p.waitForFinished();
        const bool ok = (p.exitCode() == 0) && !QString(p.readAllStandardOutput()).trimmed().isEmpty();
        if (ok) {
            b.cli = true; b.cliCommand = "magick"; return b;
        }
    }
    // Check legacy convert+identify
    {
        QProcess p1, p2;
        p1.start("bash", {"-lc", "command -v convert"});
        p2.start("bash", {"-lc", "command -v identify"});
        p1.waitForFinished(); p2.waitForFinished();
        const bool ok1 = (p1.exitCode() == 0) && !QString(p1.readAllStandardOutput()).trimmed().isEmpty();
        const bool ok2 = (p2.exitCode() == 0) && !QString(p2.readAllStandardOutput()).trimmed().isEmpty();
        if (ok1 && ok2) { b.cli = true; b.cliCommand = "legacy"; return b; }
    }
    b.cli = false;
    return b;
}
