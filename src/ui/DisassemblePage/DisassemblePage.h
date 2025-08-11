#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QGroupBox>
#include "ui/Common/ProgressLogPane.hpp"
#include "ui/Common/TaskBar.hpp"

#include "DisassembleViewModel.hpp"
#include "core/TaskRunner.hpp"
#include "core/Task.hpp"

// DisassemblePage: UI for analyzing and extracting frames from GIF/WebP
class DisassemblePage : public QWidget {
    Q_OBJECT
public:
    explicit DisassemblePage(QWidget* parent = nullptr);
    explicit DisassemblePage(TaskRunner* sharedRunner, QWidget* parent = nullptr);

signals:
    void analyzed();
    void extracted(const QString& framesFolder);

private slots:
    void onBrowseSource();
    void onBrowseOutFolder();
    void onAnalyze();
    void onExtract();

private:
    void buildUi();
    void log(const QString& line);

private:
    DisassembleViewModel vm_;
    TaskRunner* runner_ = nullptr;
    QLineEdit* srcEdit_ = nullptr;
    QLineEdit* outEdit_ = nullptr;
    QLineEdit* patternEdit_ = nullptr;
    QComboBox* stillFmt_ = nullptr;
    QCheckBox* coalesceBox_ = nullptr;

    QLabel* infoLabel_ = nullptr;
    ProgressLogPane* progressLog_ = nullptr;
    TaskBar* taskBar_ = nullptr;
    QPushButton* btnAnalyze_ = nullptr;
    QPushButton* btnExtract_ = nullptr;
    QPushButton* btnCancel_ = nullptr;
};
