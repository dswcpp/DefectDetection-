/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * UserRepository.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：用户数据仓储接口定义
 * 描述：用户数据访问层，提供用户增删改查、密码验证、权限管理等功能
 *
 * 当前版本：1.1
 * 更新：增强密码安全性，添加密码强度验证
 */

#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include "IRepository.h"
#include "../data_global.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QDateTime>

/**
 * @brief 密码强度等级
 */
enum class PasswordStrength {
  Weak,       // 弱：长度不足或过于简单
  Medium,     // 中：满足基本要求
  Strong      // 强：包含大小写、数字、特殊字符
};

/**
 * @brief 用户信息结构
 */
struct DATA_EXPORT UserInfo {
  qint64 id = 0;
  QString username;
  QString passwordHash;
  QString salt;               // 独立的盐值
  QString displayName;
  QString role;               // admin/operator/viewer
  QStringList permissions;
  bool isActive = true;
  bool mustChangePassword = false;  // 首次登录需修改密码
  int loginFailCount = 0;           // 登录失败次数
  QDateTime lockedUntil;            // 锁定到期时间
  QDateTime createdAt;
  QDateTime lastLoginAt;
};

class DATA_EXPORT UserRepository : public QObject, public IRepository {
  Q_OBJECT
public:
  explicit UserRepository(const QString& connectionName, QObject* parent = nullptr);

  QString name() const override { return "UserRepository"; }
  bool isReady() const override;
  int totalCount() const override { return count(); }

  // ============ 初始化 ============
  bool ensureTablesExist();
  bool createDefaultUsers();

  // ============ 查询 ============
  UserInfo findByUsername(const QString& username);
  UserInfo findById(qint64 id);
  QVector<UserInfo> findAll();
  bool usernameExists(const QString& username);
  int count() const;

  // ============ 增删改 ============
  qint64 insert(const UserInfo& user);
  bool update(const UserInfo& user);
  bool updatePassword(qint64 userId, const QString& newPassword);
  bool updateLastLogin(qint64 userId, const QDateTime& time);
  bool remove(qint64 userId);

  // ============ 登录安全 ============
  bool recordLoginFailure(qint64 userId);
  bool resetLoginFailCount(qint64 userId);
  bool lockUser(qint64 userId, int minutesToLock = 30);
  bool unlockUser(qint64 userId);
  bool isUserLocked(qint64 userId);
  bool setMustChangePassword(qint64 userId, bool mustChange);

  // ============ 密码工具（静态） ============

  /**
   * @brief 生成随机盐值
   */
  static QString generateSalt();

  /**
   * @brief 使用盐值哈希密码
   * @param password 原始密码
   * @param salt 盐值（如果为空则生成新的）
   * @return 哈希后的密码
   */
  static QString hashPassword(const QString& password, const QString& salt);

  /**
   * @brief 验证密码
   * @param password 用户输入的密码
   * @param hash 存储的哈希值
   * @param salt 存储的盐值
   */
  static bool verifyPassword(const QString& password, const QString& hash, const QString& salt);

  /**
   * @brief 检查密码强度
   */
  static PasswordStrength checkPasswordStrength(const QString& password);

  /**
   * @brief 验证密码是否满足最低要求
   * @param password 密码
   * @param errorMsg 如果不满足，返回错误提示
   */
  static bool validatePassword(const QString& password, QString& errorMsg);

  // 向后兼容的旧接口（使用空 salt，仅用于迁移）
  static QString hashPasswordLegacy(const QString& password);
  static bool verifyPasswordLegacy(const QString& password, const QString& hash);

private:
  QString m_connectionName;
  UserInfo parseUserFromQuery(QSqlQuery& query);

  // 密码配置
  static constexpr int MIN_PASSWORD_LENGTH = 6;
  static constexpr int MAX_LOGIN_FAILURES = 5;
  static constexpr int SALT_LENGTH = 32;
};

#endif // USERREPOSITORY_H
