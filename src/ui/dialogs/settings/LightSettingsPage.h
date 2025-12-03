/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * LightSettingsPage.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：光源设置页面接口定义
 * 描述：光源参数设置页面，配置光源类型、亮度、频闪时间等
 *
 * 当前版本：1.0
 */

#ifndef LIGHTSETTINGSPAGE_H
#define LIGHTSETTINGSPAGE_H

#include <QWidget>

class LightSettingsPage : public QWidget {
  Q_OBJECT
public:
  explicit LightSettingsPage(QWidget* parent = nullptr);

  void loadSettings();
  void saveSettings();

signals:
  void settingsChanged();

private:
  void setupUI();
};

#endif  // LIGHTSETTINGSPAGE_H
