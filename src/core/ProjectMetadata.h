#pragma once
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <optional>
#include <vector>

// ProjectMetadata models the animagic.meta.json file and provides (de)serialization.

struct AssemblyGifOptions {
    bool optimize = true;
    bool dither = true;
    int colors = 256;
};

struct AssemblyWebpOptions {
    bool lossless = false;
    int quality = 85;        // 0..100
    int method = 4;          // 0..6
    int nearLossless = 0;    // 0..100
    int alphaQuality = 100;  // 0..100
};

struct AssemblyPrefs {
    QString targetFormat = "gif";               // "gif" or "webp"
    int loop = 0;                               // 0 = infinite
    std::optional<int> delayMs;                 // global delay in ms
    std::optional<double> fps;                  // alternative to delay
    AssemblyGifOptions gif;
    AssemblyWebpOptions webp;
};

struct StillsInfo {
    QString folder;            // e.g. "/projects/intro/frames"
    QString pattern;           // e.g. "frame_%04d.png"
    QString format = "png";    // png/jpeg/...
};

struct SourceInfo {
    QString path;
    QString format;            // "gif" / "webp"
    int width = 0;
    int height = 0;
    int frames = 0;
    int loop = 0;
    std::vector<int> perFrameDelayMs;
    std::vector<QString> disposalModes; // as strings for portability
};

struct ProjectMetadata {
    SourceInfo source;
    AssemblyPrefs assembly;
    StillsInfo stills;
    QString createdAt; // ISO-8601 string
    int version = 1;

    QJsonObject toJson() const;
    static ProjectMetadata fromJson(const QJsonObject& obj);

    static bool saveToFile(const QString& filePath, const ProjectMetadata& md, QString* errorOut = nullptr);
    static std::optional<ProjectMetadata> loadFromFile(const QString& filePath, QString* errorOut = nullptr);
};

// helpers to map simple types
namespace MetaUtil {
    QJsonObject toJson(const AssemblyGifOptions& o);
    QJsonObject toJson(const AssemblyWebpOptions& o);
    QJsonObject toJson(const AssemblyPrefs& p);
    QJsonObject toJson(const StillsInfo& s);
    QJsonObject toJson(const SourceInfo& s);

    AssemblyGifOptions gifFrom(const QJsonObject& o);
    AssemblyWebpOptions webpFrom(const QJsonObject& o);
    AssemblyPrefs prefsFrom(const QJsonObject& o);
    StillsInfo stillsFrom(const QJsonObject& o);
    SourceInfo sourceFrom(const QJsonObject& o);

    QJsonArray intVectorToJson(const std::vector<int>& v);
    std::vector<int> intVectorFromJson(const QJsonArray& a);
    QJsonArray stringVectorToJson(const std::vector<QString>& v);
    std::vector<QString> stringVectorFromJson(const QJsonArray& a);
}
