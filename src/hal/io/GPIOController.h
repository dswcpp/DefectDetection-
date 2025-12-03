/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * GPIOController.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：GPIO控制器接口定义
 * 描述：通用GPIO控制器实现，支持输入输出方向设置、电平读写
 *
 * 当前版本：1.0
 */

#ifndef GPIOCONTROLLER_H
#define GPIOCONTROLLER_H

#include "IIOController.h"

class GPIOController : public IIOController {
public:
  GPIOController() = default;

  bool init(const QString &configPath) override;
  bool writeOutput(int channel, bool value) override;
  bool readInput(int channel, bool &value) override;
};

#endif // GPIOCONTROLLER_H
