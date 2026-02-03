#include "UserRepository.h"
#include "common/Logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QRegularExpression>

UserRepository::UserRepository(const QString& connectionName, QObject* parent)
    : QObject(parent), m_connectionName(connectionName) {
}

bool UserRepository::isReady() const {
  return isDatabaseOpen(m_connectionName);
}

// ============ 密码工具 ============

QString UserRepository::generateSalt() {
  QByteArray salt;
  salt.resize(SALT_LENGTH);
  for (int i = 0; i < SALT_LENGTH; ++i) {
    salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
  }
  return QString::fromLatin1(salt.toHex());
}

QString UserRepository::hashPassword(const QString& password, const QString& salt) {
  // 使用 PBKDF2 风格的多次迭代哈希（简化版）
  QByteArray data = (password + salt).toUtf8();
  for (int i = 0; i < 10000; ++i) {
    data = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
  }
  return QString::fromLatin1(data.toHex());
}

bool UserRepository::verifyPassword(const QString& password, const QString& hash, const QString& salt) {
  return hashPassword(password, salt) == hash;
}

QString UserRepository::hashPasswordLegacy(const QString& password) {
  // 兼容旧版本：使用固定 salt
  QString salted = password + "DefectDetection2025";
  return QString::fromLatin1(
      QCryptographicHash::hash(salted.toUtf8(), QCryptographicHash::Sha256).toHex());
}

bool UserRepository::verifyPasswordLegacy(const QString& password, const QString& hash) {
  return hashPasswordLegacy(password) == hash;
}

PasswordStrength UserRepository::checkPasswordStrength(const QString& password) {
  if (password.length() < MIN_PASSWORD_LENGTH) {
    return PasswordStrength::Weak;
  }

  bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
  for (const QChar& c : password) {
    if (c.isUpper()) hasUpper = true;
    else if (c.isLower()) hasLower = true;
    else if (c.isDigit()) hasDigit = true;
    else hasSpecial = true;
  }

  int score = (hasUpper ? 1 : 0) + (hasLower ? 1 : 0) + (hasDigit ? 1 : 0) + (hasSpecial ? 1 : 0);
  
  if (score >= 4 && password.length() >= 10) {
    return PasswordStrength::Strong;
  } else if (score >= 2 && password.length() >= MIN_PASSWORD_LENGTH) {
    return PasswordStrength::Medium;
  }
  return PasswordStrength::Weak;
}

bool UserRepository::validatePassword(const QString& password, QString& errorMsg) {
  if (password.length() < MIN_PASSWORD_LENGTH) {
    errorMsg = QString::fromUtf8("密码长度至少需要 %1 个字符").arg(MIN_PASSWORD_LENGTH);
    return false;
  }
  
  // 检查是否包含用户名等常见弱密码模式
  static const QStringList weakPatterns = {
    "123456", "password", "admin", "qwerty", "111111", "000000"
  };
  for (const QString& pattern : weakPatterns) {
    if (password.toLower().contains(pattern)) {
      errorMsg = QString::fromUtf8("密码过于简单，请避免使用常见密码");
      return false;
    }
  }
  
  errorMsg.clear();
  return true;
}

// ============ 表管理 ============

bool UserRepository::ensureTablesExist() {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) {
    LOG_WARN("UserRepository: Database not open for ensureTablesExist");
    return false;
  }

  QSqlQuery query(db);
  
  // 创建用户表（新结构，包含 salt 字段）
  QString createUsersTable = 
      "CREATE TABLE IF NOT EXISTS users ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
      "username TEXT UNIQUE NOT NULL, "
      "password_hash TEXT NOT NULL, "
      "salt TEXT, "
      "display_name TEXT, "
      "role TEXT DEFAULT 'operator', "
      "permissions TEXT, "
      "is_active INTEGER DEFAULT 1, "
      "must_change_password INTEGER DEFAULT 0, "
      "login_fail_count INTEGER DEFAULT 0, "
      "locked_until DATETIME, "
      "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
      "last_login_at DATETIME)";

  if (!query.exec(createUsersTable)) {
    LOG_WARN("UserRepository: Failed to create users table: {}", query.lastError().text().toStdString());
    return false;
  }

  // 检查并添加缺失的列（数据库迁移）
  struct ColumnDef {
    QString name;
    QString type;
    QString defaultVal;
  };
  QVector<ColumnDef> requiredColumns = {
      {"display_name", "TEXT", ""},
      {"role", "TEXT", "'operator'"},
      {"permissions", "TEXT", ""},
      {"is_active", "INTEGER", "1"},
      {"salt", "TEXT", ""},
      {"must_change_password", "INTEGER", "0"},
      {"login_fail_count", "INTEGER", "0"},
      {"locked_until", "DATETIME", ""},
      {"last_login_at", "DATETIME", ""}
  };

  for (const auto& col : requiredColumns) {
    QSqlQuery checkQuery(db);
    checkQuery.exec(QString("SELECT %1 FROM users LIMIT 1").arg(col.name));
    if (checkQuery.lastError().isValid()) {
      QString alterSql = QString("ALTER TABLE users ADD COLUMN %1 %2").arg(col.name, col.type);
      if (!col.defaultVal.isEmpty()) {
        alterSql += QString(" DEFAULT %1").arg(col.defaultVal);
      }
      QSqlQuery alterQuery(db);
      if (!alterQuery.exec(alterSql)) {
        LOG_WARN("UserRepository: Failed to add column {}: {}", 
                 col.name.toStdString(), alterQuery.lastError().text().toStdString());
      } else {
        LOG_INFO("UserRepository: Added missing column: {}", col.name.toStdString());
      }
    }
  }

  LOG_DEBUG("UserRepository: Users table ensured");
  return true;
}

bool UserRepository::createDefaultUsers() {
  int userCount = count();
  LOG_DEBUG("UserRepository: Current user count: {}", userCount);

  if (userCount > 0) {
    LOG_DEBUG("UserRepository: Users already exist, skipping default creation");
    return true;
  }

  bool success = true;

  // 管理员（首次登录需要修改密码）
  UserInfo admin;
  admin.username = "admin";
  admin.salt = generateSalt();
  admin.passwordHash = hashPassword("admin123", admin.salt);
  admin.displayName = QString::fromUtf8("系统管理员");
  admin.role = "admin";
  admin.permissions = {"ViewHistory", "DeleteHistory", "ViewStatistics", "ExportData",
                       "RunDetection", "SystemSettings", "ManageUsers"};
  admin.mustChangePassword = true;  // 要求首次登录修改密码
  if (insert(admin) < 0) {
    LOG_WARN("UserRepository: Failed to create admin user");
    success = false;
  }

  // 操作员
  UserInfo op;
  op.username = "operator";
  op.salt = generateSalt();
  op.passwordHash = hashPassword("operator123", op.salt);
  op.displayName = QString::fromUtf8("操作员");
  op.role = "operator";
  op.permissions = {"ViewHistory", "ViewStatistics", "RunDetection", "ExportData"};
  op.mustChangePassword = true;
  if (insert(op) < 0) {
    LOG_WARN("UserRepository: Failed to create operator user");
    success = false;
  }

  // 观察员
  UserInfo viewer;
  viewer.username = "viewer";
  viewer.salt = generateSalt();
  viewer.passwordHash = hashPassword("viewer123", viewer.salt);
  viewer.displayName = QString::fromUtf8("观察员");
  viewer.role = "viewer";
  viewer.permissions = {"ViewHistory", "ViewStatistics"};
  viewer.mustChangePassword = true;
  if (insert(viewer) < 0) {
    LOG_WARN("UserRepository: Failed to create viewer user");
    success = false;
  }

  if (success) {
    LOG_INFO("UserRepository: Default users created (require password change on first login)");
  }
  return success;
}

// ============ 内部辅助 ============

UserInfo UserRepository::parseUserFromQuery(QSqlQuery& query) {
  UserInfo user;
  user.id = query.value("id").toLongLong();
  user.username = query.value("username").toString();
  user.passwordHash = query.value("password_hash").toString();
  user.salt = query.value("salt").toString();
  user.displayName = query.value("display_name").toString();
  user.role = query.value("role").toString();
  user.permissions = query.value("permissions").toString().split(",", Qt::SkipEmptyParts);
  user.isActive = query.value("is_active").toInt() == 1;
  user.mustChangePassword = query.value("must_change_password").toInt() == 1;
  user.loginFailCount = query.value("login_fail_count").toInt();
  user.lockedUntil = query.value("locked_until").toDateTime();
  user.createdAt = query.value("created_at").toDateTime();
  user.lastLoginAt = query.value("last_login_at").toDateTime();
  return user;
}

// ============ 查询 ============

UserInfo UserRepository::findByUsername(const QString& username) {
  UserInfo user;
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) {
    LOG_WARN("UserRepository: Database not open");
    return user;
  }

  QSqlQuery query(db);
  QString sql = "SELECT id, username, password_hash, salt, display_name, role, permissions, "
                "is_active, must_change_password, login_fail_count, locked_until, "
                "created_at, last_login_at FROM users WHERE username = ? AND is_active = 1";

  if (!query.prepare(sql)) {
    LOG_WARN("UserRepository: Failed to prepare findByUsername: {}", query.lastError().text().toStdString());
    return user;
  }
  query.addBindValue(username);

  if (!query.exec()) {
    LOG_WARN("UserRepository: Failed to exec findByUsername: {}", query.lastError().text().toStdString());
    return user;
  }

  if (query.next()) {
    user = parseUserFromQuery(query);
    LOG_DEBUG("UserRepository: Found user: {} id: {}", user.username.toStdString(), user.id);
  } else {
    LOG_DEBUG("UserRepository: User not found: {}", username.toStdString());
  }

  return user;
}

UserInfo UserRepository::findById(qint64 id) {
  UserInfo user;
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return user;

  QSqlQuery query(db);
  QString sql = "SELECT id, username, password_hash, salt, display_name, role, permissions, "
                "is_active, must_change_password, login_fail_count, locked_until, "
                "created_at, last_login_at FROM users WHERE id = ?";
  if (!query.prepare(sql)) {
    return user;
  }
  query.addBindValue(id);

  if (query.exec() && query.next()) {
    user = parseUserFromQuery(query);
  }

  return user;
}

QVector<UserInfo> UserRepository::findAll() {
  QVector<UserInfo> users;
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return users;

  QSqlQuery query(db);
  QString sql = "SELECT id, username, password_hash, salt, display_name, role, permissions, "
                "is_active, must_change_password, login_fail_count, locked_until, "
                "created_at, last_login_at FROM users ORDER BY id";
  if (!query.prepare(sql)) {
    return users;
  }

  if (!query.exec()) {
    LOG_WARN("UserRepository: Failed to get users: {}", query.lastError().text().toStdString());
    return users;
  }

  while (query.next()) {
    users.append(parseUserFromQuery(query));
  }

  return users;
}

bool UserRepository::usernameExists(const QString& username) {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  if (!query.prepare("SELECT COUNT(*) FROM users WHERE username = ?")) {
    return false;
  }
  query.addBindValue(username);

  if (query.exec() && query.next()) {
    return query.value(0).toInt() > 0;
  }

  return false;
}

int UserRepository::count() const {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return 0;

  QSqlQuery query(db);
  if (query.exec("SELECT COUNT(*) FROM users") && query.next()) {
    return query.value(0).toInt();
  }

  return 0;
}

// ============ 增删改 ============

qint64 UserRepository::insert(const UserInfo& user) {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) {
    LOG_WARN("UserRepository: Database not open for insert");
    return -1;
  }

  QSqlQuery query(db);
  QString sql = "INSERT INTO users (username, password_hash, salt, display_name, role, "
                "permissions, is_active, must_change_password) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

  if (!query.prepare(sql)) {
    LOG_WARN("UserRepository: Failed to prepare insert: {}", query.lastError().text().toStdString());
    return -1;
  }

  query.addBindValue(user.username);
  query.addBindValue(user.passwordHash);
  query.addBindValue(user.salt);
  query.addBindValue(user.displayName);
  query.addBindValue(user.role);
  query.addBindValue(user.permissions.join(","));
  query.addBindValue(user.isActive ? 1 : 0);
  query.addBindValue(user.mustChangePassword ? 1 : 0);

  if (!query.exec()) {
    LOG_WARN("UserRepository: Failed to insert user: {}", query.lastError().text().toStdString());
    return -1;
  }

  return query.lastInsertId().toLongLong();
}

bool UserRepository::update(const UserInfo& user) {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  QString sql = "UPDATE users SET display_name = ?, role = ?, permissions = ?, is_active = ? WHERE id = ?";
  if (!query.prepare(sql)) {
    return false;
  }
  query.addBindValue(user.displayName);
  query.addBindValue(user.role);
  query.addBindValue(user.permissions.join(","));
  query.addBindValue(user.isActive ? 1 : 0);
  query.addBindValue(user.id);

  if (!query.exec()) {
    LOG_WARN("UserRepository: Failed to update user: {}", query.lastError().text().toStdString());
    return false;
  }

  return true;
}

bool UserRepository::updatePassword(qint64 userId, const QString& newPassword) {
  QString errorMsg;
  if (!validatePassword(newPassword, errorMsg)) {
    LOG_WARN("UserRepository: Password validation failed: {}", errorMsg.toStdString());
    return false;
  }

  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return false;

  QString newSalt = generateSalt();
  QString newHash = hashPassword(newPassword, newSalt);

  QSqlQuery query(db);
  if (!query.prepare("UPDATE users SET password_hash = ?, salt = ?, must_change_password = 0 WHERE id = ?")) {
    return false;
  }
  query.addBindValue(newHash);
  query.addBindValue(newSalt);
  query.addBindValue(userId);

  return query.exec();
}

bool UserRepository::updateLastLogin(qint64 userId, const QDateTime& time) {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  if (!query.prepare("UPDATE users SET last_login_at = ? WHERE id = ?")) {
    return false;
  }
  query.addBindValue(time);
  query.addBindValue(userId);

  return query.exec();
}

bool UserRepository::remove(qint64 userId) {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  if (!query.prepare("DELETE FROM users WHERE id = ?")) {
    return false;
  }
  query.addBindValue(userId);

  return query.exec();
}

// ============ 登录安全 ============

bool UserRepository::recordLoginFailure(qint64 userId) {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  if (!query.prepare("UPDATE users SET login_fail_count = login_fail_count + 1 WHERE id = ?")) {
    return false;
  }
  query.addBindValue(userId);

  if (!query.exec()) return false;

  // 检查是否需要锁定
  UserInfo user = findById(userId);
  if (user.loginFailCount >= MAX_LOGIN_FAILURES) {
    lockUser(userId);
    LOG_WARN("UserRepository: User {} locked due to too many failed login attempts", 
             user.username.toStdString());
  }

  return true;
}

bool UserRepository::resetLoginFailCount(qint64 userId) {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  if (!query.prepare("UPDATE users SET login_fail_count = 0 WHERE id = ?")) {
    return false;
  }
  query.addBindValue(userId);

  return query.exec();
}

bool UserRepository::lockUser(qint64 userId, int minutesToLock) {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return false;

  QDateTime lockUntil = QDateTime::currentDateTime().addSecs(minutesToLock * 60);

  QSqlQuery query(db);
  if (!query.prepare("UPDATE users SET locked_until = ? WHERE id = ?")) {
    return false;
  }
  query.addBindValue(lockUntil);
  query.addBindValue(userId);

  return query.exec();
}

bool UserRepository::unlockUser(qint64 userId) {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  if (!query.prepare("UPDATE users SET locked_until = NULL, login_fail_count = 0 WHERE id = ?")) {
    return false;
  }
  query.addBindValue(userId);

  return query.exec();
}

bool UserRepository::isUserLocked(qint64 userId) {
  UserInfo user = findById(userId);
  if (user.lockedUntil.isValid() && user.lockedUntil > QDateTime::currentDateTime()) {
    return true;
  }
  return false;
}

bool UserRepository::setMustChangePassword(qint64 userId, bool mustChange) {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  if (!query.prepare("UPDATE users SET must_change_password = ? WHERE id = ?")) {
    return false;
  }
  query.addBindValue(mustChange ? 1 : 0);
  query.addBindValue(userId);

  return query.exec();
}
