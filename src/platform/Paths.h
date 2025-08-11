#pragma once
#include <QString>
#include <QStringList>

namespace Platform::Paths {
QString home();
QString temp();
QString configDir(const QString& appName, bool ensure=true);
QString dataDir(const QString& appName, bool ensure=true);
QString cacheDir(const QString& appName, bool ensure=true);
QString defaultProjectsDir(const QString& appName="Animagic", bool ensure=true);
QString join(const QStringList& parts);
bool ensureDir(const QString& path);
QString normalize(const QString& path);
} // namespace Platform::Paths
