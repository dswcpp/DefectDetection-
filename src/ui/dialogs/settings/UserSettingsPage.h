#ifndef USERSETTINGSPAGE_H
#define USERSETTINGSPAGE_H

#include <QWidget>

class QTableWidget;

class UserSettingsPage : public QWidget {
  Q_OBJECT
public:
  explicit UserSettingsPage(QWidget* parent = nullptr);

  void loadSettings();
  void saveSettings();

signals:
  void settingsChanged();

private:
  void setupUI();
  void refreshUserList();

  QTableWidget* m_userTable = nullptr;
};

#endif  // USERSETTINGSPAGE_H
