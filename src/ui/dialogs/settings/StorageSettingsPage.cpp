#include "StorageSettingsPage.h"
#include "SettingsPageUtils.h"
#include "services/StorageService.h"
#include "config/ConfigManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QFrame>

using namespace SettingsUtils;

StorageSettingsPage::StorageSettingsPage(QWidget* parent) : QWidget(parent) {
  setupUI();
}

void StorageSettingsPage::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(24);

  // 存储路径组
  auto* pathGroup = createStyledGroupBox(tr("存储路径"), this);
  auto* pathLayout = new QGridLayout(pathGroup);
  pathLayout->setContentsMargins(20, 20, 20, 20);
  pathLayout->setHorizontalSpacing(20);
  pathLayout->setVerticalSpacing(16);

  pathLayout->addWidget(new QLabel(tr("图像目录:")), 0, 0);
  m_imagePathEdit = new QLineEdit("D:/DefectData/Images");
  m_imagePathEdit->setMinimumHeight(32);
  pathLayout->addWidget(m_imagePathEdit, 0, 1);
  auto* imageBrowseBtn = new QPushButton(tr("浏览"));
  imageBrowseBtn->setMinimumHeight(32);
  connect(imageBrowseBtn, &QPushButton::clicked, this, [this]() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择图像目录"), m_imagePathEdit->text());
    if (!dir.isEmpty()) {
      m_imagePathEdit->setText(dir);
      updateDiskSpaceInfo();
    }
  });
  pathLayout->addWidget(imageBrowseBtn, 0, 2);
  connect(m_imagePathEdit, &QLineEdit::textChanged, this, &StorageSettingsPage::updateDiskSpaceInfo);

  pathLayout->addWidget(new QLabel(tr("数据目录:")), 1, 0);
  m_dbPathEdit = new QLineEdit("D:/DefectData/Database");
  m_dbPathEdit->setMinimumHeight(32);
  pathLayout->addWidget(m_dbPathEdit, 1, 1);
  auto* dataBrowseBtn = new QPushButton(tr("浏览"));
  dataBrowseBtn->setMinimumHeight(32);
  pathLayout->addWidget(dataBrowseBtn, 1, 2);

  pathLayout->addWidget(new QLabel(tr("日志目录:")), 2, 0);
  m_logDirEdit = new QLineEdit("D:/DefectData/Logs");
  m_logDirEdit->setMinimumHeight(32);
  pathLayout->addWidget(m_logDirEdit, 2, 1);
  auto* logBrowseBtn = new QPushButton(tr("浏览"));
  logBrowseBtn->setMinimumHeight(32);
  pathLayout->addWidget(logBrowseBtn, 2, 2);

  layout->addWidget(pathGroup);

  // 存储策略组
  auto* strategyGroup = createStyledGroupBox(tr("存储策略"), this);
  auto* strategyLayout = new QGridLayout(strategyGroup);
  strategyLayout->setContentsMargins(20, 20, 20, 20);
  strategyLayout->setHorizontalSpacing(20);
  strategyLayout->setVerticalSpacing(16);

  auto* saveAllCheck = new QCheckBox(tr("保存所有图像"));
  saveAllCheck->setChecked(true);
  strategyLayout->addWidget(saveAllCheck, 0, 0, 1, 2);

  auto* saveNGCheck = new QCheckBox(tr("仅保存NG图像"));
  strategyLayout->addWidget(saveNGCheck, 1, 0, 1, 2);

  strategyLayout->addWidget(new QLabel(tr("最大记录数:")), 2, 0);
  m_maxRecordsSpin = new QSpinBox();
  m_maxRecordsSpin->setRange(1000, 1000000);
  m_maxRecordsSpin->setValue(100000);
  m_maxRecordsSpin->setMinimumHeight(32);
  strategyLayout->addWidget(m_maxRecordsSpin, 2, 1);

  m_autoCleanupCheck = new QCheckBox(tr("启用自动清理"));
  m_autoCleanupCheck->setChecked(true);
  strategyLayout->addWidget(m_autoCleanupCheck, 3, 0, 1, 2);

  layout->addWidget(strategyGroup);

  // 日志设置组
  auto* logGroup = createStyledGroupBox(tr("日志设置"), this);
  auto* logLayout = new QGridLayout(logGroup);
  logLayout->setContentsMargins(20, 20, 20, 20);
  logLayout->setHorizontalSpacing(20);
  logLayout->setVerticalSpacing(16);

  logLayout->addWidget(new QLabel(tr("单个文件大小:")), 0, 0);
  m_logMaxSizeSpin = new QSpinBox();
  m_logMaxSizeSpin->setRange(1, 100);
  m_logMaxSizeSpin->setValue(10);
  m_logMaxSizeSpin->setSuffix(" MB");
  m_logMaxSizeSpin->setMinimumHeight(32);
  logLayout->addWidget(m_logMaxSizeSpin, 0, 1);

  logLayout->addWidget(new QLabel(tr("保留文件数:")), 1, 0);
  m_logMaxCountSpin = new QSpinBox();
  m_logMaxCountSpin->setRange(1, 100);
  m_logMaxCountSpin->setValue(10);
  m_logMaxCountSpin->setMinimumHeight(32);
  logLayout->addWidget(m_logMaxCountSpin, 1, 1);

  layout->addWidget(logGroup);

  // 数据库设置组
  auto* dbGroup = createStyledGroupBox(tr("数据库设置"), this);
  auto* dbLayout = new QGridLayout(dbGroup);
  dbLayout->setContentsMargins(20, 20, 20, 20);
  dbLayout->setHorizontalSpacing(20);
  dbLayout->setVerticalSpacing(16);

  dbLayout->addWidget(new QLabel(tr("数据库类型:")), 0, 0);
  auto* dbCombo = new QComboBox();
  dbCombo->addItems({tr("SQLite"), tr("MySQL"), tr("PostgreSQL")});
  dbCombo->setMinimumHeight(32);
  dbLayout->addWidget(dbCombo, 0, 1);

  layout->addWidget(dbGroup);

  // 磁盘空间信息（多路径监控）
  auto* spaceGroup = createStyledGroupBox(tr("存储空间监控"), this);
  auto* spaceMainLayout = new QVBoxLayout(spaceGroup);
  spaceMainLayout->setContentsMargins(20, 20, 20, 20);
  spaceMainLayout->setSpacing(12);

  // 存储信息容器
  m_storageInfoContainer = new QWidget();
  m_storageInfoLayout = new QVBoxLayout(m_storageInfoContainer);
  m_storageInfoLayout->setContentsMargins(0, 0, 0, 0);
  m_storageInfoLayout->setSpacing(16);
  spaceMainLayout->addWidget(m_storageInfoContainer);

  // 刷新按钮
  auto* btnLayout = new QHBoxLayout();
  btnLayout->addStretch();
  auto* refreshBtn = new QPushButton(tr("刷新"));
  refreshBtn->setFixedWidth(100);
  refreshBtn->setMinimumHeight(32);
  connect(refreshBtn, &QPushButton::clicked, this, &StorageSettingsPage::updateDiskSpaceInfo);
  btnLayout->addWidget(refreshBtn);
  spaceMainLayout->addLayout(btnLayout);

  layout->addWidget(spaceGroup);

  layout->addStretch();

  // 初始化存储服务路径
  StorageService::instance()->initialize();
  
  // 初始化磁盘空间信息
  updateDiskSpaceInfo();
}

void StorageSettingsPage::loadSettings() {
  auto dbCfg = gConfig.databaseConfig();
  if (m_dbPathEdit) m_dbPathEdit->setText(dbCfg.path);
  if (m_maxRecordsSpin) m_maxRecordsSpin->setValue(dbCfg.maxRecords);
  if (m_autoCleanupCheck) m_autoCleanupCheck->setChecked(dbCfg.autoCleanup);

  auto logCfg = gConfig.logConfig();
  if (m_logDirEdit) m_logDirEdit->setText(logCfg.dir);
  if (m_logMaxSizeSpin) m_logMaxSizeSpin->setValue(logCfg.maxFileSizeMB);
  if (m_logMaxCountSpin) m_logMaxCountSpin->setValue(logCfg.maxFileCount);
}

void StorageSettingsPage::saveSettings() {
  DatabaseConfig dbCfg = gConfig.databaseConfig();
  if (m_dbPathEdit) dbCfg.path = m_dbPathEdit->text();
  if (m_maxRecordsSpin) dbCfg.maxRecords = m_maxRecordsSpin->value();
  if (m_autoCleanupCheck) dbCfg.autoCleanup = m_autoCleanupCheck->isChecked();
  gConfig.setDatabaseConfig(dbCfg, false);  // 不自动保存，由SettingsDialog统一保存

  LogConfig logCfg = gConfig.logConfig();
  if (m_logDirEdit) logCfg.dir = m_logDirEdit->text();
  if (m_logMaxSizeSpin) logCfg.maxFileSizeMB = m_logMaxSizeSpin->value();
  if (m_logMaxCountSpin) logCfg.maxFileCount = m_logMaxCountSpin->value();
  gConfig.setLogConfig(logCfg, false);  // 不自动保存，由SettingsDialog统一保存

  emit settingsChanged();
}

void StorageSettingsPage::updateDiskSpaceInfo() {
  auto* service = StorageService::instance();
  
  // 更新路径设置
  if (m_imagePathEdit && !m_imagePathEdit->text().isEmpty()) {
    service->setPath(StoragePathType::ImageSave, m_imagePathEdit->text());
  }
  if (m_dbPathEdit && !m_dbPathEdit->text().isEmpty()) {
    QFileInfo dbInfo(m_dbPathEdit->text());
    service->setPath(StoragePathType::Database, dbInfo.absolutePath());
  }
  if (m_logDirEdit && !m_logDirEdit->text().isEmpty()) {
    service->setPath(StoragePathType::Log, m_logDirEdit->text());
  }
  
  // 更新显示
  updateStorageInfoWidgets();
}

void StorageSettingsPage::createStorageInfoWidget(QVBoxLayout* layout, const QString& name, const StorageInfo& info) {
  auto* frame = new QFrame();
  frame->setFrameShape(QFrame::StyledPanel);
  frame->setStyleSheet(R"(
    QFrame {
      background-color: #3C3C3E;
      border: 1px solid #48484A;
      border-radius: 6px;
      padding: 8px;
    }
  )");
  
  auto* frameLayout = new QVBoxLayout(frame);
  frameLayout->setContentsMargins(12, 10, 12, 10);
  frameLayout->setSpacing(8);
  
  // 标题行：名称 + 驱动器
  auto* titleLayout = new QHBoxLayout();
  auto* nameLabel = new QLabel(QString("<b>%1</b>").arg(name));
  nameLabel->setStyleSheet("font-size: 13px; color: #E0E0E0;");
  titleLayout->addWidget(nameLabel);
  
  auto* driveLabel = new QLabel(info.isValid ? info.rootPath : tr("不可用"));
  driveLabel->setStyleSheet("font-size: 12px; color: #ADADAD;");
  titleLayout->addWidget(driveLabel);
  titleLayout->addStretch();
  
  // 状态标签
  if (info.isValid) {
    QString statusText;
    QString statusColor;
    if (info.usagePercent >= 95) {
      statusText = tr("危险");
      statusColor = "#ef4444";
    } else if (info.usagePercent >= 85) {
      statusText = tr("警告");
      statusColor = "#f59e0b";
    } else {
      statusText = tr("正常");
      statusColor = "#10b981";
    }
    auto* statusLabel = new QLabel(statusText);
    statusLabel->setStyleSheet(QString("font-size: 11px; color: white; background-color: %1; "
                                        "padding: 2px 8px; border-radius: 3px;").arg(statusColor));
    titleLayout->addWidget(statusLabel);
  }
  frameLayout->addLayout(titleLayout);
  
  if (info.isValid) {
    // 进度条
    auto* progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    progressBar->setValue(info.usagePercent);
    progressBar->setTextVisible(true);
    progressBar->setFormat(tr("%1% 已使用").arg(info.usagePercent));
    progressBar->setFixedHeight(20);
    
    QString chunkColor = (info.usagePercent >= 95) ? "#ef4444" :
                         (info.usagePercent >= 85) ? "#f59e0b" : "#10b981";
    progressBar->setStyleSheet(QString(R"(
      QProgressBar {
        border: 1px solid #555;
        border-radius: 3px;
        background-color: #2C2C2E;
        text-align: center;
        font-size: 11px;
        color: #E0E0E0;
      }
      QProgressBar::chunk {
        background-color: %1;
        border-radius: 2px;
      }
    )").arg(chunkColor));
    frameLayout->addWidget(progressBar);
    
    // 空间信息
    auto* spaceLabel = new QLabel(tr("已用: %1 / 总计: %2 | 可用: %3")
        .arg(StorageService::formatSize(info.usedBytes))
        .arg(StorageService::formatSize(info.totalBytes))
        .arg(StorageService::formatSize(info.freeBytes)));
    spaceLabel->setStyleSheet("font-size: 11px; color: #ADADAD;");
    frameLayout->addWidget(spaceLabel);
  } else {
    auto* errorLabel = new QLabel(tr("路径不存在或无法访问"));
    errorLabel->setStyleSheet("font-size: 12px; color: #888;");
    frameLayout->addWidget(errorLabel);
  }
  
  layout->addWidget(frame);
  m_storageWidgets[name] = frame;
}

void StorageSettingsPage::updateStorageInfoWidgets() {
  // 清除旧的widgets
  for (auto* widget : m_storageWidgets.values()) {
    m_storageInfoLayout->removeWidget(widget);
    widget->deleteLater();
  }
  m_storageWidgets.clear();
  
  // 获取所有监控路径的存储信息
  auto infoList = StorageService::instance()->getAllMonitoredStorageInfo();
  
  for (const auto& info : infoList) {
    createStorageInfoWidget(m_storageInfoLayout, info.name, info);
  }
}
