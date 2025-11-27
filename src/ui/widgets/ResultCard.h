#ifndef RESULTCARD_H
#define RESULTCARD_H

#include <QFrame>
#include <QVector>
#include "Types.h"

class QLabel;
class QVBoxLayout;
class QWidget;

class ResultCard : public QFrame {
  Q_OBJECT
public:
  explicit ResultCard(QWidget* parent = nullptr);

  void setResult(const DetectResult& result);
  void clear();

private:
  void setupUI();
  void updateStatus(bool isOk);
  void clearDefectList();
  void rebuildDefectList(const QString& typeName, const std::vector<DefectRegion>& defects);

  QLabel* m_titleLabel = nullptr;
  QLabel* m_statusIcon = nullptr;
  QLabel* m_statusText = nullptr;
  QWidget* m_defectList = nullptr;
  QVBoxLayout* m_defectListLayout = nullptr;
  QLabel* m_emptyHintLabel = nullptr;
  QLabel* m_timestampLabel = nullptr;
};

#endif // RESULTCARD_H
