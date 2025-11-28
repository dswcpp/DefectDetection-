#include "BackupManager.h"

bool BackupManager::backupTo(const QString &targetDir) {
  Q_UNUSED(targetDir);
  // TODO: 复制数据库/配置/图像等到目标目录
  return true;
}

bool BackupManager::restoreFrom(const QString &sourceDir) {
  Q_UNUSED(sourceDir);
  // TODO: 从备份目录恢复数据库/配置/图像
  return true;
}
