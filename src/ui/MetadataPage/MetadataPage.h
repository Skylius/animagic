#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include "ui/Common/ProgressLogPane.hpp"
#include "core/ProjectMetadata.hpp"
class MetadataPage : public QWidget {
    Q_OBJECT
public:
    explicit MetadataPage(QWidget* parent = nullptr);
signals:
    void loaded(const QString& filePath);
    void saved(const QString& filePath);
private slots:
    void onLoad();
    void onSaveAs();
    void onApplyToForm(const ProjectMetadata& md);
    void onCollectFromForm(ProjectMetadata& md);
private:
    void buildUi();
    void log(const QString& s);
private:
    QLineEdit* srcPath_ = nullptr;
    QLineEdit* srcFormat_ = nullptr;
    QSpinBox* srcWidth_ = nullptr;
    QSpinBox* srcHeight_ = nullptr;
    QSpinBox* srcFrames_ = nullptr;
    QSpinBox* srcLoop_ = nullptr;
    QPlainTextEdit* srcDelays_ = nullptr;
    QLineEdit* stillFolder_ = nullptr;
    QLineEdit* stillPattern_ = nullptr;
    QComboBox* stillFormat_ = nullptr;
    QComboBox* targetFmt_ = nullptr;
    QSpinBox* loop_ = nullptr;
    QSpinBox* delayMs_ = nullptr;
    QDoubleSpinBox* fps_ = nullptr;
    QCheckBox* gifOptimize_ = nullptr;
    QCheckBox* gifDither_ = nullptr;
    QSpinBox* gifColors_ = nullptr;
    QCheckBox* webpLossless_ = nullptr;
    QSpinBox* webpQuality_ = nullptr;
    QSpinBox* webpMethod_ = nullptr;
    QSpinBox* webpNearLossless_ = nullptr;
    QSpinBox* webpAlphaQuality_ = nullptr;
    ProgressLogPane* progressLog_ = nullptr;
};
