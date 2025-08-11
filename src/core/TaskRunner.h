#pragma once
#include <QObject>
#include <QThreadPool>
#include <QRunnable>
#include <QPointer>
#include <QMutex>
#include <QQueue>
#include <optional>
#include "Task.hpp"
#include "Cancellation.hpp"
class TaskRunner : public QObject {
    Q_OBJECT
public:
    explicit TaskRunner(QObject* parent=nullptr);
    void setMaxThreads(int n) { pool_.setMaxThreadCount(n); }
    int maxThreads() const { return pool_.maxThreadCount(); }
    void queue(Task* task);
    void cancelCurrent();
signals:
    void queueEmpty();
    void taskStarted(const QString& name);
    void taskProgress(int done, int total);
    void taskLog(const QString& msg);
    void taskSucceeded(const QString& name);
    void taskFailed(const QString& name, const QString& error);
    void taskCancelled(const QString& name);
private:
    void startNext();
private:
    QThreadPool pool_;
    QMutex mtx_;
    QQueue<QPointer<Task>> q_;
    std::optional<QString> currentName_;
    CancellationSource cancelSrc_;
};
