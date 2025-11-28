#include "ConfigRepository.h"

ConfigRepository::ConfigRepository(QObject *parent) : QObject{parent} {}

bool ConfigRepository::load(const QString &path) {
  Q_UNUSED(path);
  // TODO: 读取配置文件并填充内存模型
  emit configLoaded(path);
  return true;
}

bool ConfigRepository::save(const QString &path) const {
  Q_UNUSED(path);
  // TODO: 将内存配置写回文件
  //emit configSaved(path);
  return true;
}
