#include "SettingsDialog.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QFormLayout>
#include <QMessageBox>
#include <QLineEdit>
#include <QSlider>
#include <QComboBox>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog{parent}
{
  setupUI();
  loadSettings();
}

void SettingsDialog::loadSettings()
{

}

void SettingsDialog::saveSettings()
{

}

void SettingsDialog::onNavItemClicked(int index)
{
  if (index < 0) return;
  m_stackedWidget->setCurrentIndex(index);
  updateSectionHeader(index);
}

void SettingsDialog::onRestoreDefaultClicked()
{
  auto r = QMessageBox::question(
      this,
      tr("确认"),
      tr("确定要恢复默认设置吗？"),
      QMessageBox::Yes | QMessageBox::No,
      QMessageBox::No);
  if (r == QMessageBox::Yes) {
    QMessageBox::information(
        this,
        tr("提示"),
        tr("已恢复默认设置"),
        QMessageBox::Ok,
        QMessageBox::Ok);
  }
}

void SettingsDialog::onApplyClicked()
{
  QMessageBox::information(
      this,
      tr("提示"),
      tr("设置已应用"),
      QMessageBox::Ok,
      QMessageBox::Ok);
  emit settingsChanged();
}

void SettingsDialog::setupUI()
{
  setWindowTitle(tr("系统设置"));
  setMinimumSize(800, 600);
  resize(900, 650);

  auto* outerLayout = new QVBoxLayout(this);

  auto* headerLayout = new QHBoxLayout();
  auto* titleLabel = new QLabel(tr("系统设置"));
  auto* closeBtn = new QPushButton();
  closeBtn->setIcon(QIcon(":/icons/close.svg"));
  closeBtn->setFlat(true);
  headerLayout->addWidget(titleLabel);
  headerLayout->addStretch();
  headerLayout->addWidget(closeBtn);
  outerLayout->addLayout(headerLayout);

  auto* mainLayout = new QHBoxLayout();

  // 左侧导航
  auto* leftPanel = new QWidget();
  auto* leftLayout = new QVBoxLayout(leftPanel);
  auto* navLabel = new QLabel(tr("导航列表"));
  auto* navDivider = new QFrame();
  navDivider->setFrameShape(QFrame::HLine);
  m_navList = new QListWidget();
  m_navList->setFixedWidth(200);
  m_navList->setIconSize(QSize(20, 20));
  createNavList();
  leftLayout->addWidget(navLabel);
  leftLayout->addWidget(navDivider);
  leftLayout->addWidget(m_navList, 1);
  mainLayout->addWidget(leftPanel);

  // 右侧内容区
  auto* rightLayout = new QVBoxLayout();

  auto* contentHeaderLayout = new QHBoxLayout();
  m_sectionIconLabel = new QLabel();
  m_sectionTitleLabel = new QLabel();
  contentHeaderLayout->addWidget(m_sectionIconLabel);
  contentHeaderLayout->addWidget(m_sectionTitleLabel);
  contentHeaderLayout->addStretch();
  rightLayout->addLayout(contentHeaderLayout);

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
  outerLayout->addLayout(mainLayout, 1);

  connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
  connect(m_navList, &QListWidget::currentRowChanged,
          this, &SettingsDialog::onNavItemClicked);
  updateSectionHeader(m_navList->currentRow());
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
  m_stackedWidget->addWidget(createCameraPage());
  m_stackedWidget->addWidget(createLightPage());
  m_stackedWidget->addWidget(createPLCPage());
  m_stackedWidget->addWidget(createStoragePage());
  m_stackedWidget->addWidget(createDetectorPage());
  m_stackedWidget->addWidget(createUserPage());
}

QWidget *SettingsDialog::createCameraPage()
{
  auto* page = new QWidget();
  auto* layout = new QFormLayout(page);
  layout->addRow(tr("曝光"), new QLineEdit());
  layout->addRow(tr("增益"), new QLineEdit());
  layout->addRow(tr("帧率"), new QLineEdit());
  return page;
}

QWidget *SettingsDialog::createLightPage()
{
  auto* page = new QWidget();
  auto* layout = new QFormLayout(page);
  layout->addRow(tr("亮度"), new QSlider(Qt::Horizontal));
  layout->addRow(tr("色温"), new QSlider(Qt::Horizontal));
  return page;
}

QWidget *SettingsDialog::createPLCPage()
{
  auto* page = new QWidget();
  auto* layout = new QFormLayout(page);
  layout->addRow(tr("IP 地址"), new QLineEdit());
  layout->addRow(tr("端口"), new QLineEdit());
  return page;
}

QWidget *SettingsDialog::createStoragePage()
{
  auto* page = new QWidget();
  auto* layout = new QFormLayout(page);
  layout->addRow(tr("保存路径"), new QLineEdit());
  layout->addRow(tr("数据库"), new QComboBox());
  return page;
}

QWidget *SettingsDialog::createDetectorPage()
{
  auto* page = new QWidget();
  auto* layout = new QFormLayout(page);
  layout->addRow(tr("阈值"), new QLineEdit());
  layout->addRow(tr("模式"), new QComboBox());
  return page;
}

QWidget *SettingsDialog::createUserPage()
{
  auto* page = new QWidget();
  auto* layout = new QFormLayout(page);
  layout->addRow(tr("用户名"), new QLineEdit());
  layout->addRow(tr("权限"), new QComboBox());
  return page;
}

void SettingsDialog::updateSectionHeader(int index)
{
  if (index < 0 || index >= m_navList->count()) return;
  auto* item = m_navList->item(index);
  m_sectionIconLabel->setPixmap(item->icon().pixmap(20, 20));
  m_sectionTitleLabel->setText(item->text());
}
