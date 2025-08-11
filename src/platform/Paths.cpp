#include "Paths.hpp"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

namespace Platform::Paths {
static QString norm(const QString& p) {
    QString s = QDir::fromNativeSeparators(p);
    if (s.size() > 1 && s.endsWith('/')) s.chop(1);
    return QDir::toNativeSeparators(s);
}
QString home() { return norm(QDir::homePath()); }
QString temp() { return norm(QDir::tempPath()); }
QString configDir(const QString& appName, bool ensure) {
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (base.isEmpty()) base = QDir::home().filePath(".config/" + appName.toLower());
    if (ensure) ensureDir(base);
    return norm(base);
}
QString dataDir(const QString& appName, bool ensure) {
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty()) base = QDir::home().filePath(".local/share/" + appName.toLower());
    if (ensure) ensureDir(base);
    return norm(base);
}
QString cacheDir(const QString& appName, bool ensure) {
    QString base = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (base.isEmpty()) base = QDir::home().filePath(".cache/" + appName.toLower());
    if (ensure) ensureDir(base);
    return norm(base);
}
QString defaultProjectsDir(const QString& appName, bool ensure) {
    QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString d = docs.isEmpty() ? QDir::home().filePath(appName) : QDir(docs).filePath(appName);
    if (ensure) ensureDir(d);
    return norm(d);
}
QString join(const QStringList& parts) {
    if (parts.isEmpty()) return QString();
    QDir d(parts.first());
    for (int i=1;i<parts.size();++i) d = QDir(d.filePath(parts[i]));
    return norm(d.absolutePath());
}
bool ensureDir(const QString& path) {
    if (path.isEmpty()) return false;
    QDir d(path);
    if (d.exists()) return true;
    return QDir().mkpath(path);
}
QString normalize(const QString& path) { return norm(path); }
} // namespace Platform::Paths
