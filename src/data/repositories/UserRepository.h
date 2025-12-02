#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include "IRepository.h"
#include "../data_global.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QDateTime>

// 用户信息结构
struct DATA_EXPORT UserInfo {
    qint64 id = 0;
    QString username;
    QString passwordHash;
    QString displayName;
    QString role;
    QStringList permissions;
    bool isActive = true;
    QDateTime createdAt;
    QDateTime lastLoginAt;
};

class DATA_EXPORT UserRepository : public QObject, public IRepository {
    Q_OBJECT
public:
    explicit UserRepository(const QString& connectionName, QObject* parent = nullptr);

    QString name() const override { return "UserRepository"; }

    // 初始化
    bool ensureTablesExist();
    bool createDefaultUsers();

    // 查询
    UserInfo findByUsername(const QString& username);
    UserInfo findById(qint64 id);
    QVector<UserInfo> findAll();
    bool usernameExists(const QString& username);
    int count();

    // 增删改
    qint64 insert(const UserInfo& user);
    bool update(const UserInfo& user);
    bool updatePassword(qint64 userId, const QString& passwordHash);
    bool updateLastLogin(qint64 userId, const QDateTime& time);
    bool remove(qint64 userId);

    // 密码工具
    static QString hashPassword(const QString& password);
    static bool verifyPassword(const QString& password, const QString& hash);

private:
    QString m_connectionName;
};

#endif // USERREPOSITORY_H
