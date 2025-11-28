#include "GPIOController.h"

bool GPIOController::init(const QString &configPath) {
  Q_UNUSED(configPath);
  // TODO: 初始化 GPIO，加载映射
  return true;
}

bool GPIOController::writeOutput(int channel, bool value) {
  Q_UNUSED(channel);
  Q_UNUSED(value);
  // TODO: 写 DO
  return true;
}

bool GPIOController::readInput(int channel, bool &value) {
  Q_UNUSED(channel);
  // TODO: 读 DI
  value = false;
  return true;
}
