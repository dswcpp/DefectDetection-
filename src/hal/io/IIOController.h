/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * IIOController.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：IO控制器接口基类定义
 * 描述：数字IO控制器抽象接口，定义输入读取、输出控制方法
 *
 * 当前版本：1.0
 */

#ifndef IIOCONTROLLER_H
#define IIOCONTROLLER_H

#include <QString>

class IIOController {
public:
  virtual ~IIOController() = default;

  // 初始化 IO，加载寄存器/引脚映射
  virtual bool init(const QString &configPath) = 0;

  // 写输出（DO）
  virtual bool writeOutput(int channel, bool value) = 0;

  // 读输入（DI）
  virtual bool readInput(int channel, bool &value) = 0;

  // TODO: 心跳/重连/去抖配置
};

#endif // IIOCONTROLLER_H
