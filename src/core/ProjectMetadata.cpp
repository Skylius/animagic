#include "ProjectMetadata.hpp"
#include <QJsonDocument>
#include <QFile>
#include <QDateTime>

using namespace MetaUtil;

// -------------------- MetaUtil --------------------
QJsonArray MetaUtil::intVectorToJson(const std::vector<int>& v) {
    QJsonArray a;
    for (int x : v) a.append(x);
    return a;
}
std::vector<int> MetaUtil::intVectorFromJson(const QJsonArray& a) {
    std::vector<int> v; v.reserve(a.size());
    for (auto it = a.begin(); it != a.end(); ++it) v.push_back((*it).toInt());
    return v;
}
QJsonArray MetaUtil::stringVectorToJson(const std::vector<QString>& v) {
    QJsonArray a;
    for (const auto& s : v) a.append(s);
    return a;
}
std::vector<QString> MetaUtil::stringVectorFromJson(const QJsonArray& a) {
    std::vector<QString> v; v.reserve(a.size());
    for (auto it = a.begin(); it != a.end(); ++it) v.push_back((*it).toString());
    return v;
}

QJsonObject MetaUtil::toJson(const AssemblyGifOptions& o) {
    return QJsonObject{
        {"optimize", o.optimize},
        {"dither", o.dither},
        {"colors", o.colors}
    };
}
QJsonObject MetaUtil::toJson(const AssemblyWebpOptions& o) {
    return QJsonObject{
        {"lossless", o.lossless},
        {"quality", o.quality},
        {"method", o.method},
        {"near_lossless", o.nearLossless},
        {"alpha_quality", o.alphaQuality}
    };
}
QJsonObject MetaUtil::toJson(const AssemblyPrefs& p) {
    QJsonObject o{
        {"target_format", p.targetFormat},
        {"loop", p.loop}
    };
    if (p.delayMs.has_value()) o["delay_ms"] = *p.delayMs;
    else o["delay_ms"] = QJsonValue(); // null
    if (p.fps.has_value()) o["fps"] = *p.fps;
    else o["fps"] = QJsonValue(); // null
    o["gif"] = toJson(p.gif);
    o["webp"] = toJson(p.webp);
    return o;
}
QJsonObject MetaUtil::toJson(const StillsInfo& s) {
    return QJsonObject{
        {"folder", s.folder},
        {"pattern", s.pattern},
        {"format", s.format}
    };
}
QJsonObject MetaUtil::toJson(const SourceInfo& s) {
    QJsonObject o{
        {"path", s.path},
        {"format", s.format},
        {"width", s.width},
        {"height", s.height},
        {"frames", s.frames},
        {"loop", s.loop},
        {"per_frame_delay_ms", intVectorToJson(s.perFrameDelayMs)},
        {"disposal_modes", stringVectorToJson(s.disposalModes)}
    };
    return o;
}

AssemblyGifOptions MetaUtil::gifFrom(const QJsonObject& o) {
    AssemblyGifOptions g;
    if (o.contains("optimize")) g.optimize = o["optimize"].toBool(g.optimize);
    if (o.contains("dither")) g.dither = o["dither"].toBool(g.dither);
    if (o.contains("colors")) g.colors = o["colors"].toInt(g.colors);
    return g;
}

AssemblyWebpOptions MetaUtil::webpFrom(const QJsonObject& o) {
    AssemblyWebpOptions w;
    if (o.contains("lossless")) w.lossless = o["lossless"].toBool(w.lossless);
    if (o.contains("quality")) w.quality = o["quality"].toInt(w.quality);
    if (o.contains("method")) w.method = o["method"].toInt(w.method);
    if (o.contains("near_lossless")) w.nearLossless = o["near_lossless"].toInt(w.nearLossless);
    if (o.contains("alpha_quality")) w.alphaQuality = o["alpha_quality"].toInt(w.alphaQuality);
    return w;
}

AssemblyPrefs MetaUtil::prefsFrom(const QJsonObject& o) {
    AssemblyPrefs p;
    if (o.contains("target_format")) p.targetFormat = o["target_format"].toString(p.targetFormat);
    if (o.contains("loop")) p.loop = o["loop"].toInt(p.loop);
    if (o.contains("delay_ms") && !o["delay_ms"].isNull()) p.delayMs = o["delay_ms"].toInt();
    if (o.contains("fps") && !o["fps"].isNull()) p.fps = o["fps"].toDouble();
    if (o.contains("gif") && o["gif"].isObject()) p.gif = gifFrom(o["gif"].toObject());
    if (o.contains("webp") && o["webp"].isObject()) p.webp = webpFrom(o["webp"].toObject());
    return p;
}

StillsInfo MetaUtil::stillsFrom(const QJsonObject& o) {
    StillsInfo s;
    if (o.contains("folder")) s.folder = o["folder"].toString();
    if (o.contains("pattern")) s.pattern = o["pattern"].toString();
    if (o.contains("format")) s.format = o["format"].toString(s.format);
    return s;
}

SourceInfo MetaUtil::sourceFrom(const QJsonObject& o) {
    SourceInfo s;
    if (o.contains("path")) s.path = o["path"].toString();
    if (o.contains("format")) s.format = o["format"].toString();
    if (o.contains("width")) s.width = o["width"].toInt();
    if (o.contains("height")) s.height = o["height"].toInt();
    if (o.contains("frames")) s.frames = o["frames"].toInt();
    if (o.contains("loop")) s.loop = o["loop"].toInt();
    if (o.contains("per_frame_delay_ms") && o["per_frame_delay_ms"].isArray()) {
        s.perFrameDelayMs = intVectorFromJson(o["per_frame_delay_ms"].toArray());
    }
    if (o.contains("disposal_modes") && o["disposal_modes"].isArray()) {
        s.disposalModes = stringVectorFromJson(o["disposal_modes"].toArray());
    }
    return s;
}

// -------------------- ProjectMetadata --------------------
QJsonObject ProjectMetadata::toJson() const {
    QJsonObject root;
    root["source"] = MetaUtil::toJson(source);
    root["assembly_prefs"] = MetaUtil::toJson(assembly);
    root["stills"] = MetaUtil::toJson(stills);
    root["created_at"] = createdAt.isEmpty()
        ? QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
        : createdAt;
    root["version"] = version;
    return root;
}

ProjectMetadata ProjectMetadata::fromJson(const QJsonObject& obj) {
    ProjectMetadata md;
    if (obj.contains("source") && obj["source"].isObject()) md.source = MetaUtil::sourceFrom(obj["source"].toObject());
    if (obj.contains("assembly_prefs") && obj["assembly_prefs"].isObject()) md.assembly = MetaUtil::prefsFrom(obj["assembly_prefs"].toObject());
    if (obj.contains("stills") && obj["stills"].isObject()) md.stills = MetaUtil::stillsFrom(obj["stills"].toObject());
    if (obj.contains("created_at")) md.createdAt = obj["created_at"].toString();
    if (obj.contains("version")) md.version = obj["version"].toInt(1);
    return md;
}

bool ProjectMetadata::saveToFile(const QString& filePath, const ProjectMetadata& md, QString* errorOut) {
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorOut) *errorOut = QString("Failed to open for write: %1").arg(filePath);
        return false;
    }
    QJsonDocument doc(md.toJson());
    auto data = doc.toJson(QJsonDocument::Indented);
    if (f.write(data) != data.size()) {
        if (errorOut) *errorOut = QString("Failed to write all data to: %1").arg(filePath);
        return false;
    }
    return true;
}

std::optional<ProjectMetadata> ProjectMetadata::loadFromFile(const QString& filePath, QString* errorOut) {
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        if (errorOut) *errorOut = QString("Failed to open for read: %1").arg(filePath);
        return std::nullopt;
    }
    auto bytes = f.readAll();
    QJsonParseError perr{};
    auto doc = QJsonDocument::fromJson(bytes, &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorOut) *errorOut = QString("JSON parse error at offset %1: %2").arg(perr.offset).arg(perr.errorString());
        return std::nullopt;
    }
    return ProjectMetadata::fromJson(doc.object());
}
