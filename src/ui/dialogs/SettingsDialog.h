#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QString>

class QStackedWidget;
class QLabel;
class QWidget;
class QListWidget;

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

  // Page creators
  QWidget* createCameraPage();
  QWidget* createLightPage();
  QWidget* createPLCPage();
  QWidget* createStoragePage();
  QWidget* createDetectionPage();
  QWidget* createUserPage();

  // UI Elements
  QStackedWidget* m_stackedWidget = nullptr;
  QListWidget* m_navListWidget = nullptr;
  QLabel* m_pageTitleLabel = nullptr;
  QLabel* m_pageIconLabel = nullptr;
  QWidget* m_pageTitleWidget = nullptr;
};

#endif // SETTINGSDIALOG_H