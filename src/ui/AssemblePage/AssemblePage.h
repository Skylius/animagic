#pragma once
#include <QWidget>
#include <QListView>
#include <QSplitter>
#include <QToolBar>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QMenu>
#include <QAction>
#include "ui/Common/CenteredListView.hpp"
#include "ui/Common/ProgressLogPane.hpp"
#include "FrameListModel.hpp"
#include "ui/AssemblePage/PreviewWidget.hpp"
#include "core/FrameOps.hpp"
#include "core/Validators.hpp"
#include "core/MagickBackend.hpp"
#include "core/Timing.hpp"
#include "core/Profiles.hpp"
#include "core/ProjectMetadata.hpp"
#include "ui/Common/TaskBar.hpp"
#include "FrameTimingEditor.hpp"
#include "core/TaskRunner.hpp"
#include "core/Task.hpp"
class AssemblePage : public QWidget {
    Q_OBJECT
public:
    explicit AssemblePage(QWidget* parent = nullptr);
    explicit AssemblePage(TaskRunner* sharedRunner, QWidget* parent = nullptr);
signals:
    void assembled(const QString& outPath);
private slots:
    void onAddFiles();
    void onAddFolder();
    void onRemove();
    void onDuplicate();
    void onMoveUp();
    void onMoveDown();
    void onClear();
    void onEditTiming();
    void onPreviewPlay();
    void onPreviewPause();
    void onAssemble();
    void onImportMetadata();
    void onApplyProfile();
private:
    void buildUi();
    void log(const QString& line);
    QVector<std::optional<int>> collectOverrides() const;
private:
    CenteredListView* list_ = nullptr;
    FrameListModel* model_ = nullptr;
    QToolBar* listToolbar_ = nullptr;
    PreviewWidget* preview_ = nullptr;
    QComboBox* targetFmt_ = nullptr;
    QSpinBox* loopSpin_ = nullptr;
    QSpinBox* delayMsSpin_ = nullptr;
    QDoubleSpinBox* fpsSpin_ = nullptr;
    QCheckBox* gifOptimize_ = nullptr;
    QCheckBox* gifDither_ = nullptr;
    QSpinBox* gifColors_ = nullptr;
    QCheckBox* webpLossless_ = nullptr;
    QSpinBox* webpQuality_ = nullptr;
    QSpinBox* webpMethod_ = nullptr;
    QSpinBox* webpNearLossless_ = nullptr;
    QSpinBox* webpAlphaQuality_ = nullptr;
    QPushButton* btnAssemble_ = nullptr;
    QPushButton* btnCancel_ = nullptr;
    ProgressLogPane* progressLog_ = nullptr;
    TaskBar* taskBar_ = nullptr;
    MagickBackend backend_;
    TaskRunner* runner_ = nullptr;
