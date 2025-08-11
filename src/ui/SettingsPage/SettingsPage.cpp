#include <QPushButton>
#include "SettingsPage.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>
#include "platform/Paths.hpp"
#include "config/AppSettings.hpp"
#include "ui/Common/FilePickers.hpp"
SettingsPage::SettingsPage(QWidget* parent) : QWidget(parent) { buildUi(); }
void SettingsPage::buildUi() {
    auto* lay = new QVBoxLayout(this);
    auto* gen = new QGroupBox("General", this);
    auto* form = new QFormLayout(gen);
    defaultProject_ = new QLineEdit(Platform::Paths::defaultProjectsDir("Animagic"), this);
    auto* btnChoose = new QPushButton("Browse…", this);
    auto* row = new QWidget(this);
    auto* rowLay = new QHBoxLayout(row); rowLay->setContentsMargins(0,0,0,0);
    rowLay->addWidget(defaultProject_, 1); rowLay->addWidget(btnChoose, 0);
    form->addRow("Default Project Folder:", row);
    auto* defaultCoalesce_ = new QCheckBox("Default: Coalesce frames on disassembly", this);
    form->addRow(QString(), defaultCoalesce_);
    // Appearance
    auto* appBox = new QGroupBox("Appearance", this);
    auto* aform = new QFormLayout(appBox);
    auto* themeCombo_ = new QComboBox(this);
    themeCombo_->addItems({"system","light","dark"});
    aform->addRow("Theme:", themeCombo_);

    auto* backendBox = new QGroupBox("Backend", this);
    auto* bform = new QFormLayout(backendBox);
    backendMode_ = new QComboBox(this);
    backendMode_->addItem("Auto", (int)MagickBackend::Mode::Auto);
    backendMode_->addItem("Magick++", (int)MagickBackend::Mode::MagickPP);
    backendMode_->addItem("CLI", (int)MagickBackend::Mode::CLI);
    auto* btnProbe = new QPushButton("Probe CLI availability", this);
    bform->addRow("ImageMagick backend:", backendMode_);
    bform->addRow(btnProbe);
    auto* profBox = new QGroupBox("Profiles", this);
    auto* pform = new QHBoxLayout(profBox);
    profilesList_ = new QComboBox(this);
    profilesList_->addItems(Profiles::Store::list());
    auto* btnSeed = new QPushButton("Seed defaults", this);
    auto* btnCreate = new QPushButton("Create…", this);
    auto* btnDelete = new QPushButton("Delete", this);
    auto* btnRename = new QPushButton("Rename…", this);
    pform->addWidget(profilesList_, 1);
    pform->addWidget(btnSeed); pform->addWidget(btnCreate); pform->addWidget(btnDelete); pform->addWidget(btnRename);
    progressLog_ = new ProgressLogPane(this);
    lay->addWidget(gen); lay->addWidget(backendBox); lay->addWidget(profBox); lay->addWidget(logEdit_, 1);
    connect(btnChoose, &QPushButton::clicked, this, &SettingsPage::onChooseDefaultProject);
    connect(btnProbe, &QPushButton::clicked, this, &SettingsPage::onProbeBackends);
    connect(btnSeed, &QPushButton::clicked, this, &SettingsPage::onSeedDefaultProfiles);
    connect(btnCreate, &QPushButton::clicked, this, &SettingsPage::onCreateProfile);
    connect(btnDelete, &QPushButton::clicked, this, &SettingsPage::onDeleteProfile);
    connect(btnRename, &QPushButton::clicked, this, &SettingsPage::onRenameProfile);

    // Load settings and apply to UI
    {
        QString err;
        AppSettings::instance().load(&err); // first run writes defaults
        defaultProject_->setText(AppSettings::instance().lastProjectFolder());
        // Select backend mode
        const int mode = AppSettings::instance().backendMode();
        int idx = backendMode_->findData(mode);
        if (idx >= 0) backendMode_->setCurrentIndex(idx);
    }

    // Persist changes on user actions
    connect(defaultProject_, &QLineEdit::editingFinished, this, [this](){
        AppSettings::instance().setLastProjectFolder(defaultProject_->text());
        QString err; AppSettings::instance().save(&err);
    });
    connect(backendMode_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int idx){
        int mode = backendMode_->itemData(idx).toInt();
        AppSettings::instance().setBackendMode(mode);
        QString err; AppSettings::instance().save(&err);
    });

    /* SETTINGS_APPLIED */
    // Apply additional settings
    // Theme
    int themeIdx = themeCombo_->findText(AppSettings::instance().theme());
    if (themeIdx >= 0) themeCombo_->setCurrentIndex(themeIdx);
    // Default coalesce
    defaultCoalesce_->setChecked(AppSettings::instance().defaultCoalesce());

    // Save on change
    connect(themeCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, themeCombo_](){
        AppSettings::instance().setTheme(themeCombo_->currentText());
        QString err; AppSettings::instance().save(&err);
        Theme::apply(AppSettings::instance().theme());
    });
    connect(defaultCoalesce_, &QCheckBox::toggled, this, [this](bool on){
        AppSettings::instance().setDefaultCoalesce(on);
        QString err; AppSettings::instance().save(&err);
    });

}
void SettingsPage::log(const QString& s) { if (progressLog_) progressLog_->log(s); }
void SettingsPage::onChooseDefaultProject() {
    QString d = FilePickers::getOpenFolder(this, "Choose Default Project Folder");
    if (!d.isEmpty()) defaultProject_->setText(d);
}
void SettingsPage::onProbeBackends() {
    auto a = Validators::detectImageMagickCLI();
    if (a.cli) log(QString("CLI available: %1").arg(a.cliCommand)); else log("CLI not found. Only Magick++ backend usable.");
}
void SettingsPage::onSeedDefaultProfiles() {
    auto defaults = Profiles::Store::defaultProfiles();
    int created = 0;
    for (const auto& p : defaults) { QString err; if (Profiles::Store::save(p, &err)) ++created; else log(QString("Profile save failed: %1").arg(err)); }
    profilesList_->clear(); profilesList_->addItems(Profiles::Store::list());
    log(QString("Seeded %1 default profiles.").arg(created));
}
void SettingsPage::onCreateProfile() {
    bool ok = false;
    QString name = QInputDialog::getText(this, "Create Profile", "Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;
    Profiles::Profile p; p.name = name;
    p.prefs.targetFormat = "gif"; p.prefs.loop = 0; p.prefs.delayMs = 100;
    p.prefs.gif.optimize = true; p.prefs.gif.dither = true; p.prefs.gif.colors = 256;
    QString err; if (!Profiles::Store::save(p, &err)) { log(QString("Create failed: %1").arg(err)); return; }
    profilesList_->clear(); profilesList_->addItems(Profiles::Store::list());
    log(QString("Created profile: %1").arg(name));
}
void SettingsPage::onDeleteProfile() {
    QString name = profilesList_->currentText(); if (name.isEmpty()) return;
    QString err; if (!Profiles::Store::remove(name, &err)) { log(QString("Delete failed: %1").arg(err)); return; }
    profilesList_->clear(); profilesList_->addItems(Profiles::Store::list());
    log(QString("Deleted profile: %1").arg(name));
}
void SettingsPage::onRenameProfile() {
    QString old = profilesList_->currentText(); if (old.isEmpty()) return;
    bool ok = false; QString nu = QInputDialog::getText(this, "Rename Profile", "New name:", QLineEdit::Normal, old, &ok);
    if (!ok || nu.isEmpty() || nu == old) return;
    QString err; if (!Profiles::Store::rename(old, nu, &err)) { log(QString("Rename failed: %1").arg(err)); return; }
    profilesList_->clear(); profilesList_->addItems(Profiles::Store::list());
    log(QString("Renamed profile: %1 → %2").arg(old, nu));
}
