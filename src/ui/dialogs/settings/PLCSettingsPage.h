#ifndef PLCSETTINGSPAGE_H
#define PLCSETTINGSPAGE_H

#include <QWidget>

class PLCSettingsPage : public QWidget {
  Q_OBJECT
public:
  explicit PLCSettingsPage(QWidget* parent = nullptr);

  void loadSettings();
  void saveSettings();

signals:
  void settingsChanged();

private:
  void setupUI();
};

#endif  // PLCSETTINGSPAGE_H
