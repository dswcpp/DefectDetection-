#include "UserSettingsPage.h"
#include "SettingsPageUtils.h"
#include "services/UserManager.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

using namespace SettingsUtils;

UserSettingsPage::UserSettingsPage(QWidget* parent) : QWidget(parent) {
  setupUI();
}

void UserSettingsPage::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(24);

  // 当前用户信息
  auto* currentUserGroup = createStyledGroupBox(tr("当前用户"), this);
  auto* userLayout = new QHBoxLayout(currentUserGroup);
  userLayout->setContentsMargins(20, 20, 20, 20);
  userLayout->setSpacing(16);

  auto* avatar = new QLabel("👤");
  avatar->setFixedSize(64, 64);
  avatar->setStyleSheet(R"(
    QLabel { background-color: #48484A; border-radius: 32px; font-size: 32px; }
  )");
  avatar->setAlignment(Qt::AlignCenter);
  userLayout->addWidget(avatar);

  auto* userInfo = new QWidget();
  auto* infoLayout = new QVBoxLayout(userInfo);
  infoLayout->setContentsMargins(0, 0, 0, 0);
  infoLayout->setSpacing(4);

  auto* userMgr = UserManager::instance();
  auto* userName = new QLabel(userMgr->currentUsername().isEmpty() ? tr("未登录") : userMgr->currentUsername());
  userName->setStyleSheet("font-size: 16px; font-weight: 500; color: #E0E0E0;");
  infoLayout->addWidget(userName);

  auto* userRole = new QLabel(userMgr->currentDisplayName() + " - " + userMgr->currentRole());
  userRole->setStyleSheet("color: #ADADAD;");
  infoLayout->addWidget(userRole);

  auto* lastLogin = new QLabel(tr("登录中..."));
  lastLogin->setStyleSheet("color: #888; font-size: 12px;");
  infoLayout->addWidget(lastLogin);

  userLayout->addWidget(userInfo);
  userLayout->addStretch();

  auto* changePassBtn = new QPushButton(tr("修改密码"));
  changePassBtn->setMinimumHeight(32);
  userLayout->addWidget(changePassBtn);

  layout->addWidget(currentUserGroup);

  // 用户列表
  auto* userListGroup = createStyledGroupBox(tr("用户列表"), this);
  auto* listLayout = new QVBoxLayout(userListGroup);
  listLayout->setContentsMargins(20, 20, 20, 20);
  listLayout->setSpacing(12);

  auto* btnBar = new QWidget();
  auto* btnBarLayout = new QHBoxLayout(btnBar);
  btnBarLayout->setContentsMargins(0, 0, 0, 0);

  auto* addUserBtn = new QPushButton(tr("+ 新增用户"));
  addUserBtn->setMinimumHeight(32);
  addUserBtn->setStyleSheet(R"(
    QPushButton { padding: 0 16px; background-color: #4CAF50; border: none; border-radius: 4px; color: white; }
    QPushButton:hover { background-color: #43A047; }
  )");
  btnBarLayout->addWidget(addUserBtn);

  auto* refreshBtn = new QPushButton(tr("🔄 刷新"));
  refreshBtn->setMinimumHeight(32);
  connect(refreshBtn, &QPushButton::clicked, this, &UserSettingsPage::refreshUserList);
  btnBarLayout->addWidget(refreshBtn);

  btnBarLayout->addStretch();
  listLayout->addWidget(btnBar);

  m_userTable = new QTableWidget(0, 5);
  m_userTable->setHorizontalHeaderLabels({tr("用户名"), tr("显示名"), tr("角色"), tr("状态"), tr("操作")});
  m_userTable->verticalHeader()->setVisible(false);
  m_userTable->setAlternatingRowColors(true);
  m_userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_userTable->horizontalHeader()->setStretchLastSection(true);
  m_userTable->setMinimumHeight(200);

  listLayout->addWidget(m_userTable);
  layout->addWidget(userListGroup);

  // 权限设置
  auto* permGroup = createStyledGroupBox(tr("权限说明"), this);
  auto* permLayout = new QGridLayout(permGroup);
  permLayout->setContentsMargins(20, 20, 20, 20);
  permLayout->setHorizontalSpacing(30);
  permLayout->setVerticalSpacing(12);

  QStringList permissions = {
    tr("ViewHistory - 查看历史"),
    tr("DeleteHistory - 删除历史"),
    tr("ViewStatistics - 查看统计"),
    tr("ExportData - 导出数据"),
    tr("RunDetection - 运行检测"),
    tr("SystemSettings - 系统设置"),
    tr("ManageUsers - 用户管理")
  };

  for (int i = 0; i < permissions.size(); ++i) {
    auto* label = new QLabel(permissions[i]);
    label->setStyleSheet("color: #ADADAD;");
    permLayout->addWidget(label, i / 3, i % 3);
  }

  layout->addWidget(permGroup);
  layout->addStretch();

  refreshUserList();
}

void UserSettingsPage::refreshUserList() {
  m_userTable->setRowCount(0);

  // TODO: 从 UserRepository 加载用户列表
  // 暂时显示示例数据
  QStringList users = {"admin", "operator", "viewer"};
  QStringList displayNames = {tr("系统管理员"), tr("操作员"), tr("观察员")};
  QStringList roles = {"admin", "operator", "viewer"};
  QStringList statuses = {tr("活跃"), tr("活跃"), tr("活跃")};

  for (int i = 0; i < users.size(); ++i) {
    int row = m_userTable->rowCount();
    m_userTable->insertRow(row);

    m_userTable->setItem(row, 0, new QTableWidgetItem(users[i]));
    m_userTable->setItem(row, 1, new QTableWidgetItem(displayNames[i]));
    m_userTable->setItem(row, 2, new QTableWidgetItem(roles[i]));
    m_userTable->setItem(row, 3, new QTableWidgetItem(statuses[i]));

    auto* opsWidget = new QWidget();
    auto* opsLayout = new QHBoxLayout(opsWidget);
    opsLayout->setContentsMargins(4, 0, 4, 0);
    opsLayout->setSpacing(4);

    auto* editBtn = new QPushButton(tr("编辑"));
    editBtn->setFixedHeight(28);
    auto* deleteBtn = new QPushButton(tr("删除"));
    deleteBtn->setFixedHeight(28);
    deleteBtn->setEnabled(users[i] != "admin");  // 不能删除admin

    opsLayout->addWidget(editBtn);
    opsLayout->addWidget(deleteBtn);
    opsLayout->addStretch();

    m_userTable->setCellWidget(row, 4, opsWidget);
  }
}

void UserSettingsPage::loadSettings() {
  refreshUserList();
}

void UserSettingsPage::saveSettings() {
  emit settingsChanged();
}
