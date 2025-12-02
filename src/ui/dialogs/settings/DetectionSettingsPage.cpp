#include "DetectionSettingsPage.h"
#include "SettingsPageUtils.h"
#include "config/ConfigManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace SettingsUtils;

DetectionSettingsPage::DetectionSettingsPage(QWidget* parent) : QWidget(parent) {
  setupUI();
}

void DetectionSettingsPage::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(24);

  // 全局设置组
  auto* globalGroup = createStyledGroupBox(tr("全局设置"), this);
  auto* globalLayout = new QVBoxLayout(globalGroup);
  globalLayout->setContentsMargins(20, 20, 20, 20);
  globalLayout->setSpacing(16);

  auto* modeContainer = new QWidget();
  auto* modeLayout = new QHBoxLayout(modeContainer);
  modeLayout->setContentsMargins(0, 0, 0, 0);
  modeLayout->setSpacing(20);

  m_enableDetectionCheck = new QCheckBox(tr("启用检测"));
  m_enableDetectionCheck->setChecked(true);
  modeLayout->addWidget(m_enableDetectionCheck);

  modeLayout->addWidget(new QLabel(tr("检测模式:")));
  m_detectModeCombo = new QComboBox();
  m_detectModeCombo->addItems({tr("标准模式"), tr("快速模式"), tr("精确模式")});
  m_detectModeCombo->setMinimumHeight(32);
  m_detectModeCombo->setMinimumWidth(150);
  modeLayout->addWidget(m_detectModeCombo);

  modeLayout->addWidget(new QLabel(tr("置信度阈值:")));
  m_confidenceSlider = new QSlider(Qt::Horizontal);
  m_confidenceSlider->setRange(0, 100);
  m_confidenceSlider->setValue(75);
  m_confidenceSlider->setMinimumWidth(150);
  modeLayout->addWidget(m_confidenceSlider);

  m_confidenceSpin = new QSpinBox();
  m_confidenceSpin->setRange(0, 100);
  m_confidenceSpin->setValue(75);
  m_confidenceSpin->setSuffix(" %");
  m_confidenceSpin->setMinimumHeight(32);
  modeLayout->addWidget(m_confidenceSpin);

  connect(m_confidenceSlider, &QSlider::valueChanged, m_confidenceSpin, &QSpinBox::setValue);
  connect(m_confidenceSpin, QOverload<int>::of(&QSpinBox::valueChanged), m_confidenceSlider, &QSlider::setValue);

  modeLayout->addStretch();
  globalLayout->addWidget(modeContainer);

  // 模型路径
  auto* modelContainer = new QWidget();
  auto* modelLayout = new QHBoxLayout(modelContainer);
  modelLayout->setContentsMargins(0, 0, 0, 0);
  modelLayout->setSpacing(12);
  modelLayout->addWidget(new QLabel(tr("模型路径:")));
  m_modelPathEdit = new QLineEdit();
  m_modelPathEdit->setMinimumHeight(32);
  modelLayout->addWidget(m_modelPathEdit, 1);
  globalLayout->addWidget(modelContainer);

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
  auto* scratchGroup = createStyledGroupBox(tr("划痕检测参数"), this);
  auto* scratchLayout = new QGridLayout(scratchGroup);
  scratchLayout->setContentsMargins(20, 20, 20, 20);
  scratchLayout->setHorizontalSpacing(30);
  scratchLayout->setVerticalSpacing(16);

  scratchLayout->addWidget(new QLabel(tr("最小长度 (像素)")), 0, 0);
  scratchLayout->addWidget(createSpinBoxWithUnit(10, 500, 50, "", this), 0, 1);

  scratchLayout->addWidget(new QLabel(tr("最大宽度 (像素)")), 0, 2);
  scratchLayout->addWidget(createSpinBoxWithUnit(1, 50, 5, "", this), 0, 3);

  scratchLayout->addWidget(new QLabel(tr("灵敏度")), 1, 0);
  scratchLayout->addWidget(createSliderGroup(0, 100, 80, "", this), 1, 1, 1, 2);

  layout->addWidget(scratchGroup);

  // 裂纹检测参数组
  auto* crackGroup = createStyledGroupBox(tr("裂纹检测参数"), this);
  auto* crackLayout = new QGridLayout(crackGroup);
  crackLayout->setContentsMargins(20, 20, 20, 20);
  crackLayout->setHorizontalSpacing(30);
  crackLayout->setVerticalSpacing(16);

  crackLayout->addWidget(new QLabel(tr("最小面积 (像素²)")), 0, 0);
  crackLayout->addWidget(createSpinBoxWithUnit(50, 5000, 100, "", this), 0, 1);

  crackLayout->addWidget(new QLabel(tr("二值化阈值")), 0, 2);
  crackLayout->addWidget(createSpinBoxWithUnit(50, 200, 128, "", this), 0, 3);

  crackLayout->addWidget(new QLabel(tr("形态学核大小")), 1, 0);
  crackLayout->addWidget(createSpinBoxWithUnit(1, 10, 3, "", this), 1, 1);

  layout->addWidget(crackGroup);
  layout->addStretch();
}

void DetectionSettingsPage::loadSettings() {
  auto detCfg = gConfig.detectionConfig();
  if (m_enableDetectionCheck) m_enableDetectionCheck->setChecked(detCfg.enabled);
  if (m_confidenceSlider) m_confidenceSlider->setValue(static_cast<int>(detCfg.confidenceThreshold * 100));
  if (m_confidenceSpin) m_confidenceSpin->setValue(static_cast<int>(detCfg.confidenceThreshold * 100));
  if (m_modelPathEdit) m_modelPathEdit->setText(detCfg.modelPath);
}

void DetectionSettingsPage::saveSettings() {
  DetectionConfig detCfg = gConfig.detectionConfig();
  if (m_enableDetectionCheck) detCfg.enabled = m_enableDetectionCheck->isChecked();
  if (m_confidenceSpin) detCfg.confidenceThreshold = m_confidenceSpin->value() / 100.0;
  if (m_modelPathEdit) detCfg.modelPath = m_modelPathEdit->text();
  gConfig.setDetectionConfig(detCfg);
  emit settingsChanged();
}
