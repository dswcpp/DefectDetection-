#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "data_global.h"
#include <QObject>
#include <QString>

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

  // TODO: 事务封装 begin/commit/rollback
  // TODO: 提供执行 SQL/预处理接口

signals:
  void opened();
  void closed();

private:
  // TODO: 持有 QSqlDatabase 或封装类
};

#endif // DATABASEMANAGER_H
