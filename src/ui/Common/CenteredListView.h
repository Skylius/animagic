#pragma once
#include <QListView>
#include <QPainter>
class CenteredListView : public QListView {
    Q_OBJECT
public:
    explicit CenteredListView(QWidget* parent = nullptr);
    void setPlaceholderText(const QString& t) { placeholder_ = t; viewport()->update(); }
    QString placeholderText() const { return placeholder_; }
protected:
    void paintEvent(QPaintEvent* e) override;
private:
    QString placeholder_ = "Drop files here or use Add…";
};
