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
