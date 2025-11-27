#ifndef PARAMPANEL_H
#define PARAMPANEL_H

#include <QVariantMap>
#include <QWidget>
#include <QHash>

class QTabWidget;
class QAbstractSpinBox;
class QCheckBox;

class ParamPanel : public QWidget {
  Q_OBJECT
public:
  explicit ParamPanel(QWidget* parent = nullptr);

  void loadParams(const QString& configPath);
  void saveParams(const QString& configPath) const;
  QVariantMap getDetectorParams(const QString& detector) const;
  void setDetectorParams(const QString& detector, const QVariantMap& params);

signals:
  void paramsChanged(const QString& detector, const QVariantMap& params);

private:
  enum class ControlType { Bool, Int, Double };
  struct ControlInfo {
    ControlType type;
    QWidget* widget = nullptr;
  };

  QWidget* createScratchPage();
  QWidget* createCrackPage();
  QWidget* createForeignPage();
  QWidget* createDimensionPage();
  QWidget* createPage() const;

  QAbstractSpinBox* createDoubleSpin(const QString& detector, const QString& key,
                                     double min, double max, double step, double value,
                                     const QString& suffix = QString());
  QAbstractSpinBox* createIntSpin(const QString& detector, const QString& key,
                                  int min, int max, int step, int value,
                                  const QString& suffix = QString());
  QCheckBox* createCheckBox(const QString& detector, const QString& key, bool value);

  void registerControl(const QString& detector, const QString& key,
                       ControlType type, QWidget* widget);
  void emitParamsChanged(const QString& detector);

  QTabWidget* m_tabWidget;
  QHash<QString, QHash<QString, ControlInfo>> m_controls;
};

#endif // PARAMPANEL_H
