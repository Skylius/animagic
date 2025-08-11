#include "FrameListModel.hpp"
#include <QFileInfo>
FrameListModel::FrameListModel(QObject* parent) : QAbstractListModel(parent) {}
int FrameListModel::rowCount(const QModelIndex& parent) const { if (parent.isValid()) return 0; return items_.size(); }
QVariant FrameListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= items_.size()) return {};
    const auto& it = items_[index.row()];
    switch (role) {
        case Qt::DisplayRole: {
            QString name = QFileInfo(it.path).fileName();
            QString extra;
            if (!it.included) extra += " [x]";
            if (it.delayMs.has_value()) extra += QString(" (%1 ms)").arg(*it.delayMs);
            return name + extra;
        }
        case PathRole: return it.path;
        case IncludedRole: return it.included;
        case DelayMsRole: return it.delayMs.has_value() ? QVariant(*it.delayMs) : QVariant();
    }
    return {};
}
bool FrameListModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() < 0 || index.row() >= items_.size()) return false;
    auto& it = items_[index.row()];
    bool changed = false;
    switch (role) {
        case IncludedRole: if (value.isValid()) { it.included = value.toBool(); changed = true; } break;
        case DelayMsRole: if (!value.isValid()) { it.delayMs.reset(); changed = true; } else { it.delayMs = value.toInt(); changed = true; } break;
        default: break;
    }
    if (changed) { emit dataChanged(index, index); return true; }
    return false;
}
Qt::ItemFlags FrameListModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}
void FrameListModel::setItems(const QVector<FrameOps::FrameItem>& v) { beginResetModel(); items_ = v; endResetModel(); }
void FrameListModel::clear() { beginResetModel(); items_.clear(); endResetModel(); }
void FrameListModel::moveRow(int from, int to) {
    if (from == to || from < 0 || to < 0 || from >= items_.size() || to >= items_.size()) return;
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), (to > from) ? to + 1 : to);
    auto item = items_.takeAt(from);
    items_.insert(to, item);
    endMoveRows();
}
void FrameListModel::removeRowsByIndices(const QVector<int>& indices) {
    if (indices.isEmpty()) return;
    QVector<int> sorted = indices;
    std::sort(sorted.begin(), sorted.end());
    for (int i = sorted.size() - 1; i >= 0; --i) {
        int idx = sorted[i];
        if (idx < 0 || idx >= items_.size()) continue;
        beginRemoveRows(QModelIndex(), idx, idx);
        items_.removeAt(idx);
        endRemoveRows();
    }
}
void FrameListModel::duplicateRowsByIndices(const QVector<int>& indices) {
    if (indices.isEmpty()) return;
    QVector<int> sorted = indices;
    std::sort(sorted.begin(), sorted.end());
    int offset = 0;
    for (int idx : sorted) {
        if (idx < 0 || idx >= items_.size()) continue;
        beginInsertRows(QModelIndex(), idx + offset + 1, idx + offset + 1);
        items_.insert(idx + offset + 1, items_[idx + offset]);
        endInsertRows();
        ++offset;
    }
}
void FrameListModel::setIncluded(const QVector<int>& indices, bool value) {
    for (int idx : indices) { if (idx < 0 || idx >= items_.size()) continue; items_[idx].included = value; emit dataChanged(this->index(idx), this->index(idx)); }
}
void FrameListModel::setDelayMs(const QVector<int>& indices, std::optional<int> delayMs) {
    for (int idx : indices) { if (idx < 0 || idx >= items_.size()) continue; items_[idx].delayMs = delayMs; emit dataChanged(this->index(idx), this->index(idx)); }
}
void FrameListModel::clearAllDelays() { for (int i=0;i<items_.size();++i) { items_[i].delayMs.reset(); emit dataChanged(this->index(i), this->index(i)); } }
QStringList FrameListModel::includedPaths() const { return FrameOps::includedPaths(items_); }
