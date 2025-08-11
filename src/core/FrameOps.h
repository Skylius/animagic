#pragma once
#include <QString>
#include <QStringList>
#include <QVector>
#include <QDir>
#include <QFileInfo>
#include <optional>
#include <functional>

// FrameOps: helpers to manage lists of frames in memory (ordering, include/exclude, per-frame delays).
// Pure helpers: no GUI and minimal I/O (only directory scans).
//
// Model
// -----
// FrameItem {
//   QString path;               // absolute or project-relative path to the still image
//   bool included = true;       // whether this frame participates in assembly
//   std::optional<int> delayMs; // per-frame delay override (ms); if unset, use global delay/fps
// }
//
// The collection is represented as QVector<FrameItem>. Functions below mutate in place.

namespace FrameOps {

struct FrameItem {
    QString path;
    bool included = true;
    std::optional<int> delayMs;
};

// Natural-sort compare (numbers in filenames compare as numbers)
bool lessNatural(const QString& a, const QString& b);

// Load frames from a folder filtering by extensions, optionally applying natural sort.
QVector<FrameItem> loadFromFolder(const QString& folder,
                                  const QStringList& extensions = {"png","jpg","jpeg","tif","tiff","bmp"},
                                  bool naturalSort = true);

// Extract paths for included frames in current order.
QStringList includedPaths(const QVector<FrameItem>& items);

// Mutations
void sortByPath(QVector<FrameItem>& items, bool natural = true, bool ascending = true);
void reverse(QVector<FrameItem>& items);
void move(QVector<FrameItem>& items, int from, int to);
void removeIndices(QVector<FrameItem>& items, const QVector<int>& indices);          // delete frames
void duplicateIndices(QVector<FrameItem>& items, const QVector<int>& indices);       // duplicate frames (insert copies after original)
void setIncluded(QVector<FrameItem>& items, const QVector<int>& indices, bool value);
void setDelayMs(QVector<FrameItem>& items, const QVector<int>& indices, std::optional<int> delayMs);
void clearAllDelays(QVector<FrameItem>& items);

// Transform: apply function to each selected item
void transform(QVector<FrameItem>& items, const QVector<int>& indices, const std::function<void(FrameItem&)>& fn);

// Utility to validate paths exist (best-effort); returns list of missing ones.
QStringList missingPaths(const QVector<FrameItem>& items);

// Stable unique: remove duplicate paths keeping first occurrence; returns number removed.
int uniqueByPath(QVector<FrameItem>& items);

} // namespace FrameOps
