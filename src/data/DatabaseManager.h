#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "data_global.h"
#include <QObject>
#include <QString>
#include <QSqlDatabase>

class DatabaseManager : public QObject {
  Q_OBJECT
public:
  explicit DatabaseManager(QObject *parent = nullptr);

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

  // TODO: 提供预处理/绑定参数接口
  bool executeSchema(const QString &schemaPath);

signals:
  void opened();
  void closed();

private:
  // TODO: 持有 QSqlDatabase 或封装类
  QSqlDatabase m_db;
  QString m_connectionName;
};

#endif // DATABASEMANAGER_H
