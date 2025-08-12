#pragma once
#include <QWidget>
class QLineEdit; class QListWidget; class QPushButton;

class DisassemblePage : public QWidget{
    Q_OBJECT
public:
    explicit DisassemblePage(QWidget* parent=nullptr);
signals:
    void status(const QString&);
    void taskStarted(const QString&);
    void taskProgress(int,int);
    void taskSucceeded(const QString&);
    void taskFailed(const QString&);
private slots:
    void chooseInput();
    void chooseOutput();
    void run();
    void onWorkerProgress(int,int);
    void onWorkerDone(bool ok, const QString& msg);
private:
    QLineEdit* inEdit_;
    QLineEdit* outEdit_;
    QListWidget* log_;
    QThread* workerThread_ = nullptr;
};
