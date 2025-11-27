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
