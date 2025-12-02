#ifndef STORAGESETTINGSPAGE_H
#define STORAGESETTINGSPAGE_H

#include <QWidget>

class QLineEdit;
class QSpinBox;
class QCheckBox;
class QLabel;
class QProgressBar;

class StorageSettingsPage : public QWidget {
  Q_OBJECT
public:
  explicit StorageSettingsPage(QWidget* parent = nullptr);

  void loadSettings();
  void saveSettings();

signals:
  void settingsChanged();

private slots:
  void updateDiskSpaceInfo();

private:
  void setupUI();
  QString formatSize(qint64 bytes) const;

  QLineEdit* m_dbPathEdit = nullptr;
  QLineEdit* m_imagePathEdit = nullptr;
  QSpinBox* m_maxRecordsSpin = nullptr;
  QCheckBox* m_autoCleanupCheck = nullptr;
  QLineEdit* m_logDirEdit = nullptr;
  QSpinBox* m_logMaxSizeSpin = nullptr;
  QSpinBox* m_logMaxCountSpin = nullptr;
  
  // 磁盘空间显示
  QLabel* m_spaceInfoLabel = nullptr;
  QProgressBar* m_spaceProgressBar = nullptr;
};

#endif  // STORAGESETTINGSPAGE_H
