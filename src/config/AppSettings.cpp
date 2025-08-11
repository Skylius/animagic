#include "AppSettings.hpp"
#include "platform/Paths.hpp"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

AppSettings& AppSettings::instance() {
    static AppSettings inst;
    return inst;
}

AppSettings::AppSettings() {
    load();
}

QString AppSettings::settingsFilePath() const {
    return Platform::Paths::join({ Platform::Paths::configDir("Animagic"), "settings.json" });
}

void AppSettings::load() {
    QMutexLocker lock(&mutex_);
    values_.clear();
    QFile f(settingsFilePath());
    if (f.exists() && f.open(QIODevice::ReadOnly)) {
        auto doc = QJsonDocument::fromJson(f.readAll());
        if (doc.isObject()) {
            auto obj = doc.object();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                values_[it.key()] = it.value().toVariant();
            }
        }
    } else {
        // defaults
        values_["lastProjectDir"] = Defaults::defaultProjectsDir();
        values_["theme"] = Defaults::theme();
        values_["gifSettings"] = Defaults::gifSettings();
        values_["webpSettings"] = Defaults::webpSettings();
        save();
    }
}

void AppSettings::save() const {
    QMutexLocker lock(&mutex_);
    QJsonObject obj;
    for (auto it = values_.begin(); it != values_.end(); ++it) {
        obj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    QJsonDocument doc(obj);
    QFile f(settingsFilePath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(doc.toJson(QJsonDocument::Indented));
    }
}

QVariant AppSettings::get(const QString& key, const QVariant& defaultValue) const {
    QMutexLocker lock(&mutex_);
    return values_.value(key, defaultValue);
}

void AppSettings::set(const QString& key, const QVariant& value) {
    {
        QMutexLocker lock(&mutex_);
        values_[key] = value;
    }
    save();
}

QString AppSettings::lastProjectDir() const {
    return get("lastProjectDir", Defaults::defaultProjectsDir()).toString();
}
void AppSettings::setLastProjectDir(const QString& path) { set("lastProjectDir", path); }

QString AppSettings::theme() const {
    return get("theme", Defaults::theme()).toString();
}
void AppSettings::setTheme(const QString& theme) { set("theme", theme); }

QVariant AppSettings::gifSettings() const {
    return get("gifSettings", Defaults::gifSettings());
}
void AppSettings::setGifSettings(const QVariant& v) { set("gifSettings", v); }

QVariant AppSettings::webpSettings() const {
    return get("webpSettings", Defaults::webpSettings());
}
void AppSettings::setWebpSettings(const QVariant& v) { set("webpSettings", v); }
