/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * BackupManager.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：数据备份管理模块接口定义
 * 描述：数据库和配置文件备份管理，支持定时备份、增量备份、
 *       备份恢复等功能
 *
 * 当前版本：1.0
 */

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
