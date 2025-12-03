/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ILightController.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：光源控制器接口基类定义
 * 描述：光源控制器抽象接口，定义开关、亮度调节、频闪控制等方法
 *
 * 当前版本：1.0
 */

#ifndef ILIGHTCONTROLLER_H
#define ILIGHTCONTROLLER_H

#include <QString>

class ILightController {
public:
  virtual ~ILightController() = default;
  virtual bool connect(const QString &port) = 0;
  virtual bool setBrightness(int channel, int value) = 0;
  virtual void disconnect() = 0;

  // TODO: 心跳检测/多通道同步设置
};

#endif // ILIGHTCONTROLLER_H
