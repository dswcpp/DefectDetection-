#include "LightSettingsPage.h"
#include "SettingsPageUtils.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace SettingsUtils;

LightSettingsPage::LightSettingsPage(QWidget* parent) : QWidget(parent) {
  setupUI();
}

void LightSettingsPage::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(24);

  // 光源配置组
  auto* configGroup = createStyledGroupBox(tr("光源配置"), this);
  auto* configLayout = new QGridLayout(configGroup);
  configLayout->setContentsMargins(20, 20, 20, 20);
  configLayout->setHorizontalSpacing(20);
  configLayout->setVerticalSpacing(16);

  configLayout->addWidget(new QLabel(tr("控制方式:")), 0, 0);
  auto* controlCombo = new QComboBox();
  controlCombo->addItems({tr("串口控制"), tr("网络控制"), tr("模拟控制")});
  controlCombo->setMinimumHeight(32);
  configLayout->addWidget(controlCombo, 0, 1);

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
  auto* channelGroup = createStyledGroupBox(tr("通道配置"), this);
  auto* channelLayout = new QVBoxLayout(channelGroup);
  channelLayout->setContentsMargins(20, 20, 20, 20);
  channelLayout->setSpacing(16);

  channelLayout->addWidget(createCheckableSliderGroup(tr("通道 1 (正面)"), 0, 255, 200, true, this));
  channelLayout->addWidget(createCheckableSliderGroup(tr("通道 2 (侧光)"), 0, 255, 200, true, this));
  channelLayout->addWidget(createCheckableSliderGroup(tr("通道 3 (背光)"), 0, 255, 200, false, this));
  channelLayout->addWidget(createCheckableSliderGroup(tr("通道 4 (备用)"), 0, 255, 200, false, this));

  layout->addWidget(channelGroup);

  // 频闪设置组
  auto* strobeGroup = createStyledGroupBox(tr("频闪设置"), this);
  auto* strobeLayout = new QVBoxLayout(strobeGroup);
  strobeLayout->setContentsMargins(20, 20, 20, 20);
  strobeLayout->setSpacing(16);

  auto* enableStrobe = new QCheckBox(tr("启用频闪模式"));
  enableStrobe->setStyleSheet("QCheckBox { font-size: 14px; color: #E0E0E0; }");
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
    QPushButton { background-color: #FF9800; border: none; border-radius: 4px; color: white; }
    QPushButton:hover { background-color: #F57C00; }
  )");
  btnLayout->addWidget(openBtn);

  auto* closeBtn = new QPushButton(tr("关闭光源"));
  closeBtn->setMinimumHeight(36);
  closeBtn->setMinimumWidth(120);
  closeBtn->setStyleSheet(R"(
    QPushButton { background-color: #3C3C3E; border: 1px solid #555; border-radius: 4px; color: #E0E0E0; }
    QPushButton:hover { background-color: #48484A; }
  )");
  btnLayout->addWidget(closeBtn);
  btnLayout->addStretch();

  layout->addWidget(btnContainer);
  layout->addStretch();
}

void LightSettingsPage::loadSettings() {
  // TODO: Load light settings from config
}

void LightSettingsPage::saveSettings() {
  // TODO: Save light settings to config
  emit settingsChanged();
}
