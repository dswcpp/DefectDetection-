#ifndef CONFIGVALIDATOR_H
#define CONFIGVALIDATOR_H

#include <QString>

class ConfigValidator {
public:
  ConfigValidator() = default;

  // TODO: 校验配置 JSON 内容是否合法
  bool validateFile(const QString &path) const;

  // TODO: 校验内存配置对象
  bool validateMemory() const;
};

#endif // CONFIGVALIDATOR_H
