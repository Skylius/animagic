#include "TaskRunner.hpp"
#include <QtConcurrent>
#include <QMutexLocker>

namespace {
class TaskJob : public QRunnable {
public:
    TaskJob(QPointer<Task> t, CancellationToken tok, TaskRunner* forwardSignals, QString name)
        : task_(t), tok_(tok), forward_(forwardSignals), name_(std::move(name)) { setAutoDelete(true); }
    void run() override {
        if (!task_) return;
        QObject::connect(task_, &Task::progress, forward_, &TaskRunner::taskProgress, Qt::QueuedConnection);
        QObject::connect(task_, &Task::log, forward_, &TaskRunner::taskLog, Qt::QueuedConnection);
        task_->run(tok_);
    }
private:
    QPointer<Task> task_;
    CancellationToken tok_;
    TaskRunner* forward_;
    QString name_;
};
}
TaskRunner::TaskRunner(QObject* parent) : QObject(parent) { pool_.setMaxThreadCount(qMax(1, QThread::idealThreadCount())); }
void TaskRunner::queue(Task* task) {
    if (!task) return;
    QMutexLocker lock(&mtx_);
    q_.enqueue(task);
    if (!currentName_.has_value()) QMetaObject::invokeMethod(this, [this]{ startNext(); }, Qt::QueuedConnection);
}
void TaskRunner::cancelCurrent() { cancelSrc_.cancel(); }
void TaskRunner::startNext() {
    QPointer<Task> task;
    { QMutexLocker lock(&mtx_); if (q_.isEmpty()) { currentName_.reset(); emit queueEmpty(); return; } task = q_.dequeue(); }
    if (!task) { QMetaObject::invokeMethod(this, [this]{ startNext(); }, Qt::QueuedConnection); return; }
    currentName_ = task->name();
    cancelSrc_.reset();
    emit taskStarted(*currentName_);
    auto job = new TaskJob(task, cancelSrc_.token(), this, *currentName_);
    QObject::connect(task, &Task::succeeded, this, [this]{ if (currentName_.has_value()) emit taskSucceeded(*currentName_); currentName_.reset(); startNext(); }, Qt::QueuedConnection);
    QObject::connect(task, &Task::failed, this, [this](const QString& err){ if (currentName_.has_value()) emit taskFailed(*currentName_, err); currentName_.reset(); startNext(); }, Qt::QueuedConnection);
    QObject::connect(task, &Task::cancelled, this, [this]{ if (currentName_.has_value()) emit taskCancelled(*currentName_); currentName_.reset(); startNext(); }, Qt::QueuedConnection);
    pool_.start(job);
}
