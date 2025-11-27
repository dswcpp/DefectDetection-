#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>

class SettingsDialog : public QDialog {
  Q_OBJECT
public:
  explicit SettingsDialog(QWidget* parent = nullptr);

  void loadSettings();
  void saveSettings();

signals:
  void settingsChanged();

private slots:
  void onNavItemClicked(int index);
  void onRestoreDefaultClicked();
  void onApplyClicked();

private:
  void setupUI();
  void createNavList();
  void createStackedPages();

  QListWidget* m_navList;
  QStackedWidget* m_stackedWidget;

  // 各设置页
  QWidget* createCameraPage();
  QWidget* createLightPage();
  QWidget* createPLCPage();
  QWidget* createStoragePage();
  QWidget* createDetectorPage();
  QWidget* createUserPage();
};

#endif // SETTINGSDIALOG_H
