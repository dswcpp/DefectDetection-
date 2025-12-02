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
