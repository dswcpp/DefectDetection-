#ifndef CONFIGREPOSITORY_H
#define CONFIGREPOSITORY_H

#include "IRepository.h"
#include "data_global.h"
#include <QObject>
#include <QString>
#include <QVariant>

// 前向声明
struct DatabaseConfig;
struct LogConfig;

class DATA_EXPORT ConfigRepository : public QObject, public IRepository {
  Q_OBJECT
public:
  explicit ConfigRepository(QObject* parent = nullptr);
  ~ConfigRepository() override = default;

  QString name() const override { return QStringLiteral("ConfigRepository"); }

  // ======================== 配置文件操作 ========================

  // 加载配置文件
  bool load(const QString& path = QString());

  // 保存配置到文件
  bool save();
  bool saveAs(const QString& path);

  // 重新加载配置
  bool reload();

  // 获取配置文件路径
  QString configPath() const;

  // ======================== 数据库配置 ========================

  QString databasePath() const;
  void setDatabasePath(const QString& path);

  int maxRecords() const;
  void setMaxRecords(int count);

  bool autoCleanup() const;
  void setAutoCleanup(bool enabled);

  // 获取完整数据库配置
  DatabaseConfig databaseConfig() const;
  void setDatabaseConfig(const DatabaseConfig& config);

  // ======================== 日志配置 ========================

  QString logLevel() const;
  void setLogLevel(const QString& level);

  QString logDir() const;
  void setLogDir(const QString& dir);

  LogConfig logConfig() const;
  void setLogConfig(const LogConfig& config);

  // ======================== 通用配置访问 ========================

  // 通过路径获取配置值 (如 "database.path", "log.level")
  QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;

  // 设置配置值
  void setValue(const QString& key, const QVariant& value);

signals:
  void configLoaded(const QString& path);
  void configSaved(const QString& path);
  void configChanged(const QString& section);
  void databaseConfigChanged();
  void logConfigChanged();

private:
  void connectSignals();
};

#endif // CONFIGREPOSITORY_H
