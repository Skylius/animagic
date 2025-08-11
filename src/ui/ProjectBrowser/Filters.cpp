#include "Filters.hpp"
#include <QFileInfo>

using namespace Filters;

ProxyModel::ProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {
    setDynamicSortFilter(true);
}

void ProxyModel::setMode(Mode m) {
    if (mode_ == m) return;
    mode_ = m;
    invalidateFilter();
}

bool ProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
    if (!idx.isValid()) return false;

    QFileSystemModel const* fsm = qobject_cast<QFileSystemModel const*>(sourceModel());
    if (!fsm) return true;

    QFileInfo fi = fsm->fileInfo(idx);
    if (fi.isDir()) return true; // Always show dirs to allow navigation

    return acceptFile(fi);
}

bool ProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
    // Dirs before files, then case-insensitive name compare
    QFileSystemModel const* fsm = qobject_cast<QFileSystemModel const*>(sourceModel());
    if (fsm) {
        QFileInfo a = fsm->fileInfo(left);
        QFileInfo b = fsm->fileInfo(right);
        if (a.isDir() != b.isDir()) return a.isDir();
        return QString::compare(a.fileName(), b.fileName(), Qt::CaseInsensitive) < 0;
    }
    return QSortFilterProxyModel::lessThan(left, right);
}

bool ProxyModel::isAnimationExt(const QString& ext) {
    const QString e = ext.toLower();
    return (e == "gif" || e == "webp");
}

bool ProxyModel::isStillExt(const QString& ext) {
    const QString e = ext.toLower();
    return (e == "png" || e == "jpg" || e == "jpeg" || e == "tif" || e == "tiff" || e == "bmp" || e == "webp");
}

bool ProxyModel::acceptFile(const QFileInfo& fi) const {
    const QString ext = fi.suffix().toLower();
    switch (mode_) {
        case Mode::All:
            return true;
        case Mode::Animations:
            return isAnimationExt(ext);
        case Mode::Stills:
            return isStillExt(ext);
        case Mode::Metadata:
            return ext == "json";
        case Mode::FramesOnly:
            return isStillExt(ext);
    }
    return true;
}
