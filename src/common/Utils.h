#ifndef UTILS_H
#define UTILS_H

#include <QString>

class Utils {
public:
  // 获取当前时间戳（字符串）
  static QString timestampNow();

  // 确保目录存在
  static bool ensureDir(const QString &path);

  // 拼路径，自动添加分隔符
  static QString joinPath(const QString &lhs, const QString &rhs);
};

#endif // UTILS_H
