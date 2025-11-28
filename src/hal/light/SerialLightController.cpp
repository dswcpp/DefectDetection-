#include "SerialLightController.h"

bool SerialLightController::connect(const QString &port) {
  Q_UNUSED(port);
  // TODO: 打开串口，设置波特率/校验，握手光源
  return true;
}

bool SerialLightController::setBrightness(int channel, int value) {
  Q_UNUSED(channel);
  Q_UNUSED(value);
  // TODO: 发送串口命令调整亮度
  return true;
}

void SerialLightController::disconnect() {
  // TODO: 关闭串口
}
