/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * SerialIO.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：串口IO控制器接口定义
 * 描述：通过串口扩展的IO控制器，如Arduino等开发板
 *
 * 当前版本：1.0
 */

#ifndef SERIALIO_H
#define SERIALIO_H

#include "IIOController.h"

class SerialIO : public IIOController {
public:
  SerialIO() = default;

  bool init(const QString &configPath) override;
  bool writeOutput(int channel, bool value) override;
  bool readInput(int channel, bool &value) override;
};

#endif // SERIALIO_H
