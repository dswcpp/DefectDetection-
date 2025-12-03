/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * PLCSettingsPage.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：PLC设置页面接口定义
 * 描述：PLC通信设置页面，配置PLC类型、IP地址、端口、寄存器地址等
 *
 * 当前版本：1.0
 */

#ifndef PLCSETTINGSPAGE_H
#define PLCSETTINGSPAGE_H

#include <QWidget>

class PLCSettingsPage : public QWidget {
  Q_OBJECT
public:
  explicit PLCSettingsPage(QWidget* parent = nullptr);

  void loadSettings();
  void saveSettings();

signals:
  void settingsChanged();

private:
  void setupUI();
};

#endif  // PLCSETTINGSPAGE_H
