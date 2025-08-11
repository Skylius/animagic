#include "SpaceCheck.hpp"
#include <QFileInfo>
#include <QDir>
#include <QtGlobal>
#include <optional>
#if __has_include(<filesystem>)
  #include <filesystem>
  namespace fs = std::filesystem;
#else
  #include <experimental/filesystem>
  namespace fs = std::experimental::filesystem;
#endif
namespace SpaceCheck {
std::optional<quint64> bytesFree(const QString& path) {
    try {
        fs::path p = fs::path(path.toStdString());
        if (!fs::exists(p)) {
            p = fs::path(QFileInfo(path).absolutePath().toStdString());
        }
        auto sp = fs::space(p);
        return static_cast<quint64>(sp.available);
    } catch (...) {
        return std::nullopt;
    }
}
bool hasAtLeast(const QString& path, quint64 requireBytes, quint64* availableOut) {
    auto free = bytesFree(path);
    quint64 avail = free.value_or(0);
    if (availableOut) *availableOut = avail;
    return free.has_value() && avail >= requireBytes;
}
} // namespace SpaceCheck
