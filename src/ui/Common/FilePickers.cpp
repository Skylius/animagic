#include "FilePickers.hpp"
#include <QDir>
namespace FilePickers {
QString& lastDir() { static QString s = QDir::homePath(); return s; }
} // namespace FilePickers
