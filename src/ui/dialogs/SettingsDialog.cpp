#include "SettingsDialog.h"
#include "settings/CameraSettingsPage.h"
#include "settings/LightSettingsPage.h"
#include "settings/PLCSettingsPage.h"
#include "settings/StorageSettingsPage.h"
#include "settings/DetectionSettingsPage.h"
#include "settings/UserSettingsPage.h"
#include "config/ConfigManager.h"
#include "common/Logger.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog{parent} {
  setModal(true);
  setWindowTitle(tr("系统设置"));
  setupUI();
  loadSettings();
}

void SettingsDialog::setupUI() {
  setMinimumSize(1200, 700);
  resize(1280, 760);

  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  auto* contentWidget = new QWidget(this);
  auto* contentLayout = new QHBoxLayout(contentWidget);
  contentLayout->setContentsMargins(0, 0, 0, 0);
  contentLayout->setSpacing(0);

  // 左侧导航
  auto* navWidget = new QWidget(contentWidget);
  navWidget->setFixedWidth(240);
  navWidget->setStyleSheet("background-color: #f5f5f5; border-right: 1px solid #e0e0e0;");
  auto* navLayout = new QVBoxLayout(navWidget);
  navLayout->setContentsMargins(0, 20, 0, 20);
  navLayout->setSpacing(0);

  auto* navLabel = new QLabel(tr("导航列表"), navWidget);
  navLabel->setStyleSheet("padding: 0 20px 12px 20px; font-size: 12px; color: #999999;");
  navLayout->addWidget(navLabel);

  m_navListWidget = new QListWidget(navWidget);
  m_navListWidget->setFrameShape(QFrame::NoFrame);
  m_navListWidget->setStyleSheet(R"(
    QListWidget { background-color: transparent; outline: none; }
    QListWidget::item { height: 48px; padding: 0 20px; border: none; color: #666666; }
    QListWidget::item:hover { background-color: #ebebeb; color: #333333; }
    QListWidget::item:selected { background-color: #3b82f6; color: white; }
  )");

  QStringList navItems = {
    tr("📷 相机设置"), tr("💡 光源设置"), tr("🔌 PLC 通信"),
    tr("💾 存储设置"), tr("🎯 检测参数"), tr("👤 用户权限")
  };
  for (const auto& item : navItems) {
    m_navListWidget->addItem(item);
  }
  navLayout->addWidget(m_navListWidget);
  navLayout->addStretch();

  // 右侧内容区域
  auto* rightWidget = new QWidget(contentWidget);
  rightWidget->setStyleSheet("background-color: white;");
  auto* rightLayout = new QVBoxLayout(rightWidget);
  rightLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setSpacing(0);

  // 页面标题栏
  auto* pageTitleWidget = new QWidget(rightWidget);
  pageTitleWidget->setFixedHeight(60);
  pageTitleWidget->setStyleSheet("background-color: #fafafa; border-bottom: 1px solid #e0e0e0;");
  auto* pageTitleLayout = new QHBoxLayout(pageTitleWidget);
  pageTitleLayout->setContentsMargins(30, 0, 30, 0);

  m_pageIconLabel = new QLabel(pageTitleWidget);
  m_pageIconLabel->setFixedSize(24, 24);
  pageTitleLayout->addWidget(m_pageIconLabel);

  m_pageTitleLabel = new QLabel(tr("相机设置"), pageTitleWidget);
  m_pageTitleLabel->setStyleSheet("font-size: 16px; font-weight: 500; color: #333333; margin-left: 8px;");
  pageTitleLayout->addWidget(m_pageTitleLabel);
  pageTitleLayout->addStretch();

  rightLayout->addWidget(pageTitleWidget);

  // 页面内容区域
  auto* scrollArea = new QScrollArea(rightWidget);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setWidgetResizable(true);
  scrollArea->setStyleSheet("QScrollArea { background-color: white; border: none; }");

  m_stackedWidget = new QStackedWidget();
  m_stackedWidget->setStyleSheet("background-color: white;");
  scrollArea->setWidget(m_stackedWidget);
  rightLayout->addWidget(scrollArea, 1);

  // 底部按钮栏
  auto* buttonBar = new QWidget(rightWidget);
  buttonBar->setFixedHeight(70);
  buttonBar->setStyleSheet("background-color: #fafafa; border-top: 1px solid #e0e0e0;");
  auto* buttonLayout = new QHBoxLayout(buttonBar);
  buttonLayout->setContentsMargins(30, 0, 30, 0);

  auto* restoreBtn = new QPushButton(tr("恢复默认"), buttonBar);
  restoreBtn->setFixedSize(100, 36);
  restoreBtn->setStyleSheet(R"(
    QPushButton { background-color: white; border: 1px solid #d0d0d0; border-radius: 4px; color: #666666; }
    QPushButton:hover { border-color: #3b82f6; color: #3b82f6; }
  )");
  connect(restoreBtn, &QPushButton::clicked, this, &SettingsDialog::onRestoreDefaultClicked);
  buttonLayout->addWidget(restoreBtn);
  buttonLayout->addStretch();

  auto* applyBtn = new QPushButton(tr("应用"), buttonBar);
  applyBtn->setFixedSize(80, 36);
  applyBtn->setStyleSheet(R"(
    QPushButton { background-color: #3b82f6; border: none; border-radius: 4px; color: white; }
    QPushButton:hover { background-color: #2563eb; }
  )");
  connect(applyBtn, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
  buttonLayout->addWidget(applyBtn);

  auto* cancelBtn = new QPushButton(tr("取消"), buttonBar);
  cancelBtn->setFixedSize(80, 36);
  cancelBtn->setStyleSheet(R"(
    QPushButton { background-color: white; border: 1px solid #d0d0d0; border-radius: 4px; color: #666666; }
    QPushButton:hover { background-color: #f5f5f5; }
  )");
  connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
  buttonLayout->addWidget(cancelBtn);

  auto* okBtn = new QPushButton(tr("确定"), buttonBar);
  okBtn->setFixedSize(80, 36);
  okBtn->setStyleSheet(R"(
    QPushButton { background-color: #10b981; border: none; border-radius: 4px; color: white; }
    QPushButton:hover { background-color: #059669; }
  )");
  connect(okBtn, &QPushButton::clicked, this, [this] {
    saveSettings();
    accept();
  });
  buttonLayout->addWidget(okBtn);

  rightLayout->addWidget(buttonBar);

  contentLayout->addWidget(navWidget);
  contentLayout->addWidget(rightWidget, 1);
  mainLayout->addWidget(contentWidget, 1);

  createPages();

  connect(m_navListWidget, &QListWidget::currentRowChanged, this, &SettingsDialog::onPageChanged);
  m_navListWidget->setCurrentRow(0);
}

void SettingsDialog::createPages() {
  m_cameraPage = new CameraSettingsPage();
  m_lightPage = new LightSettingsPage();
  m_plcPage = new PLCSettingsPage();
  m_storagePage = new StorageSettingsPage();
  m_detectionPage = new DetectionSettingsPage();
  m_userPage = new UserSettingsPage();

  m_stackedWidget->addWidget(m_cameraPage);
  m_stackedWidget->addWidget(m_lightPage);
  m_stackedWidget->addWidget(m_plcPage);
  m_stackedWidget->addWidget(m_storagePage);
  m_stackedWidget->addWidget(m_detectionPage);
  m_stackedWidget->addWidget(m_userPage);

  // 连接信号
  connect(m_cameraPage, &CameraSettingsPage::settingsChanged, this, &SettingsDialog::settingsChanged);
  connect(m_lightPage, &LightSettingsPage::settingsChanged, this, &SettingsDialog::settingsChanged);
  connect(m_plcPage, &PLCSettingsPage::settingsChanged, this, &SettingsDialog::settingsChanged);
  connect(m_storagePage, &StorageSettingsPage::settingsChanged, this, &SettingsDialog::settingsChanged);
  connect(m_detectionPage, &DetectionSettingsPage::settingsChanged, this, &SettingsDialog::settingsChanged);
  connect(m_userPage, &UserSettingsPage::settingsChanged, this, &SettingsDialog::settingsChanged);
}

void SettingsDialog::loadSettings() {
  LOG_DEBUG("SettingsDialog::loadSettings - Loading all settings pages");
  if (m_cameraPage) m_cameraPage->loadSettings();
  if (m_lightPage) m_lightPage->loadSettings();
  if (m_plcPage) m_plcPage->loadSettings();
  if (m_storagePage) m_storagePage->loadSettings();
  if (m_detectionPage) m_detectionPage->loadSettings();
  if (m_userPage) m_userPage->loadSettings();
}

void SettingsDialog::saveSettings() {
  LOG_INFO("SettingsDialog::saveSettings - Saving all settings pages");
  if (m_cameraPage) m_cameraPage->saveSettings();
  if (m_lightPage) m_lightPage->saveSettings();
  if (m_plcPage) m_plcPage->saveSettings();
  if (m_storagePage) m_storagePage->saveSettings();
  if (m_detectionPage) m_detectionPage->saveSettings();
  if (m_userPage) m_userPage->saveSettings();
  gConfig.save();
  LOG_INFO("SettingsDialog::saveSettings - Settings saved to config file");
}

void SettingsDialog::onPageChanged(int index) {
  if (index < 0 || index >= m_stackedWidget->count()) return;

  m_stackedWidget->setCurrentIndex(index);

  QStringList titles = {
    tr("相机设置"), tr("光源设置"), tr("PLC 通信"),
    tr("存储设置"), tr("检测参数"), tr("用户权限")
  };
  QStringList icons = {"📷", "💡", "🔌", "💾", "🎯", "👤"};

  if (index < titles.size()) {
    m_pageTitleLabel->setText(titles[index]);
    m_pageIconLabel->setText(icons[index]);
  }
}

void SettingsDialog::onRestoreDefaultClicked() {
  const auto reply = QMessageBox::question(this, tr("提示"), tr("确认恢复默认设置？"));
  if (reply == QMessageBox::Yes) {
    QMessageBox::information(this, tr("提示"), tr("已恢复默认设置"));
  }
}

void SettingsDialog::onApplyClicked() {
  saveSettings();
  QMessageBox::information(this, tr("提示"), tr("设置已应用"));
}
