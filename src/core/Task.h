#pragma once
#include <QObject>
#include <QString>
#include <functional>
#include <memory>
#include <optional>
#include "Cancellation.h"
class Task : public QObject {
    Q_OBJECT
public:
    using ProgressFn = std::function<void(int,int)>;
    using LogFn = std::function<void(const QString&)>;
    using Body = std::function<void(CancellationToken, ProgressFn, LogFn)>;
    explicit Task(QString name, Body body, QObject* parent=nullptr)
        : QObject(parent), name_(std::move(name)), body_(std::move(body)) {}
    const QString& name() const { return name_; }
signals:
    void started(const QString& name);
    void progress(int done, int total);
    void log(const QString& line);
    void succeeded();
    void failed(const QString& error);
    void cancelled();
public:
    void run(CancellationToken tok) {
        try {
            emit started(name_);
            body_(tok,
                  [this](int d, int t){ emit progress(d,t); },
                  [this](const QString& s){ emit log(s); });
            if (tok.cancelled()) emit cancelled();
            else emit succeeded();
        } catch (const std::exception& e) {
            emit failed(QString::fromUtf8(e.what()));
        } catch (...) {
            emit failed(QStringLiteral("Unknown error"));
        }
    }
private:
    QString name_;
    Body body_;
};
