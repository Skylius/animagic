#pragma once
#include <QSortFilterProxyModel>
#include <QFileSystemModel>
#include <QSet>

// Filters: a proxy model for QFileSystemModel with several modes suitable for Animagic.

namespace Filters {

enum class Mode {
    All,
    Animations,  // *.gif, *.webp
    Stills,      // *.png, *.jpg, *.jpeg, *.tif, *.tiff, *.bmp, *.webp
    Metadata,    // *.json (animagic.meta.json highlighted by consumer)
    FramesOnly   // Only still image extensions
};

class ProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit ProxyModel(QObject* parent = nullptr);

    void setMode(Mode m);
    Mode mode() const { return mode_; }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    bool acceptFile(const QFileInfo& fi) const;
    static bool isAnimationExt(const QString& ext);
    static bool isStillExt(const QString& ext);

private:
    Mode mode_ = Mode::All;
};

} // namespace Filters
