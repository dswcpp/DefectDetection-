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
