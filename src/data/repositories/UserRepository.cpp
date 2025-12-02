#include "UserRepository.h"
#include "Logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QCryptographicHash>

UserRepository::UserRepository(const QString& connectionName, QObject* parent)
    : QObject(parent), m_connectionName(connectionName) {
}

QString UserRepository::hashPassword(const QString& password) {
    QString salted = password + "DefectDetection2025";
    return QString(QCryptographicHash::hash(salted.toUtf8(), QCryptographicHash::Sha256).toHex());
}

bool UserRepository::verifyPassword(const QString& password, const QString& hash) {
    return hashPassword(password) == hash;
}

bool UserRepository::ensureTablesExist() {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isOpen()) {
        LOG_WARN("UserRepository: Database not open for ensureTablesExist");
        return false;
    }

    QSqlQuery query(db);
    QString createUsersTable = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password_hash TEXT NOT NULL, "
        "display_name TEXT, "
        "role TEXT DEFAULT 'operator', "
        "permissions TEXT, "
        "is_active INTEGER DEFAULT 1, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "last_login_at DATETIME)";

    if (!query.exec(createUsersTable)) {
        LOG_WARN("UserRepository: Failed to create users table: {}", query.lastError().text().toStdString());
        return false;
    }

    // 检查并添加缺失的列（数据库迁移）
    QStringList requiredColumns = {"display_name", "role", "permissions", "is_active", "last_login_at"};
    for (const QString& col : requiredColumns) {
        QSqlQuery checkQuery(db);
        checkQuery.exec(QString("SELECT %1 FROM users LIMIT 1").arg(col));
        if (checkQuery.lastError().isValid()) {
            // 列不存在，添加它
            QString alterSql = QString("ALTER TABLE users ADD COLUMN %1 TEXT").arg(col);
            if (col == "is_active") {
                alterSql = "ALTER TABLE users ADD COLUMN is_active INTEGER DEFAULT 1";
            }
            QSqlQuery alterQuery(db);
            if (!alterQuery.exec(alterSql)) {
                LOG_WARN("UserRepository: Failed to add column {}: {}", col.toStdString(), alterQuery.lastError().text().toStdString());
            } else {
                LOG_INFO("UserRepository: Added missing column: {}", col.toStdString());
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

    // 管理员
    UserInfo admin;
    admin.username = "admin";
    admin.passwordHash = hashPassword("admin123");
    admin.displayName = QString::fromUtf8("系统管理员");
    admin.role = "admin";
    admin.permissions = {"ViewHistory", "DeleteHistory", "ViewStatistics", "ExportData",
                         "RunDetection", "SystemSettings", "ManageUsers"};
    if (insert(admin) < 0) {
        LOG_WARN("UserRepository: Failed to create admin user");
        success = false;
    }

    // 操作员
    UserInfo op;
    op.username = "operator";
    op.passwordHash = hashPassword("operator");
    op.displayName = QString::fromUtf8("操作员");
    op.role = "operator";
    op.permissions = {"ViewHistory", "ViewStatistics", "RunDetection", "ExportData"};
    if (insert(op) < 0) {
        LOG_WARN("UserRepository: Failed to create operator user");
        success = false;
    }

    // 观察员
    UserInfo viewer;
    viewer.username = "viewer";
    viewer.passwordHash = hashPassword("viewer");
    viewer.displayName = QString::fromUtf8("观察员");
    viewer.role = "viewer";
    viewer.permissions = {"ViewHistory", "ViewStatistics"};
    if (insert(viewer) < 0) {
        LOG_WARN("UserRepository: Failed to create viewer user");
        success = false;
    }

    if (success) {
        LOG_INFO("UserRepository: Default users created successfully");
    }
    return success;
}

UserInfo UserRepository::findByUsername(const QString& username) {
    UserInfo user;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isOpen()) {
        LOG_WARN("UserRepository: Database not open");
        return user;
    }

    QSqlQuery query(db);
    QString sql = "SELECT id, username, password_hash, display_name, role, permissions, "
                  "is_active, created_at, last_login_at FROM users WHERE username = ? AND is_active = 1";

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
        user.id = query.value("id").toLongLong();
        user.username = query.value("username").toString();
        user.passwordHash = query.value("password_hash").toString();
        user.displayName = query.value("display_name").toString();
        user.role = query.value("role").toString();
        user.permissions = query.value("permissions").toString().split(",", Qt::SkipEmptyParts);
        user.isActive = query.value("is_active").toBool();
        user.createdAt = query.value("created_at").toDateTime();
        user.lastLoginAt = query.value("last_login_at").toDateTime();
        LOG_DEBUG("UserRepository: Found user: {} id: {}", user.username.toStdString(), user.id);
    } else {
        LOG_DEBUG("UserRepository: User not found: {}", username.toStdString());
    }

    return user;
}

UserInfo UserRepository::findById(qint64 id) {
    UserInfo user;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);

    QString sql = "SELECT id, username, password_hash, display_name, role, permissions, "
                  "is_active, created_at, last_login_at FROM users WHERE id = ?";
    if (!query.prepare(sql)) {
        return user;
    }
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        user.id = query.value("id").toLongLong();
        user.username = query.value("username").toString();
        user.passwordHash = query.value("password_hash").toString();
        user.displayName = query.value("display_name").toString();
        user.role = query.value("role").toString();
        user.permissions = query.value("permissions").toString().split(",", Qt::SkipEmptyParts);
        user.isActive = query.value("is_active").toBool();
        user.createdAt = query.value("created_at").toDateTime();
        user.lastLoginAt = query.value("last_login_at").toDateTime();
    }

    return user;
}

QVector<UserInfo> UserRepository::findAll() {
    QVector<UserInfo> users;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);

    QString sql = "SELECT id, username, password_hash, display_name, role, permissions, "
                  "is_active, created_at, last_login_at FROM users ORDER BY id";
    if (!query.prepare(sql)) {
        return users;
    }

    if (!query.exec()) {
        LOG_WARN("UserRepository: Failed to get users: {}", query.lastError().text().toStdString());
        return users;
    }

    while (query.next()) {
        UserInfo user;
        user.id = query.value("id").toLongLong();
        user.username = query.value("username").toString();
        user.passwordHash = query.value("password_hash").toString();
        user.displayName = query.value("display_name").toString();
        user.role = query.value("role").toString();
        user.permissions = query.value("permissions").toString().split(",", Qt::SkipEmptyParts);
        user.isActive = query.value("is_active").toBool();
        user.createdAt = query.value("created_at").toDateTime();
        user.lastLoginAt = query.value("last_login_at").toDateTime();
        users.append(user);
    }

    return users;
}

qint64 UserRepository::insert(const UserInfo& user) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isOpen()) {
        LOG_WARN("UserRepository: Database not open for insert");
        return -1;
    }

    QSqlQuery query(db);
    QString sql = "INSERT INTO users (username, password_hash, display_name, role, permissions, is_active) "
                  "VALUES (?, ?, ?, ?, ?, ?)";

    if (!query.prepare(sql)) {
        LOG_WARN("UserRepository: Failed to prepare insert: {}", query.lastError().text().toStdString());
        return -1;
    }

    query.addBindValue(user.username);
    query.addBindValue(user.passwordHash);
    query.addBindValue(user.displayName);
    query.addBindValue(user.role);
    query.addBindValue(user.permissions.join(","));
    query.addBindValue(user.isActive ? 1 : 0);

    if (!query.exec()) {
        LOG_WARN("UserRepository: Failed to insert user: {}", query.lastError().text().toStdString());
        return -1;
    }

    return query.lastInsertId().toLongLong();
}

bool UserRepository::update(const UserInfo& user) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
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

bool UserRepository::updatePassword(qint64 userId, const QString& passwordHash) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);

    if (!query.prepare("UPDATE users SET password_hash = ? WHERE id = ?")) {
        return false;
    }
    query.addBindValue(passwordHash);
    query.addBindValue(userId);

    return query.exec();
}

bool UserRepository::updateLastLogin(qint64 userId, const QDateTime& time) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);

    if (!query.prepare("UPDATE users SET last_login_at = ? WHERE id = ?")) {
        return false;
    }
    query.addBindValue(time);
    query.addBindValue(userId);

    return query.exec();
}

bool UserRepository::remove(qint64 userId) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);

    if (!query.prepare("DELETE FROM users WHERE id = ?")) {
        return false;
    }
    query.addBindValue(userId);

    return query.exec();
}

bool UserRepository::usernameExists(const QString& username) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
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

int UserRepository::count() {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);

    if (!query.prepare("SELECT COUNT(*) FROM users")) {
        return 0;
    }
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}
