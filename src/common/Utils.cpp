#include "Utils.h"
#include <QDateTime>
#include <QDir>

QString Utils::timestampNow() {
  return QDateTime::currentDateTimeUtc().toString("yyyyMMdd_HHmmsszzz");
}

bool Utils::ensureDir(const QString &path) {
  QDir dir(path);
  if (dir.exists()) {
    return true;
  }
  // TODO: 权限/错误处理
  return dir.mkpath(".");
}

QString Utils::joinPath(const QString &lhs, const QString &rhs) {
  if (lhs.isEmpty()) {
    return rhs;
  }
  if (rhs.isEmpty()) {
    return lhs;
  }
  QDir dir(lhs);
  return dir.filePath(rhs);
}
