#ifndef LIGHTSETTINGSPAGE_H
#define LIGHTSETTINGSPAGE_H

#include <QWidget>

class LightSettingsPage : public QWidget {
  Q_OBJECT
public:
  explicit LightSettingsPage(QWidget* parent = nullptr);

  void loadSettings();
  void saveSettings();

signals:
  void settingsChanged();

private:
  void setupUI();
};

#endif  // LIGHTSETTINGSPAGE_H
