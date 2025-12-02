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
    static const int MAX_LOGIN_ATTEMPTS = 5;
};

#endif // LOGINDIALOG_H
