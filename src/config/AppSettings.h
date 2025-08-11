#pragma once
#include <QString>
#include <QVariant>
#include <QMap>
#include <QMutex>
#include "Defaults.hpp"

// AppSettings — central settings manager (thread-safe, singleton)
class AppSettings {
public:
    static AppSettings& instance();

    void load();
    void save() const;

    QVariant get(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void set(const QString& key, const QVariant& value);

    // Typed helpers
    QString lastProjectDir() const;
    void setLastProjectDir(const QString& path);

    QString theme() const;
    void setTheme(const QString& theme);

    QVariant gifSettings() const;
    void setGifSettings(const QVariant& v);

    QVariant webpSettings() const;
    void setWebpSettings(const QVariant& v);

private:
    AppSettings();
    QString settingsFilePath() const;

private:
    mutable QMutex mutex_;
    QMap<QString, QVariant> values_;
};
