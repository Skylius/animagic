#include "FrameOps.hpp"
#include <QRegularExpression>
#include <algorithm>

using namespace FrameOps;

static QStringList splitAlphaNum(const QString& s) {
    // Split into alternating digit and non-digit runs
    QStringList parts;
    QString buf;
    bool inDigit = false;
    for (QChar c : s) {
        bool d = c.isDigit();
        if (parts.isEmpty()) {
            buf = c;
            inDigit = d;
        } else if (d == inDigit) {
            buf.append(c);
        } else {
            parts << buf;
            buf = c;
            inDigit = d;
        }
    }
    if (!buf.isEmpty()) parts << buf;
    return parts;
}

bool FrameOps::lessNatural(const QString& a, const QString& b) {
    // Case-insensitive, numeric-aware compare of basenames
    QString as = QFileInfo(a).fileName().toLower();
    QString bs = QFileInfo(b).fileName().toLower();
    auto ap = splitAlphaNum(as);
    auto bp = splitAlphaNum(bs);
    int n = std::min(ap.size(), bp.size());
    for (int i=0; i<n; ++i) {
        bool ad = ap[i][0].isDigit();
        bool bd = bp[i][0].isDigit();
        if (ad && bd) {
            // compare as integers (preserve length as tie-breaker)
            auto ai = ap[i].toLongLong();
            auto bi = bp[i].toLongLong();
            if (ai != bi) return ai < bi;
            if (ap[i].size() != bp[i].size()) return ap[i].size() < bp[i].size();
        } else if (!ad && !bd) {
            int cmp = QString::localeAwareCompare(ap[i], bp[i]);
            if (cmp != 0) return cmp < 0;
        } else {
            // digits come before letters
            return ad;
        }
    }
    return ap.size() < bp.size();
}

QVector<FrameOps::FrameItem> FrameOps::loadFromFolder(const QString& folder,
                                                      const QStringList& extensions,
                                                      bool naturalSort) {
    QVector<FrameItem> out;
    QDir dir(folder);
    if (!dir.exists()) return out;
    QStringList filters;
    for (const auto& ext : extensions) {
        filters << "*." + ext;
        filters << "*." + ext.toUpper();
    }
    QFileInfoList list = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    // Convert to QStringList of absolute paths
    QStringList paths;
    for (const QFileInfo& fi : list) paths << fi.absoluteFilePath();
    if (naturalSort) {
        std::sort(paths.begin(), paths.end(), [](const QString& lhs, const QString& rhs){
            return lessNatural(lhs, rhs);
        });
    }
    out.reserve(paths.size());
    for (const auto& p : paths) out.push_back({p, true, std::nullopt});
    return out;
}

QStringList FrameOps::includedPaths(const QVector<FrameItem>& items) {
    QStringList out;
    out.reserve(items.size());
    for (const auto& it : items) if (it.included) out << it.path;
    return out;
}

void FrameOps::sortByPath(QVector<FrameItem>& items, bool natural, bool ascending) {
    std::sort(items.begin(), items.end(), [&](const FrameItem& A, const FrameItem& B){
        if (natural) {
            bool res = lessNatural(A.path, B.path);
            return ascending ? res : !res;
        } else {
            int cmp = QString::compare(A.path, B.path, Qt::CaseInsensitive);
            return ascending ? (cmp < 0) : (cmp > 0);
        }
    });
}

void FrameOps::reverse(QVector<FrameItem>& items) {
    std::reverse(items.begin(), items.end());
}

void FrameOps::move(QVector<FrameItem>& items, int from, int to) {
    if (from < 0 || from >= items.size() || to < 0 || to >= items.size() || from == to) return;
    auto item = items[from];
    items.remove(from);
    items.insert(to, item);
}

void FrameOps::removeIndices(QVector<FrameItem>& items, const QVector<int>& indices) {
    if (indices.isEmpty()) return;
    // remove from back to front to keep indices valid
    QVector<int> sorted = indices;
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
    for (int i = sorted.size() - 1; i >= 0; --i) {
        int idx = sorted[i];
        if (idx >= 0 && idx < items.size()) items.removeAt(idx);
    }
}

void FrameOps::duplicateIndices(QVector<FrameItem>& items, const QVector<int>& indices) {
    if (indices.isEmpty()) return;
    // insert duplicates AFTER each original; track offset
    QVector<int> sorted = indices;
    std::sort(sorted.begin(), sorted.end());
    int offset = 0;
    for (int idx : sorted) {
        if (idx >= 0 && idx < items.size()) {
            FrameItem copy = items[idx + offset];
            items.insert(idx + offset + 1, copy);
            ++offset;
        }
    }
}

void FrameOps::setIncluded(QVector<FrameItem>& items, const QVector<int>& indices, bool value) {
    for (int idx : indices) if (idx >= 0 && idx < items.size()) items[idx].included = value;
}

void FrameOps::setDelayMs(QVector<FrameItem>& items, const QVector<int>& indices, std::optional<int> delayMs) {
    for (int idx : indices) if (idx >= 0 && idx < items.size()) items[idx].delayMs = delayMs;
}

void FrameOps::clearAllDelays(QVector<FrameItem>& items) {
    for (auto& it : items) it.delayMs.reset();
}

void FrameOps::transform(QVector<FrameItem>& items, const QVector<int>& indices, const std::function<void(FrameItem&)>& fn) {
    for (int idx : indices) if (idx >= 0 && idx < items.size()) fn(items[idx]);
}

QStringList FrameOps::missingPaths(const QVector<FrameItem>& items) {
    QStringList miss;
    for (const auto& it : items) {
        if (!QFileInfo::exists(it.path)) miss << it.path;
    }
    return miss;
}

int FrameOps::uniqueByPath(QVector<FrameItem>& items) {
    // Preserve order of first appearance. Use a set of canonicalized paths.
    QSet<QString> seen;
    int removed = 0;
    for (int i = 0; i < items.size(); ) {
        QString key = QFileInfo(items[i].path).canonicalFilePath();
        if (key.isEmpty()) key = items[i].path;
        if (seen.contains(key)) {
            items.removeAt(i);
            ++removed;
        } else {
            seen.insert(key);
            ++i;
        }
    }
    return removed;
}
