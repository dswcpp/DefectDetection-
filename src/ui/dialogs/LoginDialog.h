#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QCheckBox;
class QLabel;
class QPushButton;
QT_END_NAMESPACE

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);
    ~LoginDialog() = default;

    QString getUsername() const;
    QString getRole() const { return m_role; }

private slots:
    void onLoginClicked();
    void onCancelClicked();
    void onUsernameChanged(const QString& text);
    void onPasswordChanged(const QString& text);

private:
    void setupUI();
    bool validateCredentials(const QString& username, const QString& password);
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
    QString m_role;
    int m_loginAttempts = 0;
    static const int MAX_LOGIN_ATTEMPTS = 3;
};

#endif // LOGINDIALOG_H
