#include "CameraSettingsPage.h"
#include "SettingsPageUtils.h"
#include "config/ConfigManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace SettingsUtils;

CameraSettingsPage::CameraSettingsPage(QWidget* parent) : QWidget(parent) {
  setupUI();
}

void CameraSettingsPage::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(24);

  // 相机配置组
  auto* configGroup = createStyledGroupBox(tr("相机配置"), this);
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
    QPushButton { padding: 0 16px; background-color: #3C3C3E; border: 1px solid #555; border-radius: 4px; color: #E0E0E0; }
    QPushButton:hover { background-color: #48484A; }
  )");
  ipLayout->addWidget(scanBtn);
  configLayout->addWidget(ipContainer, 1, 1);

  // 图片目录
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
    QPushButton { padding: 0 16px; background-color: #3C3C3E; border: 1px solid #555; border-radius: 4px; color: #E0E0E0; }
    QPushButton:hover { background-color: #48484A; }
  )");
  connect(browseBtn, &QPushButton::clicked, this, &CameraSettingsPage::onBrowseImageDir);
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
  auto* previewGroup = createStyledGroupBox(tr("相机预览"), this);
  auto* previewLayout = new QVBoxLayout(previewGroup);
  previewLayout->setContentsMargins(20, 20, 20, 20);
  previewLayout->setSpacing(16);

  auto* previewArea = new QLabel();
  previewArea->setFixedHeight(280);
  previewArea->setStyleSheet(R"(
    QLabel { background-color: #1a1f2e; border: 1px solid #48484A; border-radius: 4px; color: #ADADAD; }
  )");
  previewArea->setAlignment(Qt::AlignCenter);
  previewArea->setText(tr("未连接\n请先连接相机以查看预览"));
  previewLayout->addWidget(previewArea);

  auto* btnContainer = new QWidget();
  auto* btnLayout = new QHBoxLayout(btnContainer);
  btnLayout->setContentsMargins(0, 0, 0, 0);
  btnLayout->setSpacing(12);

  auto* connectBtn = new QPushButton(tr("📷 连接测试"));
  connectBtn->setMinimumHeight(36);
  connectBtn->setStyleSheet(R"(
    QPushButton { padding: 0 20px; background-color: #4CAF50; border: none; border-radius: 4px; color: white; }
    QPushButton:hover { background-color: #43A047; }
  )");
  btnLayout->addWidget(connectBtn);

  auto* captureBtn = new QPushButton(tr("🔄 抓取一帧"));
  captureBtn->setMinimumHeight(36);
  captureBtn->setStyleSheet(R"(
    QPushButton { padding: 0 20px; background-color: #3C3C3E; border: 1px solid #555; border-radius: 4px; color: #E0E0E0; }
    QPushButton:hover { background-color: #48484A; }
  )");
  btnLayout->addWidget(captureBtn);
  btnLayout->addStretch();
  previewLayout->addWidget(btnContainer);

  layout->addWidget(previewGroup);
  layout->addStretch();
}

void CameraSettingsPage::loadSettings() {
  auto camCfg = gConfig.cameraConfig();
  if (m_cameraTypeCombo) {
    int idx = m_cameraTypeCombo->findText(camCfg.type, Qt::MatchContains);
    if (idx < 0) {
      if (camCfg.type == "file") idx = 4;
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
}

void CameraSettingsPage::saveSettings() {
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
  emit settingsChanged();
}

void CameraSettingsPage::onBrowseImageDir() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("选择图片目录"),
    m_imageDirEdit ? m_imageDirEdit->text() : QString());
  if (!dir.isEmpty() && m_imageDirEdit) {
    m_imageDirEdit->setText(dir);
  }
}
