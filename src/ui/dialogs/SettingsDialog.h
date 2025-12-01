#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QString>

class QStackedWidget;
class QLabel;
class QWidget;
class QListWidget;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QSlider;
class QCheckBox;

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
  void onBrowseImageDir();

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

  // ========== Camera Page Widgets ==========
  QComboBox* m_cameraTypeCombo = nullptr;
  QLineEdit* m_cameraIpEdit = nullptr;
  QLineEdit* m_imageDirEdit = nullptr;
  QSlider* m_exposureSlider = nullptr;
  QSpinBox* m_exposureSpin = nullptr;
  QSlider* m_gainSlider = nullptr;
  QSpinBox* m_gainSpin = nullptr;
  QComboBox* m_triggerCombo = nullptr;
  QSpinBox* m_captureIntervalSpin = nullptr;
  QCheckBox* m_loopCheck = nullptr;

  // ========== Detection Page Widgets ==========
  QComboBox* m_detectModeCombo = nullptr;
  QSlider* m_confidenceSlider = nullptr;
  QSpinBox* m_confidenceSpin = nullptr;
  QCheckBox* m_enableDetectionCheck = nullptr;
  QLineEdit* m_modelPathEdit = nullptr;

  // ========== Storage Page Widgets ==========
  QLineEdit* m_dbPathEdit = nullptr;
  QSpinBox* m_maxRecordsSpin = nullptr;
  QCheckBox* m_autoCleanupCheck = nullptr;
  QLineEdit* m_logDirEdit = nullptr;
  QSpinBox* m_logMaxSizeSpin = nullptr;
  QSpinBox* m_logMaxCountSpin = nullptr;
};

#endif // SETTINGSDIALOG_H