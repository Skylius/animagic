#include "Profiles.hpp"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>

using namespace MetaUtil; // reuse from ProjectMetadata

QJsonObject Profile::toJson() const {
    QJsonObject o;
    o["name"] = name;
    o["prefs"] = MetaUtil::toJson(prefs);
    return o;
}

Profile Profile::fromJson(const QJsonObject& obj) {
    Profile p;
    if (obj.contains("name")) p.name = obj["name"].toString();
    if (obj.contains("prefs") && obj["prefs"].isObject()) {
        p.prefs = MetaUtil::prefsFrom(obj["prefs"].toObject());
    }
    return p;
}

ProfileManager::ProfileManager() {
    QString configRoot = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (configRoot.isEmpty()) {
        configRoot = QDir::homePath() + "/.config/animagic";
    }
    dir_ = QDir(configRoot).filePath("profiles");
    QDir().mkpath(dir_);
}

QString ProfileManager::profilesDir() const {
    return dir_;
}

std::vector<Profile> ProfileManager::loadAll(QString* errorOut) const {
    std::vector<Profile> profiles;
    QDir d(dir_);
    if (!d.exists()) return profiles;
    auto files = d.entryList({"*.json"}, QDir::Files);
    for (const auto& f : files) {
        auto prof = loadProfile(f, errorOut);
        if (prof.has_value()) profiles.push_back(*prof);
    }
    return profiles;
}

bool ProfileManager::saveProfile(const Profile& profile, QString* errorOut) const {
    if (profile.name.trimmed().isEmpty()) {
        if (errorOut) *errorOut = "Profile name is empty.";
        return false;
    }
    QString filePath = QDir(dir_).filePath(profile.name + ".json");
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorOut) *errorOut = QString("Failed to open for write: %1").arg(filePath);
        return false;
    }
    QJsonDocument doc(profile.toJson());
    auto data = doc.toJson(QJsonDocument::Indented);
    if (f.write(data) != data.size()) {
        if (errorOut) *errorOut = QString("Failed to write profile: %1").arg(filePath);
        return false;
    }
    return true;
}

bool ProfileManager::deleteProfile(const QString& name, QString* errorOut) const {
    QString filePath = QDir(dir_).filePath(name + ".json");
    if (!QFile::exists(filePath)) {
        if (errorOut) *errorOut = "Profile not found.";
        return false;
    }
    if (!QFile::remove(filePath)) {
        if (errorOut) *errorOut = QString("Failed to delete profile: %1").arg(filePath);
        return false;
    }
    return true;
}

std::optional<Profile> ProfileManager::loadProfile(const QString& name, QString* errorOut) const {
    QString filePath = name;
    if (!QFileInfo(name).isAbsolute()) {
        filePath = QDir(dir_).filePath(name);
    }
    if (!filePath.endsWith(".json", Qt::CaseInsensitive)) filePath += ".json";
    QFile f(filePath);
    if (!f.exists()) {
        if (errorOut) *errorOut = QString("Profile file not found: %1").arg(filePath);
        return std::nullopt;
    }
    if (!f.open(QIODevice::ReadOnly)) {
        if (errorOut) *errorOut = QString("Failed to open profile: %1").arg(filePath);
        return std::nullopt;
    }
    QJsonParseError perr{};
    auto doc = QJsonDocument::fromJson(f.readAll(), &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorOut) *errorOut = QString("Parse error in %1: %2").arg(filePath, perr.errorString());
        return std::nullopt;
    }
    return Profile::fromJson(doc.object());
}
