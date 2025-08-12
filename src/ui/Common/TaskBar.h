#pragma once
#include <QWidget>
class QProgressBar; class QPushButton; class QLabel;
class TaskBar : public QWidget{
    Q_OBJECT
public:
    explicit TaskBar(QWidget* parent=nullptr);
signals:
    void cancelRequested();
    void startRequested();
public slots:
    void setActive(const QString& name);
    void setIdle();
    void setProgress(int done, int total);
private:
    QLabel* label_;
    QProgressBar* bar_;
    QPushButton* start_;
    QPushButton* cancel_;
};
