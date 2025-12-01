#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "data_global.h"
#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <memory>

class ConfigRepository;
class DefectRepository;

class DATA_EXPORT DatabaseManager : public QObject {
  Q_OBJECT
public:
  explicit DatabaseManager(QObject *parent = nullptr);
  ~DatabaseManager();

  // 从配置初始化数据库
  bool initFromConfig();

  // 初始化/打开数据库连接
  bool open(const QString &dbPath);

  // 关闭连接
  void close();

  // 健康检查
  bool isOpen() const;

  // 事务封装
  bool beginTransaction();
  bool commit();
  void rollback();

  // 执行原始 SQL（无结果）
  bool exec(const QString &sql);

  // 执行 schema 文件
  bool executeSchema(const QString &schemaPath);

  // 获取配置仓库
  ConfigRepository* configRepository() const { return m_configRepo.get(); }

  // 获取缺陷仓库
  DefectRepository* defectRepository() const { return m_defectRepo.get(); }

  // 当前数据库路径
  QString databasePath() const { return m_dbPath; }

  // 获取连接名（供仓库使用）
  QString connectionName() const { return m_connectionName; }

signals:
  void opened();
  void closed();

private slots:
  void onDatabaseConfigChanged();

private:
  QSqlDatabase m_db;
  QString m_connectionName;
  QString m_dbPath;
  std::unique_ptr<ConfigRepository> m_configRepo;
  std::unique_ptr<DefectRepository> m_defectRepo;
};

#endif // DATABASEMANAGER_H
