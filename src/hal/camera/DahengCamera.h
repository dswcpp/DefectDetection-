/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * DahengCamera.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：大恒相机接口定义
 * 描述：大恒图像工业相机SDK封装，支持GigE和USB3接口相机
 *
 * 当前版本：1.0
 */

#ifndef DAHENGCAMERA_H
#define DAHENGCAMERA_H

#include "ICamera.h"
#include "hal_global.h"

class HAL_EXPORT DahengCamera : public ICamera {
public:
  DahengCamera() = default;

  bool open(const CameraConfig &cfg) override;
  bool grab(cv::Mat &frame) override;
  void close() override;
};

#endif // DAHENGCAMERA_H
