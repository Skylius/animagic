#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QStandardPaths>
#include "ui/Common/ProgressLogPane.hpp"
#include "core/Validators.hpp"
#include "core/Profiles.hpp"
#include "core/MagickBackend.hpp"
class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(QWidget* parent = nullptr);
private slots:
    void onChooseDefaultProject();
    void onProbeBackends();
    void onSeedDefaultProfiles();
    void onCreateProfile();
    void onDeleteProfile();
    void onRenameProfile();
private:
    void buildUi();
    void log(const QString& s);
private:
    QLineEdit* defaultProject_ = nullptr;
    QComboBox* backendMode_ = nullptr;
    ProgressLogPane* progressLog_ = nullptr;
    QComboBox* profilesList_ = nullptr;
};
