#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVector>

class SettingsDialog : public QDialog {
  Q_OBJECT
public:
  explicit SettingsDialog(QWidget* parent = nullptr);

  void loadSettings();
  void saveSettings();

signals:
  void settingsChanged();

private:
  struct Section {
    QString id;
    QString title;
    QString icon;
    QWidget* page = nullptr;
    QPushButton* button = nullptr;
  };

  void setupUI();
  void buildSections();
  QWidget* createSectionWidget(const QString& id);
  QPushButton* createNavButton(int index);
  void setActiveSection(int index);
  void onRestoreDefaultClicked();
  void onApplyClicked();

  // Section pages
  QWidget* createCameraPage();
  QWidget* createLightPage();
  QWidget* createPLCPage();
  QWidget* createStoragePage();
  QWidget* createDetectorPage();
  QWidget* createUserPage();

  QVector<Section> m_sections;
  int m_activeSection = -1;

  QStackedWidget* m_stackedWidget = nullptr;
  QLabel* m_sectionIconLabel = nullptr;
  QLabel* m_sectionTitleLabel = nullptr;
};

#endif // SETTINGSDIALOG_H
