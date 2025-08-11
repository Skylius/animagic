#pragma once
#include <QAbstractListModel>
#include <QVector>
#include <optional>
#include "core/FrameOps.hpp"

class FrameListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { PathRole = Qt::UserRole + 1, IncludedRole, DelayMsRole };
    explicit FrameListModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    const QVector<FrameOps::FrameItem>& items() const { return items_; }
    QVector<FrameOps::FrameItem>& items() { return items_; }
    void setItems(const QVector<FrameOps::FrameItem>& v);
    void clear();
    void moveRow(int from, int to);
    void removeRowsByIndices(const QVector<int>& indices);
    void duplicateRowsByIndices(const QVector<int>& indices);
    void setIncluded(const QVector<int>& indices, bool value);
    void setDelayMs(const QVector<int>& indices, std::optional<int> delayMs);
    void clearAllDelays();
    QStringList includedPaths() const;
    int size() const { return items_.size(); }
private:
    QVector<FrameOps::FrameItem> items_;
