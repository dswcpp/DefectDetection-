/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * CameraSettingsPage.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：相机设置页面接口定义
 * 描述：相机参数设置页面，配置相机类型、曝光、增益、触发模式等
 *
 * 当前版本：1.0
 */

#ifndef CAMERASETTINGSPAGE_H
#define CAMERASETTINGSPAGE_H

#include <QWidget>

class QComboBox;
class QLineEdit;
class QSpinBox;
class QSlider;
class QCheckBox;

class CameraSettingsPage : public QWidget {
  Q_OBJECT
public:
  explicit CameraSettingsPage(QWidget* parent = nullptr);

  void loadSettings();
  void saveSettings();

signals:
  void settingsChanged();

private slots:
  void onBrowseImageDir();

private:
  void setupUI();

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
};

#endif  // CAMERASETTINGSPAGE_H
