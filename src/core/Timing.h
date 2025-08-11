#pragma once
#include <QVector>
#include <optional>
#include <cmath>

// Timing utilities for Animagic.
// Handles conversions between ms <-> centiseconds (Magick), FPS <-> delay,
// building per-frame delay arrays with overrides, and summarization.

namespace Timing {

// Reasonable bounds for animation delays.
constexpr int kMinDelayMs = 10;       // 1 cs - many decoders treat <10ms loosely
constexpr int kMaxDelayMs = 60000;    // clamp at 60s

// Convert milliseconds to centiseconds (Magick units).
// If ms <= 0 returns 0; otherwise minimum 1cs.
inline int msToCentiseconds(int ms) {
    if (ms <= 0) return 0;
    int cs = (ms + 5) / 10;           // round to nearest
    return cs < 1 ? 1 : cs;
}

// Convert centiseconds to milliseconds.
inline int centisecondsToMs(int cs) {
    if (cs <= 0) return 0;
    return cs * 10;
}

// Convert FPS to ms delay (rounded).
inline int fpsToDelayMs(double fps) {
    if (fps <= 0.0) return 0;
    double ms = 1000.0 / fps;
    return static_cast<int>(std::lround(ms));
}

// Clamp delay to sane bounds; allow 0 to mean "unspecified".
inline int clampDelayMs(int ms) {
    if (ms <= 0) return 0;
    if (ms < kMinDelayMs) return kMinDelayMs;
    if (ms > kMaxDelayMs) return kMaxDelayMs;
    return ms;
}

// Build a per-frame delay array in milliseconds.
// Priority: per-frame override -> globalDelayMs -> fps-derived.
// If all are unspecified/zero, defaults to 100ms.
QVector<int> buildDelays(int frameCount,
                         std::optional<int> globalDelayMs,
                         std::optional<double> fps,
                         const QVector<std::optional<int>>& perFrameOverrides);

// Convert ms delays to Magick centiseconds with clamping (>=1 for positive delays).
QVector<int> toCentiseconds(const QVector<int>& delaysMs);

// Adjust playback speed by a scalar (e.g., 0.5x, 2x).
// Returns clamped, rounded ms delays.
QVector<int> scaleDelays(const QVector<int>& delaysMs, double scalar);

// Summary of delays for UI display.
struct Summary {
    bool uniform = false;
    int minMs = 0;
    int maxMs = 0;
    double avgMs = 0.0;
};

Summary summarize(const QVector<int>& delaysMs);

// Merge an existing delay array with sparse overrides (indices in [0,n)).
void applyOverrides(QVector<int>& delaysMs, const QVector<std::optional<int>>& perFrameOverrides);

} // namespace Timing
