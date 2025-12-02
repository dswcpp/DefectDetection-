#ifndef USERMANAGEMENTDIALOG_H
#define USERMANAGEMENTDIALOG_H

#include <QDialog>
#include "ui_global.h"
#include "data/repositories/UserRepository.h"

class QTableWidget;
class QLineEdit;
class QComboBox;
class QPushButton;
class QCheckBox;
class QGroupBox;

class UI_LIBRARY UserManagementDialog : public QDialog {
    Q_OBJECT
public:
    explicit UserManagementDialog(QWidget* parent = nullptr);

private slots:
    void onAddUser();
    void onEditUser();
    void onDeleteUser();
    void onResetPassword();
    void onUserSelected();
    void onRoleChanged(const QString& role);
    void refreshUserList();

private:
    void setupUI();
    void clearEditForm();
    void populateEditForm(const UserInfo& user);
    bool validateInput();
    QStringList getSelectedPermissions();

    // 表格
    QTableWidget* m_userTable;

    // 编辑区域
    QGroupBox* m_editGroup;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_displayNameEdit;
    QLineEdit* m_passwordEdit;
    QComboBox* m_roleCombo;
    QCheckBox* m_activeCheck;

    // 权限复选框
    QCheckBox* m_permViewHistory;
    QCheckBox* m_permDeleteHistory;
    QCheckBox* m_permViewStatistics;
    QCheckBox* m_permExportData;
    QCheckBox* m_permRunDetection;
    QCheckBox* m_permSystemSettings;
    QCheckBox* m_permManageUsers;

    // 按钮
    QPushButton* m_addBtn;
    QPushButton* m_saveBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_resetPwdBtn;

    qint64 m_editingUserId = 0;
    bool m_isNewUser = true;
};

#endif // USERMANAGEMENTDIALOG_H
