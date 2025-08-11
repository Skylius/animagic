#include "Theme.h"
#include <QApplication>
#include <QPalette>
#include <QColor>

namespace Theme {

static void applyLight() {
    qApp->setStyleSheet("");
    QPalette p;
    qApp->setPalette(p);
}

static void applyDark() {
    QPalette dark;
    dark.setColor(QPalette::Window, QColor(53,53,53));
    dark.setColor(QPalette::WindowText, Qt::white);
    dark.setColor(QPalette::Base, QColor(35,35,35));
    dark.setColor(QPalette::AlternateBase, QColor(53,53,53));
    dark.setColor(QPalette::ToolTipBase, Qt::white);
    dark.setColor(QPalette::ToolTipText, Qt::white);
    dark.setColor(QPalette::Text, Qt::white);
    dark.setColor(QPalette::Button, QColor(53,53,53));
    dark.setColor(QPalette::ButtonText, Qt::white);
    dark.setColor(QPalette::BrightText, Qt::red);
    dark.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
    dark.setColor(QPalette::HighlightedText, Qt::black);
    qApp->setPalette(dark);
    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a2a2a; border: 1px solid #444; }");
}

void apply(const QString& name) {
    const QString n = name.toLower();
    if (n == "dark") applyDark();
    else if (n == "light") applyLight();
    else {
        // "system" -> leave palette as default and clear any stylesheet
        applyLight(); // default to light, Qt will pick system palette on some platforms
    }
}

} // namespace Theme
