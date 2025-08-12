#pragma once
#include <QMainWindow>
class TaskBar;
class DisassemblePage;
class AssemblePage;
class MainWindow : public QMainWindow{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent=nullptr);
private:
    TaskBar* taskBar_;
};
