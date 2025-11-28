#include "DatabaseManager.h"

DatabaseManager::DatabaseManager(QObject *parent) : QObject{parent} {}

bool DatabaseManager::open(const QString &dbPath) {
  Q_UNUSED(dbPath);
  // TODO: 初始化 QSqlDatabase，设置路径并打开
  emit opened();
  return true;
}

void DatabaseManager::close() {
  // TODO: 关闭 QSqlDatabase，释放资源
  emit closed();
}

bool DatabaseManager::isOpen() const {
  // TODO: 返回数据库连接状态
  return false;
}
