#include "UserManager.h"
#include "data/DatabaseManager.h"
#include "data/repositories/UserRepository.h"
#include "Logger.h"

UserManager* UserManager::s_instance = nullptr;

UserManager* UserManager::instance() {
    if (!s_instance) {
        s_instance = new UserManager();
    }
    return s_instance;
}

UserManager::UserManager(QObject* parent) : QObject(parent) {}

void UserManager::setDatabaseManager(DatabaseManager* dbManager) {
    m_dbManager = dbManager;
    if (m_dbManager) {
        m_userRepo = m_dbManager->userRepository();
    }
}

bool UserManager::initialize() {
    if (!m_userRepo) {
        LOG_WARN("UserManager: UserRepository not available");
        return false;
    }

    if (!m_userRepo->ensureTablesExist()) {
        return false;
    }

    return m_userRepo->createDefaultUsers();
}

bool UserManager::login(const QString& username, const QString& password) {
    LOG_DEBUG("UserManager::login called for: {}", username.toStdString());

    if (!m_userRepo) {
        LOG_WARN("UserManager: userRepo is null");
        emit loginFailed(tr("数据库未连接"));
        return false;
    }

    UserInfo user = m_userRepo->findByUsername(username);
    LOG_DEBUG("UserManager: findByUsername returned id: {}", user.id);

    if (user.id == 0) {
        LOG_DEBUG("UserManager: User not found");
        emit loginFailed(tr("用户名或密码错误"));
        return false;
    }

    if (!user.isActive) {
        LOG_DEBUG("UserManager: User is inactive");
        emit loginFailed(tr("账户已禁用"));
        return false;
    }

    if (!UserRepository::verifyPassword(password, user.passwordHash)) {
        LOG_DEBUG("UserManager: Password verification failed");
        emit loginFailed(tr("用户名或密码错误"));
        return false;
    }

    // 缓存当前用户信息
    m_currentUserId = user.id;
    m_currentUsername = user.username;
    m_currentDisplayName = user.displayName;
    m_currentRole = user.role;
    m_currentPermissions = user.permissions;
    m_isLoggedIn = true;

    // 更新最后登录时间
    m_userRepo->updateLastLogin(user.id, QDateTime::currentDateTime());

    emit loginSucceeded(m_currentUsername);
    return true;
}

void UserManager::logout() {
    m_currentUserId = 0;
    m_currentUsername.clear();
    m_currentDisplayName.clear();
    m_currentRole.clear();
    m_currentPermissions.clear();
    m_isLoggedIn = false;
    emit loggedOut();
}

QString UserManager::currentUsername() const { return m_currentUsername; }
QString UserManager::currentDisplayName() const { return m_currentDisplayName; }
QString UserManager::currentRole() const { return m_currentRole; }
qint64 UserManager::currentUserId() const { return m_currentUserId; }
QStringList UserManager::currentPermissions() const { return m_currentPermissions; }

bool UserManager::hasPermission(Permission perm) const {
    if (!m_isLoggedIn) return false;
    if (m_currentRole == "admin") return true;
    return m_currentPermissions.contains(permissionName(perm));
}

bool UserManager::hasPermission(const QString& permName) const {
    if (!m_isLoggedIn) return false;
    if (m_currentRole == "admin") return true;
    return m_currentPermissions.contains(permName);
}

QVector<UserInfo> UserManager::getAllUsers() {
    if (!m_userRepo) return {};
    return m_userRepo->findAll();
}

bool UserManager::createUser(const QString& username, const QString& password,
                              const QString& displayName, const QString& role,
                              const QStringList& permissions) {
    if (!m_userRepo) return false;

    if (m_userRepo->usernameExists(username)) {
        return false;
    }

    UserInfo user;
    user.username = username;
    user.passwordHash = UserRepository::hashPassword(password);
    user.displayName = displayName;
    user.role = role;
    user.permissions = permissions;
    user.isActive = true;

    qint64 id = m_userRepo->insert(user);
    if (id > 0) {
        emit userUpdated();
        return true;
    }
    return false;
}

bool UserManager::updateUser(qint64 userId, const QString& displayName,
                              const QString& role, const QStringList& permissions, bool isActive) {
    if (!m_userRepo) return false;

    UserInfo user = m_userRepo->findById(userId);
    if (user.id == 0) return false;

    user.displayName = displayName;
    user.role = role;
    user.permissions = permissions;
    user.isActive = isActive;

    if (m_userRepo->update(user)) {
        emit userUpdated();
        return true;
    }
    return false;
}

bool UserManager::deleteUser(qint64 userId) {
    if (!m_userRepo) return false;
    if (userId == m_currentUserId) return false;  // 不能删除自己

    if (m_userRepo->remove(userId)) {
        emit userUpdated();
        return true;
    }
    return false;
}

bool UserManager::resetPassword(qint64 userId, const QString& newPassword) {
    if (!m_userRepo) return false;
    QString hash = UserRepository::hashPassword(newPassword);
    return m_userRepo->updatePassword(userId, hash);
}

bool UserManager::changeOwnPassword(const QString& oldPassword, const QString& newPassword) {
    if (!m_isLoggedIn || !m_userRepo) return false;

    UserInfo user = m_userRepo->findById(m_currentUserId);
    if (!UserRepository::verifyPassword(oldPassword, user.passwordHash)) {
        return false;
    }

    QString hash = UserRepository::hashPassword(newPassword);
    return m_userRepo->updatePassword(m_currentUserId, hash);
}

QString UserManager::permissionName(Permission perm) {
    static QMap<Permission, QString> names = {
        {Permission::ViewHistory, "ViewHistory"},
        {Permission::DeleteHistory, "DeleteHistory"},
        {Permission::ViewStatistics, "ViewStatistics"},
        {Permission::ExportData, "ExportData"},
        {Permission::RunDetection, "RunDetection"},
        {Permission::SystemSettings, "SystemSettings"},
        {Permission::ManageUsers, "ManageUsers"}
    };
    return names.value(perm, "");
}

QString UserManager::permissionDisplayName(Permission perm) {
    static QMap<Permission, QString> names = {
        {Permission::ViewHistory, QObject::tr("查看历史记录")},
        {Permission::DeleteHistory, QObject::tr("删除历史记录")},
        {Permission::ViewStatistics, QObject::tr("查看统计")},
        {Permission::ExportData, QObject::tr("导出数据")},
        {Permission::RunDetection, QObject::tr("运行检测")},
        {Permission::SystemSettings, QObject::tr("系统设置")},
        {Permission::ManageUsers, QObject::tr("用户管理")}
    };
    return names.value(perm, "");
}

Permission UserManager::permissionFromName(const QString& name) {
    static QMap<QString, Permission> perms = {
        {"ViewHistory", Permission::ViewHistory},
        {"DeleteHistory", Permission::DeleteHistory},
        {"ViewStatistics", Permission::ViewStatistics},
        {"ExportData", Permission::ExportData},
        {"RunDetection", Permission::RunDetection},
        {"SystemSettings", Permission::SystemSettings},
        {"ManageUsers", Permission::ManageUsers}
    };
    return perms.value(name, Permission::ViewHistory);
}

QStringList UserManager::allPermissionNames() {
    return {"ViewHistory", "DeleteHistory", "ViewStatistics", "ExportData",
            "RunDetection", "SystemSettings", "ManageUsers"};
}

QStringList UserManager::presetRoles() {
    return {"admin", "operator", "viewer"};
}

QStringList UserManager::permissionsForRole(const QString& role) {
    if (role == "admin") {
        return allPermissionNames();
    } else if (role == "operator") {
        return {"ViewHistory", "ViewStatistics", "RunDetection", "ExportData"};
    } else if (role == "viewer") {
        return {"ViewHistory", "ViewStatistics"};
    }
    return {};
}
