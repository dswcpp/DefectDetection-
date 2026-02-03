/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ICamera.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：相机接口基类定义
 * 描述：相机抽象接口，定义open/close/grab/setExposure等通用方法，
 *       所有相机实现类继承此接口
 *
 * 当前版本：1.0
 */

#ifndef ICAMERA_H
#define ICAMERA_H

#include <QString>
#include <opencv2/core.hpp>  // 只需要 cv::Mat
#include "hal_global.h"

struct HAL_EXPORT CameraConfig {
  QString type;    // gige/usb/file/mock
  QString ip;
  int port = 0;
  QString serial;
  int width = 0;
  int height = 0;
  double exposureUs = 0.0;
  double gainDb = 0.0;
};

class HAL_EXPORT ICamera {
public:
  virtual ~ICamera() = default;

  virtual bool open(const CameraConfig &cfg) = 0;
  virtual bool grab(cv::Mat &frame) = 0;
  virtual void close() = 0;

  // 获取当前图片路径（主要用于 FileCamera）
  virtual QString currentImagePath() const { return QString(); }

  // TODO: 参数设置/心跳/事件回调接口
};

#endif // ICAMERA_H
