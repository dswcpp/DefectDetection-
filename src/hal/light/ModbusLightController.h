/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ModbusLightController.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：Modbus光源控制器接口定义
 * 描述：通过Modbus协议控制的光源，支持TCP和RTU模式
 *
 * 当前版本：1.0
 */

#ifndef MODBUSLIGHTCONTROLLER_H
#define MODBUSLIGHTCONTROLLER_H

#include "ILightController.h"

class ModbusLightController : public ILightController {
public:
  ModbusLightController() = default;

  bool connect(const QString &port) override;
  bool setBrightness(int channel, int value) override;
  void disconnect() override;
};

#endif // MODBUSLIGHTCONTROLLER_H
