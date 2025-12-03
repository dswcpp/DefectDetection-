/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * CSVExporter.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：CSV数据导出模块接口定义
 * 描述：将检测结果、缺陷记录导出为CSV格式文件，支持自定义列、
 *       编码格式、分隔符等
 *
 * 当前版本：1.0
 */

#ifndef CSVEXPORTER_H
#define CSVEXPORTER_H

#include <QString>

class CSVExporter {
public:
  CSVExporter() = default;

  // TODO: 导出数据到 CSV
  bool exportTo(const QString &path);
};

#endif // CSVEXPORTER_H
