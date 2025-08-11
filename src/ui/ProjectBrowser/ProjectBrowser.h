#pragma once
#include <QWidget>
#include <QFileSystemModel>
#include <QTreeView>
#include <QToolBar>
#include <QLineEdit>
#include <QComboBox>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDir>

#include "Filters.hpp"

// ProjectBrowser: a left-sidebar style browser rooted at a "Project Folder".
// It shows a tree of files with a quick filter combo and emits high-level signals
// when the user selects interesting items.
//
// Filters offered (via Filters::Mode):
//  - All Files
//  - Animations (gif/webp)
//  - Stills (png/jpg/jpeg/tiff/bmp/webp)
//  - Metadata (*.json; specifically animagic.meta.json)
//  - Frames Only (common stills)
//
// Signals:
//  - projectFolderChosen(QString) : when user changes the root
//  - animationChosen(QString)     : when a .gif/.webp is activated
//  - framesFolderChosen(QString)  : when user activates a folder under root
//  - metadataChosen(QString)      : when a *.json (metadata) is activated
//  - fileChosen(QString)          : generic file activation (any extension)

class ProjectBrowser : public QWidget {
    Q_OBJECT
public:
    explicit ProjectBrowser(QWidget* parent = nullptr);

    // Set/get root project folder
    void setProjectRoot(const QString& folder);
    QString projectRoot() const;

signals:
    void projectFolderChosen(const QString& folder);
    void animationChosen(const QString& file);
    void framesFolderChosen(const QString& folder);
    void metadataChosen(const QString& file);
    void fileChosen(const QString& file);

private slots:
    void chooseRoot();
    void onActivated(const QModelIndex& idx);
    void onFilterChanged(int modeIndex);
    void onRefresh();

private:
    void buildUi();
    void rewireFilter();

private:
    QFileSystemModel* fsModel_ = nullptr;
    Filters::ProxyModel* proxy_ = nullptr;
    QTreeView* tree_ = nullptr;
    QToolBar* toolbar_ = nullptr;
    QLineEdit* rootEdit_ = nullptr;
    QComboBox* filterCombo_ = nullptr;
    QAction* actChooseRoot_ = nullptr;
    QAction* actRefresh_ = nullptr;
};
