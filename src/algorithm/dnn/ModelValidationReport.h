/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ModelValidationReport.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：模型验证报告类定义
 * 描述：存储模型验证结果的数据结构，包含验证状态、错误信息、
 *       性能指标等
 *
 * 当前版本：1.0
 */

#ifndef MODELVALIDATIONREPORT_H
#define MODELVALIDATIONREPORT_H
#include <QString>
#include <QStringList>
#include <opencv2/core.hpp>
struct ModelValidationReport {

  bool ok = false;

  QStringList errors;

  QStringList warnings;

  double warmupMs = 0.0;

  double singleRunMs = 0.0;

  QString backend;

  QString target;

  int opset = -1;

  cv::Size inputSize{640, 640};

  int inputChannels = 3;

  int numClasses = -1;

};

#endif // MODELVALIDATIONREPORT_H
