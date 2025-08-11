#pragma once
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QHBoxLayout>
class TaskBar : public QWidget {
    Q_OBJECT
public:
    explicit TaskBar(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* row = new QHBoxLayout(this);
        row->setContentsMargins(6,6,6,6);
        row->setSpacing(8);
        icon_ = new QLabel(this);
        icon_->setText("🛠️");
        label_ = new QLabel("Idle", this);
        bar_ = new QProgressBar(this);
        bar_->setRange(0, 1);
        bar_->setValue(0);
        bar_->setTextVisible(false);
        cancel_ = new QPushButton("Cancel", this);
        cancel_->setEnabled(false);
        row->addWidget(icon_, 0);
        row->addWidget(label_, 0);
        row->addWidget(bar_, 1);
        row->addWidget(cancel_, 0);
        connect(cancel_, &QPushButton::clicked, this, &TaskBar::cancelRequested);
    }
public slots:
    void setActive(const QString& name) { label_->setText(name); bar_->setRange(0, 0); cancel_->setEnabled(true); setVisible(true); }
    void setProgress(int done, int total) { if (total <= 0) { bar_->setRange(0, 0); } else { bar_->setRange(0, total); bar_->setValue(done); } }
    void setIdle() { label_->setText("Idle"); bar_->setRange(0, 1); bar_->setValue(0); cancel_->setEnabled(false); setVisible(false); }
signals:
    void cancelRequested();
private:
    QLabel* icon_ = nullptr;
    QLabel* label_ = nullptr;
    QProgressBar* bar_ = nullptr;
    QPushButton* cancel_ = nullptr;
};
