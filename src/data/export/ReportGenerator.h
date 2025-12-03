/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ReportGenerator.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：报表生成模块接口定义
 * 描述：生成检测报告文档，支持HTML/PDF格式，包含统计图表、
 *       缺陷详情、趋势分析等内容
 *
 * 当前版本：1.0
 */

#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <QString>

class ReportGenerator {
public:
  ReportGenerator() = default;

  // TODO: 生成日报/周报/月报
  bool generate(const QString &path);
};

#endif // REPORTGENERATOR_H
