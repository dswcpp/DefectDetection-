/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ExcelExporter.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：Excel数据导出模块接口定义
 * 描述：将检测数据导出为Excel格式(.xlsx)，支持多Sheet、样式设置、
 *       图表生成等功能
 *
 * 当前版本：1.0
 */

#ifndef EXCELEXPORTER_H
#define EXCELEXPORTER_H

#include <QString>

class ExcelExporter {
public:
  ExcelExporter() = default;

  // TODO: 导出数据到 Excel (xlsx)
  bool exportTo(const QString &path);
};

#endif // EXCELEXPORTER_H
