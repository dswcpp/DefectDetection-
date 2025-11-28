#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QSettings>
#include <QCryptographicHash>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setupUI();
    loadSettings();
}

QString LoginDialog::getUsername() const {
    return m_username ? m_username->text() : QString();
}

void LoginDialog::setupUI() {
    setWindowTitle(tr("系统登录"));
    setFixedSize(400, 480);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // 设置整体样式
    setStyleSheet(R"(
        QDialog {
            background-color: #f5f5f5;
        }
        QLineEdit {
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 6px;
            font-size: 14px;
            background-color: white;
        }
        QLineEdit:focus {
            border-color: #2196F3;
            outline: none;
        }
        QPushButton {
            padding: 10px;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
        }
    )");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(50, 40, 50, 40);

    // Logo和标题区域
    auto* headerWidget = new QWidget();
    headerWidget->setStyleSheet("background-color: transparent;");
    auto* headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setSpacing(10);

    // Logo（暂时用emoji代替）
    auto* logoLabel = new QLabel();
    logoLabel->setText("🔐");
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet("font-size: 48px;");
    headerLayout->addWidget(logoLabel);

    // 系统标题
    auto* titleLabel = new QLabel(tr("缺陷检测系统"));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        font-size: 20px;
        font-weight: bold;
        color: #333;
        margin-bottom: 5px;
    )");
    headerLayout->addWidget(titleLabel);

    // 副标题
    auto* subtitleLabel = new QLabel(tr("请登录以继续"));
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet(R"(
        font-size: 12px;
        color: #666;
    )");
    headerLayout->addWidget(subtitleLabel);

    mainLayout->addWidget(headerWidget);

    // 输入区域
    auto* formWidget = new QWidget();
    formWidget->setStyleSheet(R"(
        background-color: white;
        border-radius: 10px;
    )");

    // 添加阴影效果
    auto* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(20);
    shadow->setXOffset(0);
    shadow->setYOffset(2);
    shadow->setColor(QColor(0, 0, 0, 30));
    formWidget->setGraphicsEffect(shadow);

    auto* formLayout = new QVBoxLayout(formWidget);
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(30, 30, 30, 30);

    // 用户名输入框
    auto* userContainer = new QWidget();
    auto* userLayout = new QHBoxLayout(userContainer);
    userLayout->setContentsMargins(0, 0, 0, 0);
    userLayout->setSpacing(10);

    auto* userIcon = new QLabel("👤");
    userIcon->setFixedWidth(30);
    userIcon->setStyleSheet("font-size: 20px;");
    userLayout->addWidget(userIcon);

    m_username = new QLineEdit();
    m_username->setPlaceholderText(tr("请输入用户名"));
    m_username->setMaxLength(20);
    connect(m_username, &QLineEdit::textChanged, this, &LoginDialog::onUsernameChanged);
    userLayout->addWidget(m_username);

    formLayout->addWidget(userContainer);

    // 密码输入框
    auto* passContainer = new QWidget();
    auto* passLayout = new QHBoxLayout(passContainer);
    passLayout->setContentsMargins(0, 0, 0, 0);
    passLayout->setSpacing(10);

    auto* passIcon = new QLabel("🔑");
    passIcon->setFixedWidth(30);
    passIcon->setStyleSheet("font-size: 20px;");
    passLayout->addWidget(passIcon);

    m_password = new QLineEdit();
    m_password->setEchoMode(QLineEdit::Password);
    m_password->setPlaceholderText(tr("请输入密码"));
    m_password->setMaxLength(20);
    connect(m_password, &QLineEdit::textChanged, this, &LoginDialog::onPasswordChanged);
    passLayout->addWidget(m_password);

    formLayout->addWidget(passContainer);

    // 记住密码选项
    m_rememberMe = new QCheckBox(tr("记住用户名"));
    m_rememberMe->setStyleSheet(R"(
        QCheckBox {
            color: #666;
            font-size: 13px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
        }
    )");
    formLayout->addWidget(m_rememberMe);

    // 错误提示标签
    m_errorLabel = new QLabel();
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setStyleSheet(R"(
        color: #f44336;
        font-size: 12px;
        padding: 5px;
        background-color: #ffebee;
        border-radius: 4px;
    )");
    m_errorLabel->setVisible(false);
    formLayout->addWidget(m_errorLabel);

    // 按钮区域
    auto* btnContainer = new QWidget();
    auto* btnLayout = new QHBoxLayout(btnContainer);
    btnLayout->setSpacing(15);
    btnLayout->setContentsMargins(0, 10, 0, 0);

    m_loginBtn = new QPushButton(tr("登 录"));
    m_loginBtn->setDefault(true);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #2196F3;
            color: white;
            min-height: 38px;
        }
        QPushButton:hover {
            background-color: #1976D2;
        }
        QPushButton:pressed {
            background-color: #0D47A1;
        }
        QPushButton:disabled {
            background-color: #BBDEFB;
        }
    )");
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    btnLayout->addWidget(m_loginBtn);

    m_cancelBtn = new QPushButton(tr("退 出"));
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #757575;
            color: white;
            min-height: 38px;
        }
        QPushButton:hover {
            background-color: #616161;
        }
        QPushButton:pressed {
            background-color: #424242;
        }
    )");
    connect(m_cancelBtn, &QPushButton::clicked, this, &LoginDialog::onCancelClicked);
    btnLayout->addWidget(m_cancelBtn);

    formLayout->addWidget(btnContainer);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();

    // 底部版权信息
    auto* footerLabel = new QLabel(tr("© 2025 缺陷检测系统 v1.0"));
    footerLabel->setAlignment(Qt::AlignCenter);
    footerLabel->setStyleSheet(R"(
        color: #999;
        font-size: 11px;
    )");
    mainLayout->addWidget(footerLabel);

    // 设置Tab顺序
    setTabOrder(m_username, m_password);
    setTabOrder(m_password, m_rememberMe);
    setTabOrder(m_rememberMe, m_loginBtn);
    setTabOrder(m_loginBtn, m_cancelBtn);

    // 默认焦点
    m_username->setFocus();
}

void LoginDialog::onLoginClicked() {
    clearError();

    QString username = m_username->text().trimmed();
    QString password = m_password->text();

    // 基本验证
    if (username.isEmpty()) {
        showError(tr("请输入用户名"));
        m_username->setFocus();
        return;
    }

    if (password.isEmpty()) {
        showError(tr("请输入密码"));
        m_password->setFocus();
        return;
    }

    // 验证凭据
    if (validateCredentials(username, password)) {
        // 保存设置
        if (m_rememberMe->isChecked()) {
            saveSettings();
        }
        accept();  // 关闭对话框，返回Accepted
    } else {
        m_loginAttempts++;

        if (m_loginAttempts >= MAX_LOGIN_ATTEMPTS) {
            showError(tr("登录失败次数过多，请稍后再试"));
            m_loginBtn->setEnabled(false);

            // 30秒后重新启用登录按钮
            QTimer::singleShot(30000, this, [this]() {
                m_loginBtn->setEnabled(true);
                m_loginAttempts = 0;
                clearError();
            });
        } else {
            showError(tr("用户名或密码错误 (尝试 %1/%2)")
                .arg(m_loginAttempts)
                .arg(MAX_LOGIN_ATTEMPTS));
            m_password->clear();
            m_password->setFocus();
        }
    }
}

void LoginDialog::onCancelClicked() {
    reject();  // 关闭对话框，返回Rejected
}

void LoginDialog::onUsernameChanged(const QString& text) {
    Q_UNUSED(text)
    clearError();
}

void LoginDialog::onPasswordChanged(const QString& text) {
    Q_UNUSED(text)
    clearError();
}

bool LoginDialog::validateCredentials(const QString& username, const QString& password) {
    // 这里使用简单的硬编码验证，实际应用中应该查询数据库或调用认证服务

    // 计算密码的MD5哈希（仅用于演示，实际应用应使用更安全的哈希算法）
    QString hashedPassword = QString(QCryptographicHash::hash(
        password.toUtf8(),
        QCryptographicHash::Md5).toHex());

    // 预设的用户账号（实际应用中应从数据库读取）
    struct User {
        QString username;
        QString passwordHash;  // MD5哈希
        QString role;
    };

    QList<User> users = {
        {"admin", "21232f297a57a5a743894a0e4a801fc3", "管理员"},  // 密码: admin
        {"operator", "4b583376b2767b923c3e1da60d10de59", "操作员"}, // 密码: operator
        {"viewer", "feb8bc875e33bc5f8ae0e97cf78bb68f", "观察员"},   // 密码: viewer
        {"test", "098f6bcd4621d373cade4e832627b4f6", "测试员"}     // 密码: test
    };

    // 查找匹配的用户
    for (const auto& user : users) {
        if (user.username == username && user.passwordHash == hashedPassword) {
            m_role = user.role;
            return true;
        }
    }

    return false;
}

void LoginDialog::showError(const QString& message) {
    m_errorLabel->setText(message);
    m_errorLabel->setVisible(true);

    // 添加简单的动画效果
    QPropertyAnimation* animation = new QPropertyAnimation(m_errorLabel, "geometry");
    animation->setDuration(100);
    QRect startRect = m_errorLabel->geometry();
    QRect endRect = startRect;
    endRect.translate(-5, 0);
    animation->setStartValue(startRect);
    animation->setKeyValueAt(0.25, endRect);
    endRect.translate(10, 0);
    animation->setKeyValueAt(0.75, endRect);
    animation->setEndValue(startRect);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void LoginDialog::clearError() {
    m_errorLabel->clear();
    m_errorLabel->setVisible(false);
}

void LoginDialog::loadSettings() {
    QSettings settings("DefectDetection", "Login");

    if (settings.value("rememberMe", false).toBool()) {
        m_username->setText(settings.value("username").toString());
        m_rememberMe->setChecked(true);
    }
}

void LoginDialog::saveSettings() {
    QSettings settings("DefectDetection", "Login");

    settings.setValue("rememberMe", m_rememberMe->isChecked());
    if (m_rememberMe->isChecked()) {
        settings.setValue("username", m_username->text());
    } else {
        settings.remove("username");
    }
}
