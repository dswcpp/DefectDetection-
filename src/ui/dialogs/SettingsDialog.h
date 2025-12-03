/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * SettingsDialog.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：设置对话框接口定义
 * 描述：系统设置对话框，包含多个设置页面（相机、光源、检测、存储等）
 *
 * 当前版本：1.0
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QStackedWidget;
class QLabel;
class QListWidget;
class CameraSettingsPage;
class LightSettingsPage;
class PLCSettingsPage;
class StorageSettingsPage;
class DetectionSettingsPage;
class UserSettingsPage;

class SettingsDialog : public QDialog {
  Q_OBJECT
public:
  explicit SettingsDialog(QWidget* parent = nullptr);

signals:
  void settingsChanged();

private slots:
  void onRestoreDefaultClicked();
  void onApplyClicked();
  void onPageChanged(int index);

private:
  void setupUI();
  void createPages();
  void loadSettings();
  void saveSettings();

  // UI Elements
  QStackedWidget* m_stackedWidget = nullptr;
  QListWidget* m_navListWidget = nullptr;
  QLabel* m_pageTitleLabel = nullptr;
  QLabel* m_pageIconLabel = nullptr;

  // Pages
  CameraSettingsPage* m_cameraPage = nullptr;
  LightSettingsPage* m_lightPage = nullptr;
  PLCSettingsPage* m_plcPage = nullptr;
  StorageSettingsPage* m_storagePage = nullptr;
  DetectionSettingsPage* m_detectionPage = nullptr;
  UserSettingsPage* m_userPage = nullptr;
};

#endif // SETTINGSDIALOG_H
