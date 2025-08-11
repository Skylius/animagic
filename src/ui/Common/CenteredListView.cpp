#include "CenteredListView.hpp"
#include <QStyleOptionViewItem>
#include <QPaintEvent>
CenteredListView::CenteredListView(QWidget* parent) : QListView(parent) {
    setUniformItemSizes(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragDropMode(QAbstractItemView::DropOnly);
    setAcceptDrops(true);
}
void CenteredListView::paintEvent(QPaintEvent* e) {
    QListView::paintEvent(e);
    if (model() && model()->rowCount() > 0) return;
    QPainter p(viewport());
    p.setPen(QColor(128,128,128));
    QFont f = p.font(); f.setPointSizeF(f.pointSizeF() * 1.1); p.setFont(f);
    QRect r = viewport()->rect();
    p.drawText(r, Qt::AlignCenter | Qt::TextWordWrap, placeholder_);
}
