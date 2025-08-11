#include "app/Application.h"
#include "ui/Main/MainWindow.hpp"

int main(int argc, char** argv) {
    Application app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
