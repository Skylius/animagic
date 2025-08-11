#pragma once
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <optional>
#include <vector>

#include "ProjectMetadata.hpp"

// Profiles are named presets for AssemblyPrefs (target format + settings).
// Stored as JSON files under ~/.config/animagic/profiles/*.json on Linux.

struct Profile {
    QString name;
    AssemblyPrefs prefs;

    QJsonObject toJson() const;
    static Profile fromJson(const QJsonObject& obj);
};

class ProfileManager {
public:
    ProfileManager();

    // Directory where profiles are stored.
    QString profilesDir() const;

    // Load all profiles from storage.
    std::vector<Profile> loadAll(QString* errorOut = nullptr) const;

    // Save or overwrite a profile.
    bool saveProfile(const Profile& profile, QString* errorOut = nullptr) const;

    // Delete a profile by name.
    bool deleteProfile(const QString& name, QString* errorOut = nullptr) const;

    // Load a single profile by name.
    std::optional<Profile> loadProfile(const QString& name, QString* errorOut = nullptr) const;

private:
    QString dir_;
};
