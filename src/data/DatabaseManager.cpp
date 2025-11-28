#include "DatabaseManager.h"
#include "Logger.h"
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

DatabaseManager::DatabaseManager(QObject *parent) : QObject{parent} {
  m_connectionName = QUuid::createUuid().toString(QUuid::Id128);
}

bool DatabaseManager::open(const QString &dbPath) {
  if (m_db.isOpen()) {
    return true;
  }
  m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
  m_db.setDatabaseName(dbPath);
  if (!m_db.open()) {
    LOG_ERROR("open db failed: {}", m_db.lastError().text());
    return false;
  }
  emit opened();
  return true;
}

void DatabaseManager::close() {
  if (!m_db.isOpen()) {
    return;
  }
  m_db.close();
  QSqlDatabase::removeDatabase(m_connectionName);
  emit closed();
}

bool DatabaseManager::isOpen() const { return m_db.isOpen(); }

bool DatabaseManager::beginTransaction() {
  if (!m_db.isOpen()) {
    LOG_ERROR("beginTransaction: db not open");
    return false;
  }
  return m_db.transaction();
}

bool DatabaseManager::commit() {
  if (!m_db.isOpen()) {
    LOG_ERROR("commit: db not open");
    return false;
  }
  return m_db.commit();
}

void DatabaseManager::rollback() {
  if (!m_db.isOpen()) {
    return;
  }
  m_db.rollback();
}

bool DatabaseManager::exec(const QString &sql) {
  if (!m_db.isOpen()) {
    LOG_ERROR("exec: db not open");
    return false;
  }
  QSqlQuery query(m_db);
  const bool ok = query.exec(sql);
  if (!ok) {
    LOG_ERROR("exec sql failed: {} | err={}", sql, query.lastError().text());
  }
  return ok;
}

bool DatabaseManager::executeSchema(const QString &schemaPath) {
  QFile f(schemaPath);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    LOG_ERROR("open schema failed: {}", schemaPath);
    return false;
  }
  const QString content = QString::fromUtf8(f.readAll());
  const auto statements = content.split(';', Qt::SkipEmptyParts);
  QSqlQuery query(m_db);
  for (const auto &stmtRaw : statements) {
    const auto stmt = stmtRaw.trimmed();
    if (stmt.isEmpty()) continue;
    if (!query.exec(stmt)) {
      LOG_ERROR("exec schema stmt failed: {} | err={}", stmt, query.lastError().text());
      return false;
    }
  }
  return true;
}
