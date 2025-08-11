#pragma once
#include <QString>
#include <QStringList>
#include <QFileDialog>
namespace FilePickers {
QString& lastDir();
inline QString getOpenAnimation(QWidget* parent) {
    QString f = QFileDialog::getOpenFileName(parent, "Open Animation", lastDir(), "Animations (*.gif *.webp)");
    if (!f.isEmpty()) lastDir() = QFileInfo(f).absolutePath();
    return f;
}
inline QStringList getOpenImages(QWidget* parent, bool allowWebP = true) {
    QString filter = allowWebP ? "Images (*.png *.jpg *.jpeg *.tif *.tiff *.bmp *.webp)"
                               : "Images (*.png *.jpg *.jpeg *.tif *.tiff *.bmp)";
    QStringList files = QFileDialog::getOpenFileNames(parent, "Open Images", lastDir(), filter);
    if (!files.isEmpty()) lastDir() = QFileInfo(files.first()).absolutePath();
    return files;
}
inline QString getOpenJson(QWidget* parent) {
    QString f = QFileDialog::getOpenFileName(parent, "Open JSON", lastDir(), "JSON (*.json)");
    if (!f.isEmpty()) lastDir() = QFileInfo(f).absolutePath();
    return f;
}
inline QString getSaveAnimation(QWidget* parent, const QString& target) {
    QString filter = (target.toLower() == "gif") ? "GIF (*.gif)" : "WebP (*.webp)";
    QString f = QFileDialog::getSaveFileName(parent, "Save Animation", lastDir(), filter);
    if (!f.isEmpty()) lastDir() = QFileInfo(f).absolutePath();
    return f;
}
inline QString getSaveJson(QWidget* parent) {
    QString f = QFileDialog::getSaveFileName(parent, "Save JSON", lastDir(), "JSON (*.json)");
    if (!f.isEmpty()) lastDir() = QFileInfo(f).absolutePath();
    return f;
}
inline QString getOpenFolder(QWidget* parent, const QString& title = "Choose Folder") {
    QString d = QFileDialog::getExistingDirectory(parent, title, lastDir());
    if (!d.isEmpty()) lastDir() = d;
    return d;
}
} // namespace FilePickers
