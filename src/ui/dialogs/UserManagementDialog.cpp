#include "UserManagementDialog.h"
#include "services/UserManager.h"
#include "data/repositories/UserRepository.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>

UserManagementDialog::UserManagementDialog(QWidget* parent)
    : QDialog(parent) {
    setupUI();
    refreshUserList();
}

void UserManagementDialog::setupUI() {
    setWindowTitle(tr("用户管理"));
    setMinimumSize(900, 600);
    resize(1000, 650);

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ==================== 左侧：用户列表 ====================
    auto* leftPanel = new QWidget();
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(8);

    auto* listTitle = new QLabel(tr("用户列表"));
    listTitle->setStyleSheet("font-weight: bold; font-size: 14px;");
    leftLayout->addWidget(listTitle);

    m_userTable = new QTableWidget();
    m_userTable->setColumnCount(5);
    m_userTable->setHorizontalHeaderLabels({tr("用户名"), tr("显示名"), tr("角色"), tr("状态"), tr("最后登录")});
    m_userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_userTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_userTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_userTable->horizontalHeader()->setStretchLastSection(true);
    m_userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_userTable->verticalHeader()->setVisible(false);
    m_userTable->setStyleSheet(R"(
        QTableWidget {
            border: 1px solid #dee2e6;
            border-radius: 4px;
            background-color: white;
        }
        QTableWidget::item:selected {
            background-color: #cfe2ff;
            color: #084298;
        }
        QHeaderView::section {
            background-color: #f8f9fa;
            padding: 8px;
            border: none;
            border-bottom: 1px solid #dee2e6;
            font-weight: 500;
        }
    )");
    connect(m_userTable, &QTableWidget::itemSelectionChanged, this, &UserManagementDialog::onUserSelected);
    leftLayout->addWidget(m_userTable, 1);

    // 按钮栏
    auto* btnBar = new QHBoxLayout();
    m_addBtn = new QPushButton(tr("新增用户"));
    m_addBtn->setStyleSheet(R"(
        QPushButton { background-color: #28a745; color: white; border: none; border-radius: 4px; padding: 8px 16px; }
        QPushButton:hover { background-color: #218838; }
    )");
    connect(m_addBtn, &QPushButton::clicked, this, &UserManagementDialog::onAddUser);
    btnBar->addWidget(m_addBtn);

    m_deleteBtn = new QPushButton(tr("删除用户"));
    m_deleteBtn->setEnabled(false);
    m_deleteBtn->setStyleSheet(R"(
        QPushButton { background-color: #dc3545; color: white; border: none; border-radius: 4px; padding: 8px 16px; }
        QPushButton:hover { background-color: #c82333; }
        QPushButton:disabled { background-color: #f5c6cb; }
    )");
    connect(m_deleteBtn, &QPushButton::clicked, this, &UserManagementDialog::onDeleteUser);
    btnBar->addWidget(m_deleteBtn);

    m_resetPwdBtn = new QPushButton(tr("重置密码"));
    m_resetPwdBtn->setEnabled(false);
    m_resetPwdBtn->setStyleSheet(R"(
        QPushButton { background-color: #ffc107; color: #212529; border: none; border-radius: 4px; padding: 8px 16px; }
        QPushButton:hover { background-color: #e0a800; }
        QPushButton:disabled { background-color: #fff3cd; }
    )");
    connect(m_resetPwdBtn, &QPushButton::clicked, this, &UserManagementDialog::onResetPassword);
    btnBar->addWidget(m_resetPwdBtn);

    btnBar->addStretch();
    leftLayout->addLayout(btnBar);

    mainLayout->addWidget(leftPanel, 1);

    // ==================== 右侧：编辑面板 ====================
    m_editGroup = new QGroupBox(tr("用户信息"));
    m_editGroup->setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 1px solid #dee2e6;
            border-radius: 4px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )");
    auto* editLayout = new QVBoxLayout(m_editGroup);
    editLayout->setSpacing(12);
    editLayout->setContentsMargins(16, 20, 16, 16);

    auto* formLayout = new QGridLayout();
    formLayout->setSpacing(8);

    // 用户名
    formLayout->addWidget(new QLabel(tr("用户名:")), 0, 0);
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText(tr("登录用户名"));
    m_usernameEdit->setStyleSheet("QLineEdit { padding: 6px; border: 1px solid #ced4da; border-radius: 4px; }");
    formLayout->addWidget(m_usernameEdit, 0, 1);

    // 显示名
    formLayout->addWidget(new QLabel(tr("显示名:")), 1, 0);
    m_displayNameEdit = new QLineEdit();
    m_displayNameEdit->setPlaceholderText(tr("用户显示名称"));
    m_displayNameEdit->setStyleSheet(m_usernameEdit->styleSheet());
    formLayout->addWidget(m_displayNameEdit, 1, 1);

    // 密码（仅新增时显示）
    formLayout->addWidget(new QLabel(tr("密码:")), 2, 0);
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("初始密码"));
    m_passwordEdit->setStyleSheet(m_usernameEdit->styleSheet());
    formLayout->addWidget(m_passwordEdit, 2, 1);

    // 角色
    formLayout->addWidget(new QLabel(tr("角色:")), 3, 0);
    m_roleCombo = new QComboBox();
    m_roleCombo->addItem(tr("管理员"), "admin");
    m_roleCombo->addItem(tr("操作员"), "operator");
    m_roleCombo->addItem(tr("观察员"), "viewer");
    m_roleCombo->setStyleSheet("QComboBox { padding: 6px; border: 1px solid #ced4da; border-radius: 4px; }");
    connect(m_roleCombo, &QComboBox::currentTextChanged, this, &UserManagementDialog::onRoleChanged);
    formLayout->addWidget(m_roleCombo, 3, 1);

    // 状态
    m_activeCheck = new QCheckBox(tr("启用账户"));
    m_activeCheck->setChecked(true);
    formLayout->addWidget(m_activeCheck, 4, 1);

    editLayout->addLayout(formLayout);

    // 权限区域
    auto* permGroup = new QGroupBox(tr("权限设置"));
    permGroup->setStyleSheet("QGroupBox { font-weight: normal; }");
    auto* permLayout = new QGridLayout(permGroup);
    permLayout->setSpacing(8);

    m_permViewHistory = new QCheckBox(tr("查看历史记录"));
    m_permDeleteHistory = new QCheckBox(tr("删除历史记录"));
    m_permViewStatistics = new QCheckBox(tr("查看统计"));
    m_permExportData = new QCheckBox(tr("导出数据"));
    m_permRunDetection = new QCheckBox(tr("运行检测"));
    m_permSystemSettings = new QCheckBox(tr("系统设置"));
    m_permManageUsers = new QCheckBox(tr("用户管理"));

    permLayout->addWidget(m_permViewHistory, 0, 0);
    permLayout->addWidget(m_permDeleteHistory, 0, 1);
    permLayout->addWidget(m_permViewStatistics, 1, 0);
    permLayout->addWidget(m_permExportData, 1, 1);
    permLayout->addWidget(m_permRunDetection, 2, 0);
    permLayout->addWidget(m_permSystemSettings, 2, 1);
    permLayout->addWidget(m_permManageUsers, 3, 0);

    editLayout->addWidget(permGroup);

    // 保存按钮
    m_saveBtn = new QPushButton(tr("保存"));
    m_saveBtn->setStyleSheet(R"(
        QPushButton { background-color: #007bff; color: white; border: none; border-radius: 4px; padding: 10px; font-weight: bold; }
        QPushButton:hover { background-color: #0056b3; }
    )");
    connect(m_saveBtn, &QPushButton::clicked, this, &UserManagementDialog::onEditUser);
    editLayout->addWidget(m_saveBtn);

    editLayout->addStretch();

    mainLayout->addWidget(m_editGroup);

    clearEditForm();
}

void UserManagementDialog::refreshUserList() {
    m_userTable->setRowCount(0);
    auto users = UserManager::instance()->getAllUsers();

    for (const auto& user : users) {
        int row = m_userTable->rowCount();
        m_userTable->insertRow(row);

        auto* usernameItem = new QTableWidgetItem(user.username);
        usernameItem->setData(Qt::UserRole, user.id);
        m_userTable->setItem(row, 0, usernameItem);
        m_userTable->setItem(row, 1, new QTableWidgetItem(user.displayName));

        QString roleDisplay = user.role == "admin" ? tr("管理员") :
                              user.role == "operator" ? tr("操作员") : tr("观察员");
        m_userTable->setItem(row, 2, new QTableWidgetItem(roleDisplay));

        auto* statusItem = new QTableWidgetItem(user.isActive ? tr("启用") : tr("禁用"));
        statusItem->setForeground(user.isActive ? QColor("#28a745") : QColor("#dc3545"));
        m_userTable->setItem(row, 3, statusItem);

        QString lastLogin = user.lastLoginAt.isValid() ?
                            user.lastLoginAt.toString("yyyy-MM-dd hh:mm") : tr("从未");
        m_userTable->setItem(row, 4, new QTableWidgetItem(lastLogin));
    }
}

void UserManagementDialog::onUserSelected() {
    auto items = m_userTable->selectedItems();
    if (items.isEmpty()) {
        clearEditForm();
        m_deleteBtn->setEnabled(false);
        m_resetPwdBtn->setEnabled(false);
        return;
    }

    int row = items.first()->row();
    qint64 userId = m_userTable->item(row, 0)->data(Qt::UserRole).toLongLong();

    auto users = UserManager::instance()->getAllUsers();
    for (const auto& user : users) {
        if (user.id == userId) {
            populateEditForm(user);
            break;
        }
    }

    // 不能删除自己或admin账户
    bool canDelete = (userId != UserManager::instance()->currentUserId()) &&
                     (m_userTable->item(row, 0)->text() != "admin");
    m_deleteBtn->setEnabled(canDelete);
    m_resetPwdBtn->setEnabled(true);
}

void UserManagementDialog::onAddUser() {
    clearEditForm();
    m_isNewUser = true;
    m_editingUserId = 0;
    m_usernameEdit->setEnabled(true);
    m_passwordEdit->setEnabled(true);
    m_usernameEdit->setFocus();
}

void UserManagementDialog::onEditUser() {
    if (!validateInput()) return;

    QString username = m_usernameEdit->text().trimmed();
    QString displayName = m_displayNameEdit->text().trimmed();
    QString role = m_roleCombo->currentData().toString();
    bool isActive = m_activeCheck->isChecked();
    QStringList permissions = getSelectedPermissions();

    bool success = false;
    if (m_isNewUser) {
        QString password = m_passwordEdit->text();
        if (password.isEmpty()) {
            QMessageBox::warning(this, tr("错误"), tr("请输入初始密码"));
            return;
        }
        success = UserManager::instance()->createUser(username, password, displayName, role, permissions);
        if (!success) {
            QMessageBox::warning(this, tr("错误"), tr("创建用户失败，用户名可能已存在"));
            return;
        }
    } else {
        success = UserManager::instance()->updateUser(m_editingUserId, displayName, role, permissions, isActive);
        if (!success) {
            QMessageBox::warning(this, tr("错误"), tr("更新用户失败"));
            return;
        }
    }

    QMessageBox::information(this, tr("成功"), m_isNewUser ? tr("用户创建成功") : tr("用户更新成功"));
    refreshUserList();
    clearEditForm();
}

void UserManagementDialog::onDeleteUser() {
    auto items = m_userTable->selectedItems();
    if (items.isEmpty()) return;

    int row = items.first()->row();
    QString username = m_userTable->item(row, 0)->text();
    qint64 userId = m_userTable->item(row, 0)->data(Qt::UserRole).toLongLong();

    auto result = QMessageBox::question(this, tr("确认删除"),
                                         tr("确定要删除用户 \"%1\" 吗？").arg(username));
    if (result != QMessageBox::Yes) return;

    if (UserManager::instance()->deleteUser(userId)) {
        refreshUserList();
        clearEditForm();
    } else {
        QMessageBox::warning(this, tr("错误"), tr("删除用户失败"));
    }
}

void UserManagementDialog::onResetPassword() {
    auto items = m_userTable->selectedItems();
    if (items.isEmpty()) return;

    int row = items.first()->row();
    QString username = m_userTable->item(row, 0)->text();
    qint64 userId = m_userTable->item(row, 0)->data(Qt::UserRole).toLongLong();

    bool ok;
    QString newPassword = QInputDialog::getText(this, tr("重置密码"),
                                                  tr("请输入 \"%1\" 的新密码:").arg(username),
                                                  QLineEdit::Password, "", &ok);
    if (!ok || newPassword.isEmpty()) return;

    if (UserManager::instance()->resetPassword(userId, newPassword)) {
        QMessageBox::information(this, tr("成功"), tr("密码重置成功"));
    } else {
        QMessageBox::warning(this, tr("错误"), tr("密码重置失败"));
    }
}

void UserManagementDialog::onRoleChanged(const QString& role) {
    Q_UNUSED(role)
    QString roleData = m_roleCombo->currentData().toString();
    QStringList perms = UserManager::permissionsForRole(roleData);

    m_permViewHistory->setChecked(perms.contains("ViewHistory"));
    m_permDeleteHistory->setChecked(perms.contains("DeleteHistory"));
    m_permViewStatistics->setChecked(perms.contains("ViewStatistics"));
    m_permExportData->setChecked(perms.contains("ExportData"));
    m_permRunDetection->setChecked(perms.contains("RunDetection"));
    m_permSystemSettings->setChecked(perms.contains("SystemSettings"));
    m_permManageUsers->setChecked(perms.contains("ManageUsers"));
}

void UserManagementDialog::clearEditForm() {
    m_isNewUser = true;
    m_editingUserId = 0;
    m_usernameEdit->clear();
    m_usernameEdit->setEnabled(true);
    m_displayNameEdit->clear();
    m_passwordEdit->clear();
    m_passwordEdit->setEnabled(true);
    m_roleCombo->setCurrentIndex(1);  // operator
    m_activeCheck->setChecked(true);
    onRoleChanged("");
}

void UserManagementDialog::populateEditForm(const UserInfo& user) {
    m_isNewUser = false;
    m_editingUserId = user.id;
    m_usernameEdit->setText(user.username);
    m_usernameEdit->setEnabled(false);  // 不能修改用户名
    m_displayNameEdit->setText(user.displayName);
    m_passwordEdit->clear();
    m_passwordEdit->setEnabled(false);  // 编辑时不显示密码

    int roleIndex = m_roleCombo->findData(user.role);
    if (roleIndex >= 0) m_roleCombo->setCurrentIndex(roleIndex);

    m_activeCheck->setChecked(user.isActive);

    m_permViewHistory->setChecked(user.permissions.contains("ViewHistory"));
    m_permDeleteHistory->setChecked(user.permissions.contains("DeleteHistory"));
    m_permViewStatistics->setChecked(user.permissions.contains("ViewStatistics"));
    m_permExportData->setChecked(user.permissions.contains("ExportData"));
    m_permRunDetection->setChecked(user.permissions.contains("RunDetection"));
    m_permSystemSettings->setChecked(user.permissions.contains("SystemSettings"));
    m_permManageUsers->setChecked(user.permissions.contains("ManageUsers"));
}

bool UserManagementDialog::validateInput() {
    if (m_usernameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请输入用户名"));
        return false;
    }
    if (m_displayNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请输入显示名"));
        return false;
    }
    return true;
}

QStringList UserManagementDialog::getSelectedPermissions() {
    QStringList perms;
    if (m_permViewHistory->isChecked()) perms << "ViewHistory";
    if (m_permDeleteHistory->isChecked()) perms << "DeleteHistory";
    if (m_permViewStatistics->isChecked()) perms << "ViewStatistics";
    if (m_permExportData->isChecked()) perms << "ExportData";
    if (m_permRunDetection->isChecked()) perms << "RunDetection";
    if (m_permSystemSettings->isChecked()) perms << "SystemSettings";
    if (m_permManageUsers->isChecked()) perms << "ManageUsers";
    return perms;
}
