#include "SettingsDialog.h"
#include <QPushButton>
#include <QHBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog{parent} {}

void SettingsDialog::loadSettings()
{

}

void SettingsDialog::saveSettings()
{

}

void SettingsDialog::onNavItemClicked(int index)
{

}

void SettingsDialog::onRestoreDefaultClicked()
{

}

void SettingsDialog::onApplyClicked()
{

}

void SettingsDialog::setupUI()
{
  setWindowTitle(tr("系统设置"));
  setMinimumSize(800, 600);
  resize(900, 650);

  auto* mainLayout = new QHBoxLayout(this);

  // 左侧导航
  m_navList = new QListWidget();
  m_navList->setFixedWidth(180);
  m_navList->setIconSize(QSize(24, 24));
  createNavList();
  mainLayout->addWidget(m_navList);

  // 右侧内容区
  auto* rightLayout = new QVBoxLayout();

  m_stackedWidget = new QStackedWidget();
  createStackedPages();
  rightLayout->addWidget(m_stackedWidget, 1);

  // 底部按钮
  auto* btnLayout = new QHBoxLayout();
  btnLayout->addStretch();

  auto* restoreBtn = new QPushButton(tr("恢复默认"));
  connect(restoreBtn, &QPushButton::clicked, this, &SettingsDialog::onRestoreDefaultClicked);
  btnLayout->addWidget(restoreBtn);

  auto* applyBtn = new QPushButton(tr("应用"));
  connect(applyBtn, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
  btnLayout->addWidget(applyBtn);

  auto* cancelBtn = new QPushButton(tr("取消"));
  connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
  btnLayout->addWidget(cancelBtn);

  auto* okBtn = new QPushButton(tr("确定"));
  okBtn->setDefault(true);
  connect(okBtn, &QPushButton::clicked, this, [this]{
    saveSettings();
    accept();
  });
  btnLayout->addWidget(okBtn);

  rightLayout->addLayout(btnLayout);
  mainLayout->addLayout(rightLayout);

  connect(m_navList, &QListWidget::currentRowChanged,
          m_stackedWidget, &QStackedWidget::setCurrentIndex);
}

void SettingsDialog::createNavList()
{
  m_navList->addItem(new QListWidgetItem(QIcon(":/icons/camera.svg"), tr("相机设置")));
  m_navList->addItem(new QListWidgetItem(QIcon(":/icons/light.svg"), tr("光源设置")));
  m_navList->addItem(new QListWidgetItem(QIcon(":/icons/plc.svg"), tr("PLC 通信")));
  m_navList->addItem(new QListWidgetItem(QIcon(":/icons/storage.svg"), tr("存储设置")));
  m_navList->addItem(new QListWidgetItem(QIcon(":/icons/detect.svg"), tr("检测参数")));
  m_navList->addItem(new QListWidgetItem(QIcon(":/icons/user.svg"), tr("用户权限")));
  m_navList->setCurrentRow(0);
}

void SettingsDialog::createStackedPages()
{

}

QWidget *SettingsDialog::createCameraPage()
{

}

QWidget *SettingsDialog::createLightPage()
{

}

QWidget *SettingsDialog::createPLCPage()
{

}

QWidget *SettingsDialog::createStoragePage()
{

}

QWidget *SettingsDialog::createDetectorPage()
{

}

QWidget *SettingsDialog::createUserPage()
{

}
