#pragma once
#include <QApplication>
#include <QIcon>
#include "config/AppSettings.h"
#include "Theme.h"

// Application: central bootstrap for settings, theme, and org/app identity.
class Application : public QApplication {
public:
    Application(int& argc, char** argv) : QApplication(argc, argv) {
        setOrganizationName("animagic");
        setOrganizationDomain("example.animagic"); // placeholder
        setApplicationName("Animagic");

        // Load settings and apply theme early
        AppSettings::instance().load();
        Theme::apply(AppSettings::instance().theme());
        setWindowIcon(QIcon(":/icons/animagic.png"));
    }
};
