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
