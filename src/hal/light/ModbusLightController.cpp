#include "ModbusLightController.h"

bool ModbusLightController::connect(const QString &port) {
  Q_UNUSED(port);
  // TODO: 建立 Modbus 连接，配置超时/波特率
  return true;
}

bool ModbusLightController::setBrightness(int channel, int value) {
  Q_UNUSED(channel);
  Q_UNUSED(value);
  // TODO: 通过 Modbus 写光源通道值
  return true;
}

void ModbusLightController::disconnect() {
  // TODO: 关闭 Modbus 连接
}
