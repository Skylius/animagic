#pragma once
#include <QString>
#include <QSettings>

class AppSettings {
public:
    static AppSettings& instance(){
        static AppSettings s; return s;
    }
    void load(){
        QSettings set;
        theme_ = set.value("theme", "system").toString();
        lastInput_ = set.value("lastInput").toString();
        lastOutput_ = set.value("lastOutput").toString();
    }
    void save(){
        QSettings set;
        set.setValue("theme", theme_);
        set.setValue("lastInput", lastInput_);
        set.setValue("lastOutput", lastOutput_);
    }
    QString theme() const { return theme_; }
    void setTheme(const QString& t){ theme_ = t; }
    QString lastInput() const { return lastInput_; }
    void setLastInput(const QString& s){ lastInput_ = s; }
    QString lastOutput() const { return lastOutput_; }
    void setLastOutput(const QString& s){ lastOutput_ = s; }
private:
    QString theme_ = "system";
    QString lastInput_;
    QString lastOutput_;
};
