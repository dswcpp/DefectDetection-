#include "SerialIO.h"

bool SerialIO::init(const QString &configPath) {
  Q_UNUSED(configPath);
  // TODO: 打开串口，配置波特率/校验，加载 IO 映射
  return true;
}

bool SerialIO::writeOutput(int channel, bool value) {
  Q_UNUSED(channel);
  Q_UNUSED(value);
  // TODO: 通过串口发送 DO 命令
  return true;
}

bool SerialIO::readInput(int channel, bool &value) {
  Q_UNUSED(channel);
  // TODO: 通过串口读取 DI 状态
  value = false;
  return true;
}
