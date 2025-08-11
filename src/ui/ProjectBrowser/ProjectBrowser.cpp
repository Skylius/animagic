#include "ProjectBrowser.hpp"
#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>
#include <QDebug>

ProjectBrowser::ProjectBrowser(QWidget* parent) : QWidget(parent) {
    buildUi();
    setProjectRoot(QDir::homePath());
}

void ProjectBrowser::buildUi() {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(4);

    toolbar_ = new QToolBar(this);
    actChooseRoot_ = toolbar_->addAction("Open Project Folder…");
    actRefresh_ = toolbar_->addAction("Refresh");

    toolbar_->addSeparator();
    filterCombo_ = new QComboBox(this);
    filterCombo_->addItem("All files", static_cast<int>(Filters::Mode::All));
    filterCombo_->addItem("Animations", static_cast<int>(Filters::Mode::Animations));
    filterCombo_->addItem("Stills", static_cast<int>(Filters::Mode::Stills));
    filterCombo_->addItem("Metadata", static_cast<int>(Filters::Mode::Metadata));
    filterCombo_->addItem("Frames only", static_cast<int>(Filters::Mode::FramesOnly));
    toolbar_->addWidget(filterCombo_);

    toolbar_->addSeparator();
    rootEdit_ = new QLineEdit(this);
    rootEdit_->setReadOnly(true);
    rootEdit_->setPlaceholderText("Project folder");
    rootEdit_->setMinimumWidth(220);
    toolbar_->addWidget(rootEdit_);

    lay->addWidget(toolbar_);

    fsModel_ = new QFileSystemModel(this);
    fsModel_->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
    fsModel_->setNameFilterDisables(false);

    proxy_ = new Filters::ProxyModel(this);
    proxy_->setSourceModel(fsModel_);
    proxy_->setRecursiveFilteringEnabled(true);

    tree_ = new QTreeView(this);
    tree_->setModel(proxy_);
    tree_->setHeaderHidden(false);
    tree_->setRootIsDecorated(true);
    tree_->setSortingEnabled(true);
    tree_->sortByColumn(0, Qt::AscendingOrder);
    tree_->header()->setStretchLastSection(true);
    tree_->setUniformRowHeights(true);

    lay->addWidget(tree_, 1);

    connect(actChooseRoot_, &QAction::triggered, this, &ProjectBrowser::chooseRoot);
    connect(actRefresh_, &QAction::triggered, this, &ProjectBrowser::onRefresh);
    connect(tree_, &QTreeView::activated, this, &ProjectBrowser::onActivated);
    connect(filterCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProjectBrowser::onFilterChanged);
}

void ProjectBrowser::rewireFilter() {
    auto mode = static_cast<Filters::Mode>(filterCombo_->currentData().toInt());
    proxy_->setMode(mode);
}

void ProjectBrowser::setProjectRoot(const QString& folder) {
    QModelIndex srcRoot = fsModel_->setRootPath(folder);
    QModelIndex proxyRoot = proxy_->mapFromSource(srcRoot);
    tree_->setRootIndex(proxyRoot);
    rootEdit_->setText(folder);
    emit projectFolderChosen(folder);
    onFilterChanged(filterCombo_->currentIndex());
}

QString ProjectBrowser::projectRoot() const {
    return rootEdit_->text();
}

void ProjectBrowser::chooseRoot() {
    QString dir = QFileDialog::getExistingDirectory(this, "Open Project Folder", projectRoot());
    if (dir.isEmpty()) return;
    setProjectRoot(dir);
}

void ProjectBrowser::onRefresh() {
    // QFileSystemModel updates automatically, but we can refresh root mapping and filters
    setProjectRoot(projectRoot());
}

void ProjectBrowser::onFilterChanged(int modeIndex) {
    Q_UNUSED(modeIndex);
    rewireFilter();
    // expand root for visibility
    tree_->expand(proxy_->index(0,0));
}

void ProjectBrowser::onActivated(const QModelIndex& idx) {
    if (!idx.isValid()) return;
    QModelIndex src = proxy_->mapToSource(idx);
    QFileInfo fi = fsModel_->fileInfo(src);
    if (fi.isDir()) {
        emit framesFolderChosen(fi.absoluteFilePath());
        return;
    }
    const QString path = fi.absoluteFilePath();
    const QString ext = fi.suffix().toLower();
    if (ext == "gif" || ext == "webp") emit animationChosen(path);
    if (ext == "json") emit metadataChosen(path);
    emit fileChosen(path);
}
