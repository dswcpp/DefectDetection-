/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * IRepository.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：数据仓储接口基类定义
 * 描述：Repository模式基类，定义CRUD操作接口，所有具体Repository继承此类
 *
 * 当前版本：1.1
 * 更新：增强接口，添加通用辅助方法
 */

#ifndef IREPOSITORY_H
#define IREPOSITORY_H

#include "../data_global.h"
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

/**
 * @brief Repository 接口基类
 *
 * 提供数据访问层的通用接口和辅助方法
 */
class DATA_EXPORT IRepository {
public:
  virtual ~IRepository() = default;

  /**
   * @brief 获取 Repository 名称
   */
  virtual QString name() const = 0;

  /**
   * @brief 检查 Repository 是否就绪
   */
  virtual bool isReady() const { return true; }

  /**
   * @brief 清空所有数据（谨慎使用）
   */
  virtual bool clear() { return false; }

  /**
   * @brief 获取记录总数
   */
  virtual int totalCount() const { return -1; }

protected:
  /**
   * @brief 获取数据库连接（供子类使用）
   */
  static QSqlDatabase getDatabase(const QString& connectionName) {
    return QSqlDatabase::database(connectionName);
  }

  /**
   * @brief 检查数据库是否打开
   */
  static bool isDatabaseOpen(const QString& connectionName) {
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    return db.isOpen();
  }

  /**
   * @brief 开始事务
   */
  static bool beginTransaction(const QString& connectionName) {
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    return db.isOpen() && db.transaction();
  }

  /**
   * @brief 提交事务
   */
  static bool commitTransaction(const QString& connectionName) {
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    return db.commit();
  }

  /**
   * @brief 回滚事务
   */
  static bool rollbackTransaction(const QString& connectionName) {
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    return db.rollback();
  }

  /**
   * @brief 获取最后的错误信息
   */
  static QString lastError(const QSqlQuery& query) {
    return query.lastError().text();
  }
};

/**
 * @brief RAII 事务管理器
 *
 * 自动管理事务的提交和回滚
 * 用法：
 *   TransactionGuard guard(connectionName);
 *   // ... 执行数据库操作 ...
 *   guard.commit();  // 成功则提交
 *   // 如果没有调用 commit()，析构时自动回滚
 */
class DATA_EXPORT TransactionGuard {
public:
  explicit TransactionGuard(const QString& connectionName)
      : m_connectionName(connectionName), m_committed(false) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    m_active = db.isOpen() && db.transaction();
  }

  ~TransactionGuard() {
    if (m_active && !m_committed) {
      QSqlDatabase db = QSqlDatabase::database(m_connectionName);
      db.rollback();
    }
  }

  bool isActive() const { return m_active; }

  bool commit() {
    if (m_active && !m_committed) {
      QSqlDatabase db = QSqlDatabase::database(m_connectionName);
      m_committed = db.commit();
      return m_committed;
    }
    return false;
  }

  void rollback() {
    if (m_active && !m_committed) {
      QSqlDatabase db = QSqlDatabase::database(m_connectionName);
      db.rollback();
      m_committed = true; // 标记已处理，防止析构时再次回滚
    }
  }

  // 禁止拷贝
  TransactionGuard(const TransactionGuard&) = delete;
  TransactionGuard& operator=(const TransactionGuard&) = delete;

private:
  QString m_connectionName;
  bool m_active;
  bool m_committed;
};

#endif // IREPOSITORY_H
