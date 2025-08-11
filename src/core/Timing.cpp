#include "Timing.hpp"
#include <algorithm>
#include <numeric>

using namespace Timing;

QVector<int> Timing::buildDelays(int frameCount,
                                 std::optional<int> globalDelayMs,
                                 std::optional<double> fps,
                                 const QVector<std::optional<int>>& perFrameOverrides) {
    if (frameCount <= 0) return {};
    int baseMs = 0;
    if (globalDelayMs && *globalDelayMs > 0) baseMs = clampDelayMs(*globalDelayMs);
    else if (fps && *fps > 0.0) baseMs = clampDelayMs(fpsToDelayMs(*fps));
    else baseMs = 100; // sensible default

    QVector<int> delays(frameCount, baseMs);
    applyOverrides(delays, perFrameOverrides);
    return delays;
}

QVector<int> Timing::toCentiseconds(const QVector<int>& delaysMs) {
    QVector<int> out;
    out.reserve(delaysMs.size());
    for (int ms : delaysMs) {
        out.push_back(msToCentiseconds(ms));
    }
    return out;
}

QVector<int> Timing::scaleDelays(const QVector<int>& delaysMs, double scalar) {
    if (scalar <= 0.0) scalar = 1.0;
    QVector<int> out;
    out.reserve(delaysMs.size());
    for (int ms : delaysMs) {
        // Keep 0 as 0 (unspecified), scale others
        if (ms <= 0) { out.push_back(0); continue; }
        double scaled = static_cast<double>(ms) * scalar;
        int clamped = clampDelayMs(static_cast<int>(std::lround(scaled)));
        out.push_back(clamped);
    }
    return out;
}

Timing::Summary Timing::summarize(const QVector<int>& delaysMs) {
    Summary s;
    if (delaysMs.isEmpty()) return s;
    s.minMs = *std::min_element(delaysMs.begin(), delaysMs.end());
    s.maxMs = *std::max_element(delaysMs.begin(), delaysMs.end());
    long long sum = 0;
    for (int v : delaysMs) sum += v;
    s.avgMs = static_cast<double>(sum) / static_cast<double>(delaysMs.size());
    s.uniform = (s.minMs == s.maxMs);
    return s;
}

void Timing::applyOverrides(QVector<int>& delaysMs, const QVector<std::optional<int>>& perFrameOverrides) {
    if (perFrameOverrides.isEmpty()) return;
    const int n = std::min(delaysMs.size(), perFrameOverrides.size());
    for (int i = 0; i < n; ++i) {
        if (perFrameOverrides[i].has_value()) {
            delaysMs[i] = clampDelayMs(*perFrameOverrides[i]);
        }
    }
}
