#pragma once
#include <QString>
#include <optional>

namespace SpaceCheck {
std::optional<quint64> bytesFree(const QString& path);
bool hasAtLeast(const QString& path, quint64 requireBytes, quint64* availableOut = nullptr);
} // namespace SpaceCheck
