/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * UserSettingsPage.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：用户设置页面接口定义
 * 描述：用户偏好设置页面，配置界面语言、主题、快捷键等
 *
 * 当前版本：1.0
 */

#ifndef USERSETTINGSPAGE_H
#define USERSETTINGSPAGE_H

#include <QWidget>

class QTableWidget;

class UserSettingsPage : public QWidget {
  Q_OBJECT
public:
  explicit UserSettingsPage(QWidget* parent = nullptr);

  void loadSettings();
  void saveSettings();

signals:
  void settingsChanged();

private:
  void setupUI();
  void refreshUserList();

  QTableWidget* m_userTable = nullptr;
};

#endif  // USERSETTINGSPAGE_H
