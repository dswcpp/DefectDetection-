#include "StorageSettingsPage.h"
#include "SettingsPageUtils.h"
#include "config/ConfigManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QStorageInfo>
#include <QVBoxLayout>

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

  // 磁盘空间信息
  auto* spaceGroup = createStyledGroupBox(tr("磁盘空间"), this);
  auto* spaceLayout = new QVBoxLayout(spaceGroup);
  spaceLayout->setContentsMargins(20, 20, 20, 20);
  spaceLayout->setSpacing(12);

  m_spaceInfoLabel = new QLabel();
  m_spaceInfoLabel->setStyleSheet("font-size: 14px; color: #333;");
  spaceLayout->addWidget(m_spaceInfoLabel);

  m_spaceProgressBar = new QProgressBar();
  m_spaceProgressBar->setRange(0, 100);
  m_spaceProgressBar->setTextVisible(true);
  m_spaceProgressBar->setFixedHeight(24);
  m_spaceProgressBar->setStyleSheet(R"(
    QProgressBar {
      border: 1px solid #d0d0d0;
      border-radius: 4px;
      background-color: #f0f0f0;
      text-align: center;
    }
    QProgressBar::chunk {
      background-color: #3b82f6;
      border-radius: 3px;
    }
  )");
  spaceLayout->addWidget(m_spaceProgressBar);

  auto* refreshBtn = new QPushButton(tr("🔄 刷新"));
  refreshBtn->setFixedWidth(100);
  refreshBtn->setMinimumHeight(32);
  connect(refreshBtn, &QPushButton::clicked, this, &StorageSettingsPage::updateDiskSpaceInfo);
  spaceLayout->addWidget(refreshBtn);

  layout->addWidget(spaceGroup);

  layout->addStretch();

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
  gConfig.setDatabaseConfig(dbCfg);

  LogConfig logCfg = gConfig.logConfig();
  if (m_logDirEdit) logCfg.dir = m_logDirEdit->text();
  if (m_logMaxSizeSpin) logCfg.maxFileSizeMB = m_logMaxSizeSpin->value();
  if (m_logMaxCountSpin) logCfg.maxFileCount = m_logMaxCountSpin->value();
  gConfig.setLogConfig(logCfg);

  emit settingsChanged();
}

void StorageSettingsPage::updateDiskSpaceInfo() {
  QString path = m_imagePathEdit ? m_imagePathEdit->text() : QString();
  if (path.isEmpty()) {
    path = QDir::currentPath();
  }

  QStorageInfo storage(path);
  if (!storage.isValid() || !storage.isReady()) {
    // 尝试使用根目录
    storage = QStorageInfo::root();
  }

  if (storage.isValid() && storage.isReady()) {
    qint64 totalBytes = storage.bytesTotal();
    qint64 freeBytes = storage.bytesAvailable();
    qint64 usedBytes = totalBytes - freeBytes;
    int usagePercent = totalBytes > 0 ? static_cast<int>((usedBytes * 100) / totalBytes) : 0;

    QString info = tr("💾 驱动器: %1\n"
                      "   总空间: %2\n"
                      "   已用空间: %3\n"
                      "   可用空间: %4")
                       .arg(storage.rootPath())
                       .arg(formatSize(totalBytes))
                       .arg(formatSize(usedBytes))
                       .arg(formatSize(freeBytes));

    if (m_spaceInfoLabel) {
      m_spaceInfoLabel->setText(info);
    }

    if (m_spaceProgressBar) {
      m_spaceProgressBar->setValue(usagePercent);
      m_spaceProgressBar->setFormat(tr("已使用 %1%").arg(usagePercent));

      // 根据使用率改变颜色
      QString chunkColor;
      if (usagePercent < 70) {
        chunkColor = "#10b981";  // 绿色
      } else if (usagePercent < 90) {
        chunkColor = "#f59e0b";  // 黄色
      } else {
        chunkColor = "#ef4444";  // 红色
      }

      m_spaceProgressBar->setStyleSheet(QString(R"(
        QProgressBar {
          border: 1px solid #d0d0d0;
          border-radius: 4px;
          background-color: #f0f0f0;
          text-align: center;
        }
        QProgressBar::chunk {
          background-color: %1;
          border-radius: 3px;
        }
      )").arg(chunkColor));
    }
  } else {
    if (m_spaceInfoLabel) {
      m_spaceInfoLabel->setText(tr("💾 无法获取磁盘信息"));
    }
    if (m_spaceProgressBar) {
      m_spaceProgressBar->setValue(0);
    }
  }
}

QString StorageSettingsPage::formatSize(qint64 bytes) const {
  const qint64 KB = 1024;
  const qint64 MB = KB * 1024;
  const qint64 GB = MB * 1024;
  const qint64 TB = GB * 1024;

  if (bytes >= TB) {
    return QString("%1 TB").arg(bytes / static_cast<double>(TB), 0, 'f', 2);
  } else if (bytes >= GB) {
    return QString("%1 GB").arg(bytes / static_cast<double>(GB), 0, 'f', 2);
  } else if (bytes >= MB) {
    return QString("%1 MB").arg(bytes / static_cast<double>(MB), 0, 'f', 2);
  } else if (bytes >= KB) {
    return QString("%1 KB").arg(bytes / static_cast<double>(KB), 0, 'f', 2);
  } else {
    return QString("%1 B").arg(bytes);
  }
}
