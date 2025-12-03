/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * USBCamera.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：USB相机接口定义
 * 描述：基于OpenCV VideoCapture的USB相机封装，支持普通USB摄像头
 *
 * 当前版本：1.0
 */

#ifndef USBCAMERA_H
#define USBCAMERA_H

#include "ICamera.h"
#include "hal_global.h"

class HAL_EXPORT USBCamera : public ICamera {
public:
  USBCamera() = default;

  bool open(const CameraConfig &cfg) override;
  bool grab(cv::Mat &frame) override;
  void close() override;
};

#endif // USBCAMERA_H
