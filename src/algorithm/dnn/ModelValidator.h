/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ModelValidator.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：模型验证模块接口定义
 * 描述：模型质量验证工具，检查模型格式、输入输出维度、
 *       推理速度等指标
 *
 * 当前版本：1.0
 */

#ifndef MODELVALIDATOR_H
#define MODELVALIDATOR_H

#include "ModelValidationReport.h"
#include <QFileInfo>

// ModelValidator.h

#include <QString>
#include "ModelValidationReport.h"

class ModelValidator {
public:
  struct Expectation {
    cv::Size inputSize = {640, 640};
    int inputChannels = 3;          // BGR
    int minOutputDims = 3;          // e.g. [N, dets, 5+C]
    int expectedClasses = -1;       // 不校验则置 -1
    double maxWarmupMs = 200.0;     // 基于设备标定
    double maxSingleRunMs = 50.0;   // 基于设备标定
    bool requireNCHW = true;
  };

  // 基于 ONNX 文件路径的完整校验
  ModelValidationReport validateONNX(const QString& onnxPath,
                                     const Expectation& exp,
                                     bool tryCUDA = false);
};

#endif // MODELVALIDATOR_H
