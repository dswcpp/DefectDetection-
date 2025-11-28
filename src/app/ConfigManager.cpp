#include "ConfigManager.h"
#include "ConfigValidator.h"
#include "Logger.h"

ConfigManager::ConfigManager(QObject *parent) : QObject(parent) {}

bool ConfigManager::load(const QString &path) {
  m_configPath = path;
  // TODO: 读取 JSON 并回填默认值
  if (!validate()) {
    emit configInvalid("validate failed");
    return false;
  }
  emit configLoaded(path);
  return true;
}

bool ConfigManager::reload() {
  // TODO: 重新加载 m_configPath
  if (m_configPath.isEmpty()) {
    emit configInvalid("config path empty");
    return false;
  }
  return load(m_configPath);
}

bool ConfigManager::validate() const {
  // TODO: 调用 ConfigValidator 进行字段校验
  return true;
}

QString ConfigManager::configPath() const { return m_configPath; }
