#include "Defaults.hpp"
#include "platform/Paths.hpp"
#include <QVariantMap>

namespace Defaults {

QString defaultProjectsDir() {
    return Platform::Paths::defaultProjectsDir("Animagic");
}

QString theme() {
    return "system"; // could be "dark", "light", or "system"
}

QVariant gifSettings() {
    QVariantMap m;
    m["optimize"] = true;
    m["dither"] = true;
    m["colors"] = 256;
    return m;
}

QVariant webpSettings() {
    QVariantMap m;
    m["lossless"] = false;
    m["quality"] = 80;
    m["method"] = 4;
    m["nearLossless"] = 60;
    m["alphaQuality"] = 100;
    return m;
}

} // namespace Defaults
