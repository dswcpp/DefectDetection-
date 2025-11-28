#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H

#include <QString>

class BackupManager {
public:
  BackupManager() = default;

  // TODO: 备份数据库/配置到指定路径
  bool backupTo(const QString &targetDir);

  // TODO: 从备份恢复
  bool restoreFrom(const QString &sourceDir);
};

#endif // BACKUPMANAGER_H
