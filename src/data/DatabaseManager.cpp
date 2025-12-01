#include "DatabaseManager.h"
#include "repositories/ConfigRepository.h"
#include "repositories/DefectRepository.h"
#include "repositories/ImageRepository.h"
#include "repositories/AnnotationRepository.h"
#include "Logger.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
    , m_configRepo(std::make_unique<ConfigRepository>(this)) {
  m_connectionName = QUuid::createUuid().toString(QUuid::Id128);

  // 创建仓库
  m_defectRepo = std::make_unique<DefectRepository>(m_connectionName, this);
  m_imageRepo = std::make_unique<ImageRepository>(m_connectionName, this);
  m_annotationRepo = std::make_unique<AnnotationRepository>(m_connectionName, this);

  // 监听配置变更
  connect(m_configRepo.get(), &ConfigRepository::databaseConfigChanged,
          this, &DatabaseManager::onDatabaseConfigChanged);
}

DatabaseManager::~DatabaseManager() {
  close();
}

bool DatabaseManager::initFromConfig() {
  QString dbPath = m_configRepo->databasePath();
  if (dbPath.isEmpty()) {
    dbPath = QStringLiteral("./data/defects.db");
  }

  LOG_INFO("DatabaseManager: Initializing from config, path={}", dbPath);
  return open(dbPath);
}

bool DatabaseManager::open(const QString& dbPath) {
  if (m_db.isOpen()) {
    if (m_dbPath == dbPath) {
      return true;  // 已经打开了相同的数据库
    }
    close();  // 关闭旧连接，打开新连接
  }

  // 确保目录存在
  QFileInfo fi(dbPath);
  QDir dir = fi.dir();
  if (!dir.exists()) {
    if (!dir.mkpath(".")) {
      LOG_ERROR("DatabaseManager: Failed to create directory {}", dir.path());
      return false;
    }
  }

  m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
  m_db.setDatabaseName(dbPath);

  if (!m_db.open()) {
    LOG_ERROR("DatabaseManager: Failed to open database: {}",
              m_db.lastError().text());
    return false;
  }

  m_dbPath = dbPath;
  LOG_INFO("DatabaseManager: Database opened successfully: {}", dbPath);
  emit opened();
  return true;
}

void DatabaseManager::close() {
  if (!m_db.isOpen()) {
    return;
  }

  QString path = m_dbPath;
  m_db.close();
  m_dbPath.clear();

  // 移除连接
  QSqlDatabase::removeDatabase(m_connectionName);

  LOG_INFO("DatabaseManager: Database closed: {}", path);
  emit closed();
}

bool DatabaseManager::isOpen() const {
  return m_db.isOpen();
}

bool DatabaseManager::beginTransaction() {
  if (!m_db.isOpen()) {
    LOG_ERROR("DatabaseManager: Cannot begin transaction, db not open");
    return false;
  }
  return m_db.transaction();
}

bool DatabaseManager::commit() {
  if (!m_db.isOpen()) {
    LOG_ERROR("DatabaseManager: Cannot commit, db not open");
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

bool DatabaseManager::exec(const QString& sql) {
  if (!m_db.isOpen()) {
    LOG_ERROR("DatabaseManager: Cannot exec, db not open");
    return false;
  }

  QSqlQuery query(m_db);
  if (!query.exec(sql)) {
    LOG_ERROR("DatabaseManager: SQL exec failed: {} | error={}",
              sql, query.lastError().text());
    return false;
  }
  return true;
}

bool DatabaseManager::executeSchema(const QString& schemaPath) {
  QFile file(schemaPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    LOG_ERROR("DatabaseManager: Failed to open schema file: {}", schemaPath);
    return false;
  }

  QString content = QString::fromUtf8(file.readAll());
  file.close();

  // 按分号分割 SQL 语句
  QStringList statements = content.split(';', Qt::SkipEmptyParts);

  QSqlQuery query(m_db);
  for (const QString& stmtRaw : statements) {
    QString stmt = stmtRaw.trimmed();
    if (stmt.isEmpty() || stmt.startsWith("--")) {
      continue;
    }

    if (!query.exec(stmt)) {
      LOG_ERROR("DatabaseManager: Schema statement failed: {} | error={}",
                stmt.left(100), query.lastError().text());
      return false;
    }
  }

  LOG_INFO("DatabaseManager: Schema executed successfully: {}", schemaPath);
  return true;
}

void DatabaseManager::onDatabaseConfigChanged() {
  QString newPath = m_configRepo->databasePath();
  if (newPath != m_dbPath && !newPath.isEmpty()) {
    LOG_INFO("DatabaseManager: Database config changed, reconnecting to {}",
             newPath);
    close();
    open(newPath);
  }
}
