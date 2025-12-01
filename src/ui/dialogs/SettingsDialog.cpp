#include "SettingsDialog.h"
#include "config/ConfigManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
// 创建统一样式的分组框
QGroupBox* createStyledGroupBox(const QString& title, QWidget* parent) {
  auto* group = new QGroupBox(title, parent);
  group->setStyleSheet(R"(
    QGroupBox {
      font-weight: normal;
      border: 1px solid #d0d0d0;
      border-radius: 4px;
      margin-top: 12px;
      padding-top: 8px;
      background-color: #ffffff;
    }
    QGroupBox::title {
      subcontrol-origin: margin;
      left: 12px;
      padding: 0 8px;
      background-color: #f5f5f5;
      border: 1px solid #d0d0d0;
      border-radius: 2px;
    }
  )");
  return group;
}

// 创建带单位的输入框
QWidget* createSpinBoxWithUnit(int min, int max, int value, const QString& suffix, QWidget* parent) {
  auto* container = new QWidget(parent);
  auto* layout = new QHBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(4);

  auto* spinBox = new QSpinBox(container);
  spinBox->setRange(min, max);
  spinBox->setValue(value);
  spinBox->setMinimumHeight(32);
  spinBox->setMinimumWidth(100);
  layout->addWidget(spinBox);

  if (!suffix.isEmpty()) {
    auto* label = new QLabel(suffix, container);
    label->setStyleSheet("color: #666666;");
    layout->addWidget(label);
  }

  layout->addStretch();
  return container;
}

// 创建滑块控件组
QWidget* createSliderGroup(int min, int max, int value, const QString& suffix, QWidget* parent) {
  auto* container = new QWidget(parent);
  auto* layout = new QHBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(12);

  auto* slider = new QSlider(Qt::Horizontal, container);
  slider->setRange(min, max);
  slider->setValue(value);
  slider->setMinimumWidth(200);
  layout->addWidget(slider, 1);

  auto* spinBox = new QSpinBox(container);
  spinBox->setRange(min, max);
  spinBox->setValue(value);
  spinBox->setMinimumHeight(32);
  spinBox->setMinimumWidth(80);
  layout->addWidget(spinBox);

  if (!suffix.isEmpty()) {
    auto* label = new QLabel(suffix, container);
    label->setStyleSheet("color: #666666;");
    layout->addWidget(label);
  }

  // 连接滑块和数值框
  QObject::connect(slider, &QSlider::valueChanged, spinBox, &QSpinBox::setValue);
  QObject::connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), slider, &QSlider::setValue);

  return container;
}

// 创建带启用复选框的滑块组
QWidget* createCheckableSliderGroup(const QString& label, int min, int max, int value, bool checked, QWidget* parent) {
  auto* container = new QWidget(parent);
  auto* layout = new QHBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(12);

  auto* checkBox = new QCheckBox(label, container);
  checkBox->setChecked(checked);
  checkBox->setMinimumWidth(120);
  layout->addWidget(checkBox);

  auto* slider = new QSlider(Qt::Horizontal, container);
  slider->setRange(min, max);
  slider->setValue(value);
  slider->setEnabled(checked);
  slider->setMinimumWidth(200);
  layout->addWidget(slider, 1);

  auto* spinBox = new QSpinBox(container);
  spinBox->setRange(min, max);
  spinBox->setValue(value);
  spinBox->setEnabled(checked);
  spinBox->setMinimumHeight(32);
  spinBox->setMinimumWidth(80);
  layout->addWidget(spinBox);

  // 连接信号
  QObject::connect(checkBox, &QCheckBox::toggled, slider, &QSlider::setEnabled);
  QObject::connect(checkBox, &QCheckBox::toggled, spinBox, &QSpinBox::setEnabled);
  QObject::connect(slider, &QSlider::valueChanged, spinBox, &QSpinBox::setValue);
  QObject::connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), slider, &QSlider::setValue);

  return container;
}

}  // namespace

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog{parent} {
  setModal(true);
  setWindowTitle(tr("系统设置"));
  setupUI();
  loadSettings();
}

void SettingsDialog::loadSettings() {
  // 从 ConfigManager 加载相机配置
  auto camCfg = gConfig.cameraConfig();
  if (m_cameraTypeCombo) {
    int idx = m_cameraTypeCombo->findText(camCfg.type, Qt::MatchContains);
    if (idx < 0) {
      // 映射 type 到 combo 选项
      if (camCfg.type == "file") idx = 4;  // 离线调试
      else if (camCfg.type == "hik") idx = 2;
      else if (camCfg.type == "daheng") idx = 3;
      else idx = 0;
    }
    m_cameraTypeCombo->setCurrentIndex(idx);
  }
  if (m_cameraIpEdit) m_cameraIpEdit->setText(camCfg.ip);
  if (m_imageDirEdit) m_imageDirEdit->setText(camCfg.imageDir);
  if (m_exposureSlider) m_exposureSlider->setValue(static_cast<int>(camCfg.exposureUs));
  if (m_exposureSpin) m_exposureSpin->setValue(static_cast<int>(camCfg.exposureUs));
  if (m_gainSlider) m_gainSlider->setValue(static_cast<int>(camCfg.gainDb));
  if (m_gainSpin) m_gainSpin->setValue(static_cast<int>(camCfg.gainDb));
  if (m_captureIntervalSpin) m_captureIntervalSpin->setValue(camCfg.captureIntervalMs);
  if (m_loopCheck) m_loopCheck->setChecked(camCfg.loop);

  // 从 ConfigManager 加载检测配置
  auto detCfg = gConfig.detectionConfig();
  if (m_enableDetectionCheck) m_enableDetectionCheck->setChecked(detCfg.enabled);
  if (m_confidenceSlider) m_confidenceSlider->setValue(static_cast<int>(detCfg.confidenceThreshold * 100));
  if (m_confidenceSpin) m_confidenceSpin->setValue(static_cast<int>(detCfg.confidenceThreshold * 100));
  if (m_modelPathEdit) m_modelPathEdit->setText(detCfg.modelPath);

  // 从 ConfigManager 加载存储配置
  auto dbCfg = gConfig.databaseConfig();
  if (m_dbPathEdit) m_dbPathEdit->setText(dbCfg.path);
  if (m_maxRecordsSpin) m_maxRecordsSpin->setValue(dbCfg.maxRecords);
  if (m_autoCleanupCheck) m_autoCleanupCheck->setChecked(dbCfg.autoCleanup);

  auto logCfg = gConfig.logConfig();
  if (m_logDirEdit) m_logDirEdit->setText(logCfg.dir);
  if (m_logMaxSizeSpin) m_logMaxSizeSpin->setValue(logCfg.maxFileSizeMB);
  if (m_logMaxCountSpin) m_logMaxCountSpin->setValue(logCfg.maxFileCount);
}

void SettingsDialog::saveSettings() {
  // 保存相机配置
  CameraConfig camCfg = gConfig.cameraConfig();
  if (m_cameraTypeCombo) {
    int idx = m_cameraTypeCombo->currentIndex();
    QStringList types = {"gige", "usb", "hik", "daheng", "file"};
    if (idx >= 0 && idx < types.size()) camCfg.type = types[idx];
  }
  if (m_cameraIpEdit) camCfg.ip = m_cameraIpEdit->text();
  if (m_imageDirEdit) camCfg.imageDir = m_imageDirEdit->text();
  if (m_exposureSpin) camCfg.exposureUs = m_exposureSpin->value();
  if (m_gainSpin) camCfg.gainDb = m_gainSpin->value();
  if (m_captureIntervalSpin) camCfg.captureIntervalMs = m_captureIntervalSpin->value();
  if (m_loopCheck) camCfg.loop = m_loopCheck->isChecked();
  gConfig.setCameraConfig(camCfg);

  // 保存检测配置
  DetectionConfig detCfg = gConfig.detectionConfig();
  if (m_enableDetectionCheck) detCfg.enabled = m_enableDetectionCheck->isChecked();
  if (m_confidenceSpin) detCfg.confidenceThreshold = m_confidenceSpin->value() / 100.0;
  if (m_modelPathEdit) detCfg.modelPath = m_modelPathEdit->text();
  gConfig.setDetectionConfig(detCfg);

  // 保存存储配置
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

  // 保存到文件
  gConfig.save();
  emit settingsChanged();
}

void SettingsDialog::onBrowseImageDir() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("选择图片目录"),
    m_imageDirEdit ? m_imageDirEdit->text() : QString());
  if (!dir.isEmpty() && m_imageDirEdit) {
    m_imageDirEdit->setText(dir);
  }
}

void SettingsDialog::setupUI() {
  // 设置对话框大小
  setMinimumSize(1200, 700);
  resize(1280, 760);

  // 主布局
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // 内容区域
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

  // 导航按钮列表
  m_navListWidget = new QListWidget(navWidget);
  m_navListWidget->setFrameShape(QFrame::NoFrame);
  m_navListWidget->setStyleSheet(R"(
    QListWidget {
      background-color: transparent;
      outline: none;
    }
    QListWidget::item {
      height: 48px;
      padding: 0 20px;
      border: none;
      color: #666666;
    }
    QListWidget::item:hover {
      background-color: #ebebeb;
      color: #333333;
    }
    QListWidget::item:selected {
      background-color: #3b82f6;
      color: white;
    }
  )");

  // 添加导航项
  QStringList navItems = {
    tr("📷 相机设置"),
    tr("💡 光源设置"),
    tr("🔌 PLC 通信"),
    tr("💾 存储设置"),
    tr("🎯 检测参数"),
    tr("👤 用户权限")
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
  m_pageTitleWidget = new QWidget(rightWidget);
  m_pageTitleWidget->setFixedHeight(60);
  m_pageTitleWidget->setStyleSheet("background-color: #fafafa; border-bottom: 1px solid #e0e0e0;");
  auto* pageTitleLayout = new QHBoxLayout(m_pageTitleWidget);
  pageTitleLayout->setContentsMargins(30, 0, 30, 0);

  m_pageIconLabel = new QLabel(m_pageTitleWidget);
  m_pageIconLabel->setFixedSize(24, 24);
  pageTitleLayout->addWidget(m_pageIconLabel);

  m_pageTitleLabel = new QLabel(tr("相机设置"), m_pageTitleWidget);
  m_pageTitleLabel->setStyleSheet("font-size: 16px; font-weight: 500; color: #333333; margin-left: 8px;");
  pageTitleLayout->addWidget(m_pageTitleLabel);
  pageTitleLayout->addStretch();

  rightLayout->addWidget(m_pageTitleWidget);

  // 页面内容区域（带滚动）
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
    QPushButton {
      background-color: white;
      border: 1px solid #d0d0d0;
      border-radius: 4px;
      color: #666666;
    }
    QPushButton:hover {
      border-color: #3b82f6;
      color: #3b82f6;
    }
  )");
  connect(restoreBtn, &QPushButton::clicked, this, &SettingsDialog::onRestoreDefaultClicked);
  buttonLayout->addWidget(restoreBtn);

  buttonLayout->addStretch();

  auto* applyBtn = new QPushButton(tr("应用"), buttonBar);
  applyBtn->setFixedSize(80, 36);
  applyBtn->setStyleSheet(R"(
    QPushButton {
      background-color: #3b82f6;
      border: none;
      border-radius: 4px;
      color: white;
    }
    QPushButton:hover {
      background-color: #2563eb;
    }
  )");
  connect(applyBtn, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
  buttonLayout->addWidget(applyBtn);

  auto* cancelBtn = new QPushButton(tr("取消"), buttonBar);
  cancelBtn->setFixedSize(80, 36);
  cancelBtn->setStyleSheet(R"(
    QPushButton {
      background-color: white;
      border: 1px solid #d0d0d0;
      border-radius: 4px;
      color: #666666;
    }
    QPushButton:hover {
      background-color: #f5f5f5;
    }
  )");
  connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
  buttonLayout->addWidget(cancelBtn);

  auto* okBtn = new QPushButton(tr("确定"), buttonBar);
  okBtn->setFixedSize(80, 36);
  okBtn->setStyleSheet(R"(
    QPushButton {
      background-color: #10b981;
      border: none;
      border-radius: 4px;
      color: white;
    }
    QPushButton:hover {
      background-color: #059669;
    }
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

  // 创建所有页面
  createPages();

  // 连接导航切换
  connect(m_navListWidget, &QListWidget::currentRowChanged, this, &SettingsDialog::onPageChanged);
  m_navListWidget->setCurrentRow(0);
}

void SettingsDialog::createPages() {
  // 创建所有设置页面
  m_stackedWidget->addWidget(createCameraPage());
  m_stackedWidget->addWidget(createLightPage());
  m_stackedWidget->addWidget(createPLCPage());
  m_stackedWidget->addWidget(createStoragePage());
  m_stackedWidget->addWidget(createDetectionPage());
  m_stackedWidget->addWidget(createUserPage());
}

void SettingsDialog::onPageChanged(int index) {
  if (index < 0 || index >= m_stackedWidget->count())
    return;

  m_stackedWidget->setCurrentIndex(index);

  // 更新页面标题
  QStringList titles = {
    tr("相机设置"),
    tr("光源设置"),
    tr("PLC 通信"),
    tr("存储设置"),
    tr("检测参数"),
    tr("用户权限")
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
    // 恢复默认设置
    QMessageBox::information(this, tr("提示"), tr("已恢复默认设置"));
  }
}

void SettingsDialog::onApplyClicked() {
  saveSettings();
  QMessageBox::information(this, tr("提示"), tr("设置已应用"));
}

QWidget* SettingsDialog::createCameraPage() {
  auto* page = new QWidget();
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(24);

  // 相机配置组
  auto* configGroup = createStyledGroupBox(tr("相机配置"), page);
  auto* configLayout = new QGridLayout(configGroup);
  configLayout->setContentsMargins(20, 20, 20, 20);
  configLayout->setHorizontalSpacing(20);
  configLayout->setVerticalSpacing(16);

  // 相机类型
  configLayout->addWidget(new QLabel(tr("相机类型:")), 0, 0);
  m_cameraTypeCombo = new QComboBox();
  m_cameraTypeCombo->addItems({tr("GigE Vision"), tr("USB3 Vision"), tr("海康 SDK"), tr("大恒 SDK"), tr("离线调试")});
  m_cameraTypeCombo->setMinimumHeight(32);
  configLayout->addWidget(m_cameraTypeCombo, 0, 1);

  // 相机IP
  configLayout->addWidget(new QLabel(tr("相机 IP:")), 1, 0);
  auto* ipContainer = new QWidget();
  auto* ipLayout = new QHBoxLayout(ipContainer);
  ipLayout->setContentsMargins(0, 0, 0, 0);
  ipLayout->setSpacing(8);
  m_cameraIpEdit = new QLineEdit();
  m_cameraIpEdit->setText("192.168.1.100");
  m_cameraIpEdit->setMinimumHeight(32);
  ipLayout->addWidget(m_cameraIpEdit, 1);
  auto* scanBtn = new QPushButton(tr("扫描"));
  scanBtn->setMinimumHeight(32);
  scanBtn->setStyleSheet(R"(
    QPushButton {
      padding: 0 16px;
      background-color: white;
      border: 1px solid #d0d0d0;
      border-radius: 4px;
    }
    QPushButton:hover {
      background-color: #f5f5f5;
    }
  )");
  ipLayout->addWidget(scanBtn);
  configLayout->addWidget(ipContainer, 1, 1);

  // 图片目录（离线调试用）
  configLayout->addWidget(new QLabel(tr("图片目录:")), 2, 0);
  auto* dirContainer = new QWidget();
  auto* dirLayout = new QHBoxLayout(dirContainer);
  dirLayout->setContentsMargins(0, 0, 0, 0);
  dirLayout->setSpacing(8);
  m_imageDirEdit = new QLineEdit();
  m_imageDirEdit->setText("./images");
  m_imageDirEdit->setMinimumHeight(32);
  dirLayout->addWidget(m_imageDirEdit, 1);
  auto* browseBtn = new QPushButton(tr("浏览"));
  browseBtn->setMinimumHeight(32);
  browseBtn->setStyleSheet(R"(
    QPushButton {
      padding: 0 16px;
      background-color: white;
      border: 1px solid #d0d0d0;
      border-radius: 4px;
    }
    QPushButton:hover {
      background-color: #f5f5f5;
    }
  )");
  connect(browseBtn, &QPushButton::clicked, this, &SettingsDialog::onBrowseImageDir);
  dirLayout->addWidget(browseBtn);
  configLayout->addWidget(dirContainer, 2, 1);

  // 采集间隔
  configLayout->addWidget(new QLabel(tr("采集间隔:")), 3, 0);
  auto* intervalContainer = new QWidget();
  auto* intervalLayout = new QHBoxLayout(intervalContainer);
  intervalLayout->setContentsMargins(0, 0, 0, 0);
  intervalLayout->setSpacing(8);
  m_captureIntervalSpin = new QSpinBox();
  m_captureIntervalSpin->setRange(100, 10000);
  m_captureIntervalSpin->setValue(1000);
  m_captureIntervalSpin->setSuffix(" ms");
  m_captureIntervalSpin->setMinimumHeight(32);
  m_captureIntervalSpin->setMinimumWidth(120);
  intervalLayout->addWidget(m_captureIntervalSpin);
  m_loopCheck = new QCheckBox(tr("循环播放"));
  m_loopCheck->setChecked(true);
  intervalLayout->addWidget(m_loopCheck);
  intervalLayout->addStretch();
  configLayout->addWidget(intervalContainer, 3, 1);

  // 曝光时间
  configLayout->addWidget(new QLabel(tr("曝光时间:")), 4, 0);
  auto* exposureContainer = new QWidget();
  auto* exposureLayout = new QHBoxLayout(exposureContainer);
  exposureLayout->setContentsMargins(0, 0, 0, 0);
  exposureLayout->setSpacing(12);
  m_exposureSlider = new QSlider(Qt::Horizontal);
  m_exposureSlider->setRange(100, 100000);
  m_exposureSlider->setValue(10000);
  m_exposureSlider->setMinimumWidth(200);
  exposureLayout->addWidget(m_exposureSlider, 1);
  m_exposureSpin = new QSpinBox();
  m_exposureSpin->setRange(100, 100000);
  m_exposureSpin->setValue(10000);
  m_exposureSpin->setMinimumHeight(32);
  m_exposureSpin->setMinimumWidth(80);
  exposureLayout->addWidget(m_exposureSpin);
  exposureLayout->addWidget(new QLabel(tr("μs")));
  connect(m_exposureSlider, &QSlider::valueChanged, m_exposureSpin, &QSpinBox::setValue);
  connect(m_exposureSpin, QOverload<int>::of(&QSpinBox::valueChanged), m_exposureSlider, &QSlider::setValue);
  configLayout->addWidget(exposureContainer, 4, 1);

  // 增益
  configLayout->addWidget(new QLabel(tr("增益:")), 5, 0);
  auto* gainContainer = new QWidget();
  auto* gainLayout = new QHBoxLayout(gainContainer);
  gainLayout->setContentsMargins(0, 0, 0, 0);
  gainLayout->setSpacing(12);
  m_gainSlider = new QSlider(Qt::Horizontal);
  m_gainSlider->setRange(0, 24);
  m_gainSlider->setValue(0);
  m_gainSlider->setMinimumWidth(200);
  gainLayout->addWidget(m_gainSlider, 1);
  m_gainSpin = new QSpinBox();
  m_gainSpin->setRange(0, 24);
  m_gainSpin->setValue(0);
  m_gainSpin->setMinimumHeight(32);
  m_gainSpin->setMinimumWidth(80);
  gainLayout->addWidget(m_gainSpin);
  gainLayout->addWidget(new QLabel(tr("dB")));
  connect(m_gainSlider, &QSlider::valueChanged, m_gainSpin, &QSpinBox::setValue);
  connect(m_gainSpin, QOverload<int>::of(&QSpinBox::valueChanged), m_gainSlider, &QSlider::setValue);
  configLayout->addWidget(gainContainer, 5, 1);

  // 触发模式
  configLayout->addWidget(new QLabel(tr("触发模式:")), 6, 0);
  m_triggerCombo = new QComboBox();
  m_triggerCombo->addItems({tr("硬触发"), tr("软触发"), tr("连续采集")});
  m_triggerCombo->setCurrentIndex(2);
  m_triggerCombo->setMinimumHeight(32);
  configLayout->addWidget(m_triggerCombo, 6, 1);

  layout->addWidget(configGroup);

  // 相机预览组
  auto* previewGroup = createStyledGroupBox(tr("相机预览"), page);
  auto* previewLayout = new QVBoxLayout(previewGroup);
  previewLayout->setContentsMargins(20, 20, 20, 20);
  previewLayout->setSpacing(16);

  // 预览区域
  auto* previewArea = new QLabel();
  previewArea->setFixedHeight(280);
  previewArea->setStyleSheet(R"(
    QLabel {
      background-color: #1a1f2e;
      border: 1px solid #d0d0d0;
      border-radius: 4px;
      color: #666666;
    }
  )");
  previewArea->setAlignment(Qt::AlignCenter);
  previewArea->setText(tr("未连接\n请先连接相机以查看预览"));
  previewLayout->addWidget(previewArea);

  // 控制按钮
  auto* btnContainer = new QWidget();
  auto* btnLayout = new QHBoxLayout(btnContainer);
  btnLayout->setContentsMargins(0, 0, 0, 0);
  btnLayout->setSpacing(12);

  auto* connectBtn = new QPushButton(tr("📷 连接测试"));
  connectBtn->setMinimumHeight(36);
  connectBtn->setStyleSheet(R"(
    QPushButton {
      padding: 0 20px;
      background-color: #3b82f6;
      border: none;
      border-radius: 4px;
      color: white;
    }
    QPushButton:hover {
      background-color: #2563eb;
    }
  )");
  btnLayout->addWidget(connectBtn);

  auto* captureBtn = new QPushButton(tr("🔄 抓取一帧"));
  captureBtn->setMinimumHeight(36);
  captureBtn->setStyleSheet(R"(
    QPushButton {
      padding: 0 20px;
      background-color: white;
      border: 1px solid #d0d0d0;
      border-radius: 4px;
    }
    QPushButton:hover {
      background-color: #f5f5f5;
    }
  )");
  btnLayout->addWidget(captureBtn);

  btnLayout->addStretch();
  previewLayout->addWidget(btnContainer);

  layout->addWidget(previewGroup);
  layout->addStretch();

  return page;
}

QWidget* SettingsDialog::createLightPage() {
  auto* page = new QWidget();
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(24);

  // 光源配置组
  auto* configGroup = createStyledGroupBox(tr("光源配置"), page);
  auto* configLayout = new QGridLayout(configGroup);
  configLayout->setContentsMargins(20, 20, 20, 20);
  configLayout->setHorizontalSpacing(20);
  configLayout->setVerticalSpacing(16);

  // 控制方式
  configLayout->addWidget(new QLabel(tr("控制方式:")), 0, 0);
  auto* controlCombo = new QComboBox();
  controlCombo->addItems({tr("串口控制"), tr("网络控制"), tr("模拟控制")});
  controlCombo->setMinimumHeight(32);
  configLayout->addWidget(controlCombo, 0, 1);

  // 串口设置
  configLayout->addWidget(new QLabel(tr("串口设置:")), 1, 0);
  auto* serialContainer = new QWidget();
  auto* serialLayout = new QHBoxLayout(serialContainer);
  serialLayout->setContentsMargins(0, 0, 0, 0);
  serialLayout->setSpacing(12);

  auto* portCombo = new QComboBox();
  portCombo->addItems({tr("COM1"), tr("COM2"), tr("COM3"), tr("COM4")});
  portCombo->setMinimumHeight(32);
  portCombo->setMinimumWidth(120);
  serialLayout->addWidget(portCombo);

  auto* baudCombo = new QComboBox();
  baudCombo->addItems({tr("9600"), tr("19200"), tr("38400"), tr("115200")});
  baudCombo->setMinimumHeight(32);
  baudCombo->setMinimumWidth(120);
  serialLayout->addWidget(baudCombo);
  serialLayout->addStretch();

  configLayout->addWidget(serialContainer, 1, 1);

  layout->addWidget(configGroup);

  // 通道配置组
  auto* channelGroup = createStyledGroupBox(tr("通道配置"), page);
  auto* channelLayout = new QVBoxLayout(channelGroup);
  channelLayout->setContentsMargins(20, 20, 20, 20);
  channelLayout->setSpacing(16);

  // 通道1（正面）
  channelLayout->addWidget(createCheckableSliderGroup(tr("通道 1 (正面)"), 0, 255, 200, true, page));

  // 通道2（侧光）
  channelLayout->addWidget(createCheckableSliderGroup(tr("通道 2 (侧光)"), 0, 255, 200, true, page));

  // 通道3（背光）
  channelLayout->addWidget(createCheckableSliderGroup(tr("通道 3 (背光)"), 0, 255, 200, false, page));

  // 通道4（备用）
  channelLayout->addWidget(createCheckableSliderGroup(tr("通道 4 (备用)"), 0, 255, 200, false, page));

  layout->addWidget(channelGroup);

  // 频闪设置组
  auto* strobeGroup = createStyledGroupBox(tr("频闪设置"), page);
  auto* strobeLayout = new QVBoxLayout(strobeGroup);
  strobeLayout->setContentsMargins(20, 20, 20, 20);
  strobeLayout->setSpacing(16);

  auto* enableStrobe = new QCheckBox(tr("启用频闪模式"));
  enableStrobe->setStyleSheet("QCheckBox { font-size: 14px; }");
  strobeLayout->addWidget(enableStrobe);

  auto* freqContainer = new QWidget();
  auto* freqLayout = new QHBoxLayout(freqContainer);
  freqLayout->setContentsMargins(0, 0, 0, 0);
  freqLayout->setSpacing(12);

  freqLayout->addWidget(new QLabel(tr("频闪时长:")));
  auto* freqSpin = new QSpinBox();
  freqSpin->setRange(10, 10000);
  freqSpin->setValue(1000);
  freqSpin->setMinimumHeight(32);
  freqSpin->setMinimumWidth(120);
  freqLayout->addWidget(freqSpin);
  freqLayout->addWidget(new QLabel(tr("μs")));
  freqLayout->addStretch();

  strobeLayout->addWidget(freqContainer);
  layout->addWidget(strobeGroup);

  // 控制按钮
  auto* btnContainer = new QWidget();
  auto* btnLayout = new QHBoxLayout(btnContainer);
  btnLayout->setContentsMargins(0, 0, 0, 0);
  btnLayout->setSpacing(12);

  auto* openBtn = new QPushButton(tr("💡 开启光源"));
  openBtn->setMinimumHeight(36);
  openBtn->setMinimumWidth(120);
  openBtn->setStyleSheet(R"(
    QPushButton {
      background-color: #f59e0b;
      border: none;
      border-radius: 4px;
      color: white;
    }
    QPushButton:hover {
      background-color: #d97706;
    }
  )");
  btnLayout->addWidget(openBtn);

  auto* closeBtn = new QPushButton(tr("关闭光源"));
  closeBtn->setMinimumHeight(36);
  closeBtn->setMinimumWidth(120);
  closeBtn->setStyleSheet(R"(
    QPushButton {
      background-color: white;
      border: 1px solid #d0d0d0;
      border-radius: 4px;
    }
    QPushButton:hover {
      background-color: #f5f5f5;
    }
  )");
  btnLayout->addWidget(closeBtn);

  btnLayout->addStretch();
  layout->addWidget(btnContainer);

  layout->addStretch();

  return page;
}

QWidget* SettingsDialog::createPLCPage() {
  auto* page = new QWidget();
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(24);

  // 通信方式
  auto* commGroup = new QWidget();
  auto* commLayout = new QHBoxLayout(commGroup);
  commLayout->setContentsMargins(0, 0, 0, 0);
  commLayout->setSpacing(20);

  commLayout->addWidget(new QLabel(tr("通信方式")));
  auto* protocolCombo = new QComboBox();
  protocolCombo->addItems({tr("Modbus TCP"), tr("Modbus RTU"), tr("西门子 S7"), tr("三菱 MC")});
  protocolCombo->setMinimumHeight(32);
  protocolCombo->setMinimumWidth(200);
  commLayout->addWidget(protocolCombo);

  commLayout->addWidget(new QLabel(tr("连接状态")));
  auto* statusLabel = new QLabel(tr("● 未连接"));
  statusLabel->setStyleSheet("color: #ef4444; background-color: #fee; padding: 6px 12px; border-radius: 4px;");
  commLayout->addWidget(statusLabel);
  commLayout->addStretch();

  layout->addWidget(commGroup);

  // 连接参数
  auto* connGroup = new QWidget();
  auto* connLayout = new QGridLayout(connGroup);
  connLayout->setHorizontalSpacing(20);
  connLayout->setVerticalSpacing(16);

  connLayout->addWidget(new QLabel(tr("IP 地址")), 0, 0);
  auto* ipEdit = new QLineEdit("192.168.1.100");
  ipEdit->setMinimumHeight(32);
  connLayout->addWidget(ipEdit, 0, 1);

  connLayout->addWidget(new QLabel(tr("端口号")), 0, 2);
  auto* portSpin = new QSpinBox();
  portSpin->setRange(1, 65535);
  portSpin->setValue(502);
  portSpin->setMinimumHeight(32);
  connLayout->addWidget(portSpin, 0, 3);

  connLayout->addWidget(new QLabel(tr("超时时间 (ms)")), 1, 0);
  auto* timeoutSpin = new QSpinBox();
  timeoutSpin->setRange(100, 10000);
  timeoutSpin->setValue(3000);
  timeoutSpin->setMinimumHeight(32);
  connLayout->addWidget(timeoutSpin, 1, 1);

  connLayout->addWidget(new QLabel(tr("重试次数")), 1, 2);
  auto* retrySpin = new QSpinBox();
  retrySpin->setRange(1, 10);
  retrySpin->setValue(3);
  retrySpin->setMinimumHeight(32);
  connLayout->addWidget(retrySpin, 1, 3);

  layout->addWidget(connGroup);

  // 信号映射
  auto* signalGroup = createStyledGroupBox(tr("信号映射"), page);
  auto* signalLayout = new QGridLayout(signalGroup);
  signalLayout->setContentsMargins(20, 20, 20, 20);
  signalLayout->setHorizontalSpacing(30);
  signalLayout->setVerticalSpacing(12);

  signalLayout->addWidget(new QLabel(tr("触发信号 (输入)")), 0, 0);
  signalLayout->addWidget(new QLabel(tr("寄存器地址")), 0, 1);

  signalLayout->addWidget(new QLabel(tr("检测完成 (输出)")), 1, 0);
  signalLayout->addWidget(new QLabel(tr("寄存器地址")), 1, 1);

  auto* triggerEdit = new QLineEdit("M100");
  triggerEdit->setMinimumHeight(32);
  signalLayout->addWidget(triggerEdit, 0, 2);

  auto* completeEdit = new QLineEdit("M200");
  completeEdit->setMinimumHeight(32);
  signalLayout->addWidget(completeEdit, 1, 2);

  signalLayout->addWidget(new QLabel(tr("检测结果 OK (输出)")), 2, 0);
  signalLayout->addWidget(new QLabel(tr("寄存器地址")), 2, 1);

  auto* okEdit = new QLineEdit("M201");
  okEdit->setMinimumHeight(32);
  signalLayout->addWidget(okEdit, 2, 2);

  signalLayout->addWidget(new QLabel(tr("检测结果 NG (输出)")), 3, 0);
  signalLayout->addWidget(new QLabel(tr("寄存器地址")), 3, 1);

  auto* ngEdit = new QLineEdit("M202");
  ngEdit->setMinimumHeight(32);
  signalLayout->addWidget(ngEdit, 3, 2);

  layout->addWidget(signalGroup);

  // 测试按钮
  auto* btnContainer = new QWidget();
  auto* btnLayout = new QHBoxLayout(btnContainer);
  btnLayout->setContentsMargins(0, 0, 0, 0);
  btnLayout->setSpacing(12);

  auto* testBtn = new QPushButton(tr("测试连接"));
  testBtn->setMinimumHeight(36);
  testBtn->setMinimumWidth(120);
  testBtn->setStyleSheet(R"(
    QPushButton {
      background-color: #3b82f6;
      border: none;
      border-radius: 4px;
      color: white;
    }
    QPushButton:hover {
      background-color: #2563eb;
    }
  )");
  btnLayout->addWidget(testBtn);

  auto* connectBtn = new QPushButton(tr("连接 PLC"));
  connectBtn->setMinimumHeight(36);
  connectBtn->setMinimumWidth(120);
  connectBtn->setStyleSheet(R"(
    QPushButton {
      background-color: #10b981;
      border: none;
      border-radius: 4px;
      color: white;
    }
    QPushButton:hover {
      background-color: #059669;
    }
  )");
  btnLayout->addWidget(connectBtn);

  auto* disconnectBtn = new QPushButton(tr("断开连接"));
  disconnectBtn->setMinimumHeight(36);
  disconnectBtn->setMinimumWidth(120);
  disconnectBtn->setStyleSheet(R"(
    QPushButton {
      background-color: white;
      border: 1px solid #d0d0d0;
      border-radius: 4px;
    }
    QPushButton:hover {
      background-color: #f5f5f5;
    }
  )");
  btnLayout->addWidget(disconnectBtn);

  btnLayout->addStretch();
  layout->addWidget(btnContainer);

  layout->addStretch();

  return page;
}

QWidget* SettingsDialog::createStoragePage() {
  auto* page = new QWidget();
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(24);

  // 存储路径组
  auto* pathGroup = createStyledGroupBox(tr("存储路径"), page);
  auto* pathLayout = new QGridLayout(pathGroup);
  pathLayout->setContentsMargins(20, 20, 20, 20);
  pathLayout->setHorizontalSpacing(20);
  pathLayout->setVerticalSpacing(16);

  // 图像目录
  pathLayout->addWidget(new QLabel(tr("图像目录:")), 0, 0);
  auto* imagePathEdit = new QLineEdit("D:/DefectData/Images");
  imagePathEdit->setMinimumHeight(32);
  pathLayout->addWidget(imagePathEdit, 0, 1);
  auto* imageBrowseBtn = new QPushButton(tr("浏览"));
  imageBrowseBtn->setMinimumHeight(32);
  pathLayout->addWidget(imageBrowseBtn, 0, 2);

  // 数据目录
  pathLayout->addWidget(new QLabel(tr("数据目录:")), 1, 0);
  auto* dataPathEdit = new QLineEdit("D:/DefectData/Database");
  dataPathEdit->setMinimumHeight(32);
  pathLayout->addWidget(dataPathEdit, 1, 1);
  auto* dataBrowseBtn = new QPushButton(tr("浏览"));
  dataBrowseBtn->setMinimumHeight(32);
  pathLayout->addWidget(dataBrowseBtn, 1, 2);

  // 日志目录
  pathLayout->addWidget(new QLabel(tr("日志目录:")), 2, 0);
  auto* logPathEdit = new QLineEdit("D:/DefectData/Logs");
  logPathEdit->setMinimumHeight(32);
  pathLayout->addWidget(logPathEdit, 2, 1);
  auto* logBrowseBtn = new QPushButton(tr("浏览"));
  logBrowseBtn->setMinimumHeight(32);
  pathLayout->addWidget(logBrowseBtn, 2, 2);

  layout->addWidget(pathGroup);

  // 存储策略组
  auto* strategyGroup = createStyledGroupBox(tr("存储策略"), page);
  auto* strategyLayout = new QGridLayout(strategyGroup);
  strategyLayout->setContentsMargins(20, 20, 20, 20);
  strategyLayout->setHorizontalSpacing(20);
  strategyLayout->setVerticalSpacing(16);

  // 保存选项
  auto* saveAllCheck = new QCheckBox(tr("保存所有图像"));
  saveAllCheck->setChecked(true);
  strategyLayout->addWidget(saveAllCheck, 0, 0, 1, 2);

  auto* saveNGCheck = new QCheckBox(tr("仅保存NG图像"));
  strategyLayout->addWidget(saveNGCheck, 1, 0, 1, 2);

  // 保留天数
  strategyLayout->addWidget(new QLabel(tr("保留天数:")), 2, 0);
  auto* retainSpin = new QSpinBox();
  retainSpin->setRange(1, 365);
  retainSpin->setValue(30);
  retainSpin->setMinimumHeight(32);
  strategyLayout->addWidget(retainSpin, 2, 1);

  // 自动清理
  auto* autoCleanCheck = new QCheckBox(tr("启用自动清理"));
  autoCleanCheck->setChecked(true);
  strategyLayout->addWidget(autoCleanCheck, 3, 0, 1, 2);

  layout->addWidget(strategyGroup);

  // 数据库设置组
  auto* dbGroup = createStyledGroupBox(tr("数据库设置"), page);
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
  auto* spaceInfo = new QLabel(tr("💾 可用空间: 256 GB / 512 GB (50%)"));
  spaceInfo->setStyleSheet("padding: 12px; background-color: #f0f9ff; border: 1px solid #bae6fd; border-radius: 4px;");
  layout->addWidget(spaceInfo);

  layout->addStretch();

  return page;
}

QWidget* SettingsDialog::createDetectionPage() {
  auto* page = new QWidget();
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(24);

  // 全局设置组
  auto* globalGroup = createStyledGroupBox(tr("全局设置"), page);
  auto* globalLayout = new QVBoxLayout(globalGroup);
  globalLayout->setContentsMargins(20, 20, 20, 20);
  globalLayout->setSpacing(16);

  // 检测模式
  auto* modeContainer = new QWidget();
  auto* modeLayout = new QHBoxLayout(modeContainer);
  modeLayout->setContentsMargins(0, 0, 0, 0);
  modeLayout->setSpacing(20);

  modeLayout->addWidget(new QLabel(tr("检测模式:")));
  auto* modeCombo = new QComboBox();
  modeCombo->addItems({tr("标准模式"), tr("快速模式"), tr("精确模式")});
  modeCombo->setMinimumHeight(32);
  modeCombo->setMinimumWidth(200);
  modeLayout->addWidget(modeCombo);

  modeLayout->addWidget(new QLabel(tr("置信度阈值:")));
  auto* confSlider = createSliderGroup(0, 100, 75, "%", page);
  modeLayout->addWidget(confSlider);
  modeLayout->addStretch();

  globalLayout->addWidget(modeContainer);

  // 启用的检测项
  auto* detectItems = new QWidget();
  auto* detectLayout = new QGridLayout(detectItems);
  detectLayout->setHorizontalSpacing(40);
  detectLayout->setVerticalSpacing(12);

  auto* scratchCheck = new QCheckBox(tr("☑ 启用划痕检测"));
  scratchCheck->setChecked(true);
  detectLayout->addWidget(scratchCheck, 0, 0);

  auto* crackCheck = new QCheckBox(tr("☑ 启用裂纹检测"));
  crackCheck->setChecked(true);
  detectLayout->addWidget(crackCheck, 0, 1);

  auto* foreignCheck = new QCheckBox(tr("☑ 启用异物检测"));
  foreignCheck->setChecked(true);
  detectLayout->addWidget(foreignCheck, 1, 0);

  auto* sizeCheck = new QCheckBox(tr("☑ 启用尺寸测量"));
  sizeCheck->setChecked(true);
  detectLayout->addWidget(sizeCheck, 1, 1);

  globalLayout->addWidget(detectItems);
  layout->addWidget(globalGroup);

  // 划痕检测参数组
  auto* scratchGroup = createStyledGroupBox(tr("划痕检测参数"), page);
  auto* scratchLayout = new QGridLayout(scratchGroup);
  scratchLayout->setContentsMargins(20, 20, 20, 20);
  scratchLayout->setHorizontalSpacing(30);
  scratchLayout->setVerticalSpacing(16);

  scratchLayout->addWidget(new QLabel(tr("最小长度 (像素)")), 0, 0);
  scratchLayout->addWidget(createSpinBoxWithUnit(10, 500, 50, "", page), 0, 1);

  scratchLayout->addWidget(new QLabel(tr("最大宽度 (像素)")), 0, 2);
  scratchLayout->addWidget(createSpinBoxWithUnit(1, 50, 5, "", page), 0, 3);

  scratchLayout->addWidget(new QLabel(tr("灵敏度")), 1, 0);
  scratchLayout->addWidget(createSliderGroup(0, 100, 80, "", page), 1, 1, 1, 2);

  scratchLayout->addWidget(new QLabel(tr("对比度阈值")), 1, 2);
  scratchLayout->addWidget(createSpinBoxWithUnit(10, 100, 30, "", page), 1, 3);

  layout->addWidget(scratchGroup);

  // 裂纹检测参数组
  auto* crackGroup = createStyledGroupBox(tr("裂纹检测参数"), page);
  auto* crackLayout = new QGridLayout(crackGroup);
  crackLayout->setContentsMargins(20, 20, 20, 20);
  crackLayout->setHorizontalSpacing(30);
  crackLayout->setVerticalSpacing(16);

  crackLayout->addWidget(new QLabel(tr("最小面积 (像素²)")), 0, 0);
  crackLayout->addWidget(createSpinBoxWithUnit(50, 5000, 100, "", page), 0, 1);

  crackLayout->addWidget(new QLabel(tr("二值化阈值")), 0, 2);
  crackLayout->addWidget(createSpinBoxWithUnit(50, 200, 128, "", page), 0, 3);

  crackLayout->addWidget(new QLabel(tr("形态学核大小")), 1, 0);
  crackLayout->addWidget(createSpinBoxWithUnit(1, 10, 3, "", page), 1, 1);

  layout->addWidget(crackGroup);

  layout->addStretch();

  return page;
}

QWidget* SettingsDialog::createUserPage() {
  auto* page = new QWidget();
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(24);

  // 当前用户信息
  auto* currentUserGroup = createStyledGroupBox(tr("当前用户"), page);
  auto* userLayout = new QHBoxLayout(currentUserGroup);
  userLayout->setContentsMargins(20, 20, 20, 20);
  userLayout->setSpacing(16);

  auto* avatar = new QLabel("👤");
  avatar->setFixedSize(64, 64);
  avatar->setStyleSheet(R"(
    QLabel {
      background-color: #e0e7ff;
      border-radius: 32px;
      font-size: 32px;
    }
  )");
  avatar->setAlignment(Qt::AlignCenter);
  userLayout->addWidget(avatar);

  auto* userInfo = new QWidget();
  auto* infoLayout = new QVBoxLayout(userInfo);
  infoLayout->setContentsMargins(0, 0, 0, 0);
  infoLayout->setSpacing(4);

  auto* userName = new QLabel(tr("admin"));
  userName->setStyleSheet("font-size: 16px; font-weight: 500;");
  infoLayout->addWidget(userName);

  auto* userRole = new QLabel(tr("管理员 - 全部权限"));
  userRole->setStyleSheet("color: #666666;");
  infoLayout->addWidget(userRole);

  auto* lastLogin = new QLabel(tr("最后登录: 2024-01-15 09:30"));
  lastLogin->setStyleSheet("color: #999999; font-size: 12px;");
  infoLayout->addWidget(lastLogin);

  userLayout->addWidget(userInfo);
  userLayout->addStretch();

  auto* changePassBtn = new QPushButton(tr("修改密码"));
  changePassBtn->setMinimumHeight(32);
  userLayout->addWidget(changePassBtn);

  layout->addWidget(currentUserGroup);

  // 用户列表
  auto* userListGroup = createStyledGroupBox(tr("用户列表"), page);
  auto* listLayout = new QVBoxLayout(userListGroup);
  listLayout->setContentsMargins(20, 20, 20, 20);
  listLayout->setSpacing(12);

  // 操作按钮
  auto* btnBar = new QWidget();
  auto* btnBarLayout = new QHBoxLayout(btnBar);
  btnBarLayout->setContentsMargins(0, 0, 0, 0);

  auto* addUserBtn = new QPushButton(tr("+ 新增用户"));
  addUserBtn->setMinimumHeight(32);
  addUserBtn->setStyleSheet(R"(
    QPushButton {
      padding: 0 16px;
      background-color: #3b82f6;
      border: none;
      border-radius: 4px;
      color: white;
    }
    QPushButton:hover {
      background-color: #2563eb;
    }
  )");
  btnBarLayout->addWidget(addUserBtn);
  btnBarLayout->addStretch();

  listLayout->addWidget(btnBar);

  // 用户表格
  auto* userTable = new QTableWidget(4, 5);
  userTable->setHorizontalHeaderLabels({tr("用户名"), tr("角色"), tr("状态"), tr("最后登录"), tr("操作")});
  userTable->verticalHeader()->setVisible(false);
  userTable->setAlternatingRowColors(true);
  userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  userTable->horizontalHeader()->setStretchLastSection(true);
  userTable->setMinimumHeight(200);

  // 添加示例数据
  QStringList users = {"admin", "operator1", "engineer", "viewer"};
  QStringList roles = {tr("管理员"), tr("操作员"), tr("工程师"), tr("观察员")};
  QStringList statuses = {tr("在线"), tr("在线"), tr("离线"), tr("离线")};
  QStringList logins = {tr("今天 09:30"), tr("今天 08:15"), tr("昨天 17:45"), tr("3天前")};

  for (int i = 0; i < 4; ++i) {
    userTable->setItem(i, 0, new QTableWidgetItem(users[i]));
    userTable->setItem(i, 1, new QTableWidgetItem(roles[i]));
    userTable->setItem(i, 2, new QTableWidgetItem(statuses[i]));
    userTable->setItem(i, 3, new QTableWidgetItem(logins[i]));

    auto* opsWidget = new QWidget();
    auto* opsLayout = new QHBoxLayout(opsWidget);
    opsLayout->setContentsMargins(4, 0, 4, 0);
    opsLayout->setSpacing(4);

    auto* editBtn = new QPushButton(tr("编辑"));
    editBtn->setFixedHeight(28);
    auto* deleteBtn = new QPushButton(tr("删除"));
    deleteBtn->setFixedHeight(28);

    opsLayout->addWidget(editBtn);
    opsLayout->addWidget(deleteBtn);
    opsLayout->addStretch();

    userTable->setCellWidget(i, 4, opsWidget);
  }

  listLayout->addWidget(userTable);
  layout->addWidget(userListGroup);

  // 权限设置
  auto* permGroup = createStyledGroupBox(tr("权限设置"), page);
  auto* permLayout = new QGridLayout(permGroup);
  permLayout->setContentsMargins(20, 20, 20, 20);
  permLayout->setHorizontalSpacing(30);
  permLayout->setVerticalSpacing(12);

  QStringList permissions = {
    tr("查看实时画面"), tr("执行检测"), tr("修改参数"),
    tr("查看报表"), tr("导出数据"), tr("系统设置"),
    tr("用户管理"), tr("远程维护")
  };

  for (int i = 0; i < permissions.size(); ++i) {
    auto* check = new QCheckBox(permissions[i]);
    check->setChecked(i < 4);
    permLayout->addWidget(check, i / 3, i % 3);
  }

  layout->addWidget(permGroup);
  layout->addStretch();

  return page;
}