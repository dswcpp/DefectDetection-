#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "ui_global.h"
#include "Types.h"

class DatabaseManager;
class UserRepository;
struct UserInfo;

// 用户管理服务（单例）- 处理业务逻辑
class UI_LIBRARY UserManager : public QObject {
    Q_OBJECT
public:
    static UserManager* instance();

    void setDatabaseManager(DatabaseManager* dbManager);
    bool initialize();

    // 登录/登出
    bool login(const QString& username, const QString& password);
    void logout();
    bool isLoggedIn() const { return m_isLoggedIn; }

    // 当前用户信息
    QString currentUsername() const;
    QString currentDisplayName() const;
    QString currentRole() const;
    qint64 currentUserId() const;

    // 权限检查
    bool hasPermission(Permission perm) const;
    bool hasPermission(const QString& permName) const;
    QStringList currentPermissions() const;

    // 用户管理（需要 ManageUsers 权限）
    QVector<UserInfo> getAllUsers();
    bool createUser(const QString& username, const QString& password,
                    const QString& displayName, const QString& role,
                    const QStringList& permissions);
    bool updateUser(qint64 userId, const QString& displayName,
                    const QString& role, const QStringList& permissions, bool isActive);
    bool deleteUser(qint64 userId);
    bool resetPassword(qint64 userId, const QString& newPassword);

    // 当前用户改密码
    bool changeOwnPassword(const QString& oldPassword, const QString& newPassword);

    // 权限名称转换
    static QString permissionName(Permission perm);
    static QString permissionDisplayName(Permission perm);
    static Permission permissionFromName(const QString& name);
    static QStringList allPermissionNames();

    // 预设角色权限
    static QStringList presetRoles();
    static QStringList permissionsForRole(const QString& role);

signals:
    void loginSucceeded(const QString& username);
    void loginFailed(const QString& reason);
    void loggedOut();
    void userUpdated();

private:
    explicit UserManager(QObject* parent = nullptr);
    ~UserManager() = default;

    static UserManager* s_instance;
    DatabaseManager* m_dbManager = nullptr;
    UserRepository* m_userRepo = nullptr;

    // 当前登录用户缓存
    qint64 m_currentUserId = 0;
    QString m_currentUsername;
    QString m_currentDisplayName;
    QString m_currentRole;
    QStringList m_currentPermissions;
    bool m_isLoggedIn = false;
};

#endif // USERMANAGER_H
