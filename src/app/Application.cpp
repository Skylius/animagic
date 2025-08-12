#include <QIcon>
#include "Application.hpp"
#include "config/AppSettings.hpp"
#include "app/Theme.hpp"
Application::Application(int& argc, char** argv) : QApplication(argc, argv){
    setApplicationName("Animagic");
    setOrganizationName("animagic");
    setOrganizationDomain("animagic.local");
    AppSettings::instance().load();
    Theme::apply(AppSettings::instance().theme());
    setWindowIcon(QIcon(":/icons/animagic.png"));
}
