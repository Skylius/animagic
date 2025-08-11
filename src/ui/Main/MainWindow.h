#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QStatusBar>
#include "core/TaskRunner.hpp"
#include "ui/Common/TaskBar.hpp"
#include "ui/ProjectBrowser/ProjectBrowser.hpp"
#include "ui/DisassemblePage/DisassemblePage.hpp"
#include "ui/AssemblePage/AssemblePage.hpp"
#include "ui/MetadataPage/MetadataPage.hpp"
#include "ui/SettingsPage/SettingsPage.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void buildUi();
    void wireGlobalTaskBar();

private:
    QTabWidget* tabs_ = nullptr;
    TaskRunner* runner_ = nullptr;
    TaskBar* taskBar_ = nullptr;
    QComboBox* themeCombo_ = nullptr;
};
