#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QString>

class ConfigManager : public QObject {
  Q_OBJECT
public:
  explicit ConfigManager(QObject *parent = nullptr);

  // 加载配置（含默认值回填）
  bool load(const QString &path);

  // 热更新配置
  bool reload();

  // 校验配置
  bool validate() const;

  // 获取当前配置路径
  QString configPath() const;

signals:
  void configLoaded(const QString &path);
  void configReloaded(const QString &path);
  void configInvalid(const QString &reason);

private:
  QString m_configPath;
  // TODO: 存放配置数据结构
};

#endif // CONFIGMANAGER_H
