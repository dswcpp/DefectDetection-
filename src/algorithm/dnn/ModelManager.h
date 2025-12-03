/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ModelManager.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：模型管理模块接口定义
 * 描述：DNN模型管理器，负责模型文件的加载、缓存、版本管理、
 *       热更新等功能
 *
 * 当前版本：1.0
 */

#ifndef MODELMANAGER_H
#define MODELMANAGER_H
#include <QString>
#include <QMutex>
#include <QReadLocker>
#include "opencv2/dnn/dnn.hpp"
#include "ModelValidator.h"

static bool validateModelFile(const QString& onnx, QString *err, QStringList* warns) {

  ModelValidator v;

  ModelValidator::Expectation exp;

  exp.inputSize = {640, 640};

  exp.inputChannels = 3;

  exp.expectedClasses = 4;   // 与业务类别对齐：{"scratch","crack","foreign","dimension"}

  // 依据平台标定：以下数值以 i5 CPU 为例

  exp.maxWarmupMs = 300.0;

  exp.maxSingleRunMs = 60.0;

  exp.minOutputDims = 3;

  exp.requireNCHW = true;

  auto rep = v.validateONNX(onnx, exp, /*tryCUDA=*/false);

  if (!rep.ok && err) *err = rep.errors.join("; ");

  if (warns) *warns = rep.warnings;

  return rep.ok;

}

class ModelManager {
public:
  void loadModel(const QString& path) {
    // 后台加载新模型
    auto newNet = cv::dnn::readNetFromONNX(path.toStdString());

    // 验证模型
    if (!validateModel(newNet)) {
      emit loadFailed("模型验证失败");
      return;
    }

    // 原子切换
    {
      QWriteLocker lock(&m_lock);
      m_net = std::move(newNet);
      m_modelVersion = extractVersion(path);
    }

    emit modelUpdated(m_modelVersion);
  }

  cv::dnn::Net getModel() {
    QReadLocker lock(&m_lock);
    return m_net;
  }

private:
  cv::dnn::Net m_net;
  QString m_modelVersion;
  QReadWriteLock m_lock;
};

#endif // MODELMANAGER_H
