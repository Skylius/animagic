#pragma once
#include <QWidget>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
class ProgressLogPane : public QWidget {
    Q_OBJECT
public:
    explicit ProgressLogPane(QWidget* parent = nullptr);
    void setIndeterminate(bool on);
    void setProgress(int done, int total);
    void setRange(int min, int max) { bar_->setRange(min, max); }
    void setValue(int v) { bar_->setValue(v); }
    void log(const QString& line);
    void clearLog();
    QProgressBar* progressBar() const { return bar_; }
    QPlainTextEdit* logView() const { return log_; }
private:
    QProgressBar* bar_ = nullptr;
    QPlainTextEdit* log_ = nullptr;
};
