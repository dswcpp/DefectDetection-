/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * GigECamera.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：GigE相机接口定义
 * 描述：通用GigE Vision协议相机封装，基于GenICam标准
 *
 * 当前版本：1.0
 */

#ifndef GIGECAMERA_H
#define GIGECAMERA_H

#include "ICamera.h"
#include "hal_global.h"

class HAL_EXPORT GigECamera : public ICamera {
public:
  GigECamera() = default;

  bool open(const CameraConfig &cfg) override;
  bool grab(cv::Mat &frame) override;
  void close() override;
};

#endif // GIGECAMERA_H
