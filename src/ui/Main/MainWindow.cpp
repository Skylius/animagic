#include "MainWindow.hpp"
#include <QVBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QActionGroup>
#include <QIcon>
#include <QComboBox>
#include "app/Theme.hpp"
#include "config/AppSettings.hpp"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    runner_ = new TaskRunner(this);
    buildUi();
    wireGlobalTaskBar();
}

void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    auto* v = new QVBoxLayout(central);
    v->setContentsMargins(0,0,0,0);
    v->setSpacing(0);

    // Menu: View → Theme
    auto* viewMenu = menuBar()->addMenu("&View");
    auto* themeMenu = viewMenu->addMenu("Theme");
    QAction* actSys = themeMenu->addAction("System");
    QAction* actLight = themeMenu->addAction("Light");
    QAction* actDark = themeMenu->addAction("Dark");
    actSys->setCheckable(true); actLight->setCheckable(true); actDark->setCheckable(true);
    QActionGroup* g = new QActionGroup(this); g->setExclusive(true);
    g->addAction(actSys); g->addAction(actLight); g->addAction(actDark);
    const QString curTheme = AppSettings::instance().theme();
    if (curTheme == "dark") actDark->setChecked(true);
    else if (curTheme == "light") actLight->setChecked(true);
    else actSys->setChecked(true);
    auto applyTheme = [](const QString& t){ AppSettings::instance().setTheme(t); QString err; AppSettings::instance().save(&err); Theme::apply(t); };
    connect(actSys, &QAction::triggered, this, [applyTheme,this]{ applyTheme("system"); if (themeCombo_) themeCombo_->setCurrentText("system"); });
    connect(actLight, &QAction::triggered, this, [applyTheme,this]{ applyTheme("light"); if (themeCombo_) themeCombo_->setCurrentText("light"); });
    connect(actDark, &QAction::triggered, this, [applyTheme,this]{ applyTheme("dark"); if (themeCombo_) themeCombo_->setCurrentText("dark"); });
    /* THEME_MENU_ADDED */

    tabs_ = new QTabWidget(central);
    tabs_->setDocumentMode(true);
    tabs_->setTabsClosable(false);

    // Create pages. Disassemble/Assemble receive the shared TaskRunner
    auto* browser = new ProjectBrowser(central);
    auto* dis = new DisassemblePage(runner_, central);
    auto* asmbl = new AssemblePage(runner_, central);
    auto* meta = new MetadataPage(central);
    auto* settings = new SettingsPage(central);

    tabs_->addTab(browser, "Project");
    tabs_->addTab(dis, "Disassemble");
    tabs_->addTab(asmbl, "Assemble");
    tabs_->addTab(meta, "Metadata");
    tabs_->addTab(settings, "Settings");

    taskBar_ = new TaskBar(central);
    taskBar_->setVisible(false);

    v->addWidget(tabs_, 1);
    v->addWidget(taskBar_, 0);

    setCentralWidget(central);
    setWindowTitle("Animagic");
    setWindowIcon(QIcon(":/icons/animagic.png"));
    resize(1100, 720);

    themeCombo_ = new QComboBox(this);
    themeCombo_->addItems({"system","light","dark"});
    themeCombo_->setCurrentText(AppSettings::instance().theme());
    statusBar()->addPermanentWidget(themeCombo_);
    connect(themeCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]{
        const QString t = themeCombo_->currentText();
        AppSettings::instance().setTheme(t);
        QString err; AppSettings::instance().save(&err);
        Theme::apply(t);
    });
    /* THEME_STATUSBAR */
}

void MainWindow::wireGlobalTaskBar() {
    connect(taskBar_, &TaskBar::cancelRequested, runner_, &TaskRunner::cancelCurrent);
    connect(runner_, &TaskRunner::taskStarted, this, [this](const QString& name){ taskBar_->setActive(name); });
    connect(runner_, &TaskRunner::taskProgress, this, [this](int d,int t){ taskBar_->setProgress(d,t); });
    connect(runner_, &TaskRunner::taskSucceeded, this, [this](const QString&){ taskBar_->setIdle(); });
    connect(runner_, &TaskRunner::taskFailed, this, [this](const QString&, const QString&){ taskBar_->setIdle(); });
    connect(runner_, &TaskRunner::taskCancelled, this, [this](const QString&){ taskBar_->setIdle(); });
}
