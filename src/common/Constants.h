#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace Constants {
inline constexpr auto kAppName = "DefectDetection";
inline constexpr auto kDefaultConfig = "config/default.json";
inline constexpr auto kDefaultLogDir = "logs";
inline constexpr auto kDefaultDbPath = "data/inspection.db";

inline const QString &appName() {
  static const QString name = QString::fromUtf8(kAppName);
  return name;
}
} // namespace Constants

#endif // CONSTANTS_H
