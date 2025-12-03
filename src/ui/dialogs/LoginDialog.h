/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * LoginDialog.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：登录对话框接口定义
 * 描述：用户登录对话框，输入用户名密码进行身份验证
 *
 * 当前版本：1.0
 */

#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "ui_global.h"

QT_BEGIN_NAMESPACE
class QLineEdit;
class QCheckBox;
class QLabel;
class QPushButton;
QT_END_NAMESPACE

class DatabaseManager;

class UI_LIBRARY LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);
    ~LoginDialog() = default;

    void setDatabaseManager(DatabaseManager* dbManager);
    QString getUsername() const;
    QString getRole() const;

private slots:
    void onLoginClicked();
    void onCancelClicked();
    void onUsernameChanged(const QString& text);
    void onPasswordChanged(const QString& text);

private:
    void setupUI();
    void showError(const QString& message);
    void clearError();
    void loadSettings();
    void saveSettings();

    // UI元素
    QLineEdit* m_username;
    QLineEdit* m_password;
    QCheckBox* m_rememberMe;
    QLabel* m_errorLabel;
    QPushButton* m_loginBtn;
    QPushButton* m_cancelBtn;

    // 数据
    DatabaseManager* m_dbManager = nullptr;
    int m_loginAttempts = 0;
    static constexpr int MAX_LOGIN_ATTEMPTS = 5;
};

#endif // LOGINDIALOG_H
