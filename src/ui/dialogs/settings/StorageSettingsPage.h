/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * StorageSettingsPage.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：存储设置页面接口定义
 * 描述：数据存储设置页面，配置图像保存路径、保留天数、备份策略等
 *
 * 当前版本：1.0
 */

#ifndef STORAGESETTINGSPAGE_H
#define STORAGESETTINGSPAGE_H

#include <QWidget>
#include <QMap>

class QLineEdit;
class QSpinBox;
class QCheckBox;
class QLabel;
class QProgressBar;
class QVBoxLayout;
struct StorageInfo;

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
  void createStorageInfoWidget(QVBoxLayout* layout, const QString& name, const StorageInfo& info);
  void updateStorageInfoWidgets();

  QLineEdit* m_dbPathEdit = nullptr;
  QLineEdit* m_imagePathEdit = nullptr;
  QSpinBox* m_maxRecordsSpin = nullptr;
  QCheckBox* m_autoCleanupCheck = nullptr;
  QLineEdit* m_logDirEdit = nullptr;
  QSpinBox* m_logMaxSizeSpin = nullptr;
  QSpinBox* m_logMaxCountSpin = nullptr;
  
  // 磁盘空间显示（多路径）
  QWidget* m_storageInfoContainer = nullptr;
  QVBoxLayout* m_storageInfoLayout = nullptr;
  QMap<QString, QWidget*> m_storageWidgets;
};

#endif  // STORAGESETTINGSPAGE_H
