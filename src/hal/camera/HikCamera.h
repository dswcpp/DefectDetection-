/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * HikCamera.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：海康相机接口定义
 * 描述：海康威视工业相机SDK封装，支持参数设置、触发模式、图像采集
 *
 * 当前版本：1.0
 */

#ifndef HIKCAMERA_H
#define HIKCAMERA_H

#include "ICamera.h"
#include "hal_global.h"

class HAL_EXPORT HikCamera : public ICamera {
public:
  HikCamera() = default;

  bool open(const CameraConfig &cfg) override;
  bool grab(cv::Mat &frame) override;
  void close() override;
};

#endif // HIKCAMERA_H
