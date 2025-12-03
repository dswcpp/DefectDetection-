/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * SerialLightController.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：串口光源控制器接口定义
 * 描述：通过RS232/RS485串口控制的光源，支持自定义协议
 *
 * 当前版本：1.0
 */

#ifndef SERIALLIGHTCONTROLLER_H
#define SERIALLIGHTCONTROLLER_H

#include "ILightController.h"

class SerialLightController : public ILightController {
public:
  SerialLightController() = default;

  bool connect(const QString &port) override;
  bool setBrightness(int channel, int value) override;
  void disconnect() override;
};

#endif // SERIALLIGHTCONTROLLER_H
