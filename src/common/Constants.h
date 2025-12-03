/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * Constants.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：全局常量定义
 * 描述：定义系统级常量：默认超时时间、缓冲区大小、文件路径、版本号等
 *
 * 当前版本：1.0
 */

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
