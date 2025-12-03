/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * DetectionSettingsPage.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：检测设置页面接口定义
 * 描述：检测参数设置页面，配置各检测器的启用状态和参数
 *
 * 当前版本：1.0
 */

#ifndef DETECTIONSETTINGSPAGE_H
#define DETECTIONSETTINGSPAGE_H

#include <QWidget>

class QComboBox;
class QSlider;
class QSpinBox;
class QCheckBox;
class QLineEdit;

class DetectionSettingsPage : public QWidget {
  Q_OBJECT
public:
  explicit DetectionSettingsPage(QWidget* parent = nullptr);

  void loadSettings();
  void saveSettings();

signals:
  void settingsChanged();

private:
  void setupUI();

  QComboBox* m_detectModeCombo = nullptr;
  QSlider* m_confidenceSlider = nullptr;
  QSpinBox* m_confidenceSpin = nullptr;
  QCheckBox* m_enableDetectionCheck = nullptr;
  QLineEdit* m_modelPathEdit = nullptr;
};

#endif  // DETECTIONSETTINGSPAGE_H
