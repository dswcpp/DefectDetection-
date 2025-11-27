#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>

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
  void updateSectionHeader(int index);

  QListWidget* m_navList;
  QStackedWidget* m_stackedWidget;
  QLabel* m_sectionIconLabel;
  QLabel* m_sectionTitleLabel;

  // 各设置页
  QWidget* createCameraPage();
  QWidget* createLightPage();
  QWidget* createPLCPage();
  QWidget* createStoragePage();
  QWidget* createDetectorPage();
  QWidget* createUserPage();
};

#endif // SETTINGSDIALOG_H
