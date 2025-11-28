#include "ConfigValidator.h"

bool ConfigValidator::validateFile(const QString &path) const {
  Q_UNUSED(path);
  // TODO: 读取文件并校验字段
  return true;
}

bool ConfigValidator::validateMemory() const {
  // TODO: 校验内存中的配置结构
  return true;
}
