#include "PLCSettingsPage.h"
#include "SettingsPageUtils.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace SettingsUtils;

PLCSettingsPage::PLCSettingsPage(QWidget* parent) : QWidget(parent) {
  setupUI();
}

void PLCSettingsPage::setupUI() {
  auto* layout = new QVBoxLayout(this);
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

  auto* connStatusTitle = new QLabel(tr("连接状态"));
  connStatusTitle->setStyleSheet("color: #E0E0E0;");
  commLayout->addWidget(connStatusTitle);
  auto* statusLabel = new QLabel(tr("● 未连接"));
  statusLabel->setStyleSheet("color: #ef4444; background-color: #3C2020; padding: 6px 12px; border-radius: 4px;");
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
  auto* signalGroup = createStyledGroupBox(tr("信号映射"), this);
  auto* signalLayout = new QGridLayout(signalGroup);
  signalLayout->setContentsMargins(20, 20, 20, 20);
  signalLayout->setHorizontalSpacing(30);
  signalLayout->setVerticalSpacing(12);

  signalLayout->addWidget(new QLabel(tr("触发信号 (输入)")), 0, 0);
  signalLayout->addWidget(new QLabel(tr("寄存器地址")), 0, 1);
  auto* triggerEdit = new QLineEdit("M100");
  triggerEdit->setMinimumHeight(32);
  signalLayout->addWidget(triggerEdit, 0, 2);

  signalLayout->addWidget(new QLabel(tr("检测完成 (输出)")), 1, 0);
  signalLayout->addWidget(new QLabel(tr("寄存器地址")), 1, 1);
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
    QPushButton { background-color: #5C6BC0; border: none; border-radius: 4px; color: white; }
    QPushButton:hover { background-color: #3F51B5; }
  )");
  btnLayout->addWidget(testBtn);

  auto* connectBtn = new QPushButton(tr("连接 PLC"));
  connectBtn->setMinimumHeight(36);
  connectBtn->setMinimumWidth(120);
  connectBtn->setStyleSheet(R"(
    QPushButton { background-color: #4CAF50; border: none; border-radius: 4px; color: white; }
    QPushButton:hover { background-color: #43A047; }
  )");
  btnLayout->addWidget(connectBtn);

  auto* disconnectBtn = new QPushButton(tr("断开连接"));
  disconnectBtn->setMinimumHeight(36);
  disconnectBtn->setMinimumWidth(120);
  disconnectBtn->setStyleSheet(R"(
    QPushButton { background-color: #3C3C3E; border: 1px solid #555; border-radius: 4px; color: #E0E0E0; }
    QPushButton:hover { background-color: #48484A; }
  )");
  btnLayout->addWidget(disconnectBtn);
  btnLayout->addStretch();

  layout->addWidget(btnContainer);
  layout->addStretch();
}

void PLCSettingsPage::loadSettings() {
  // TODO: Load PLC settings from config
}

void PLCSettingsPage::saveSettings() {
  // TODO: Save PLC settings to config
  emit settingsChanged();
}
