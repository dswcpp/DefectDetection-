/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * Utils.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：通用工具函数接口定义
 * 描述：提供常用工具函数：字符串处理、文件操作、时间格式化、UUID生成等
 *
 * 当前版本：1.0
 */

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
