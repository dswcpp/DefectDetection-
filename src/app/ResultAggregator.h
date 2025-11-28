#ifndef RESULTAGGREGATOR_H
#define RESULTAGGREGATOR_H

#include "Types.h"
#include <QObject>
#include <vector>

class ResultAggregator : public QObject {
  Q_OBJECT
public:
  explicit ResultAggregator(QObject *parent = nullptr);

  // 聚合多工位结果
  DetectResult aggregate(const std::vector<DetectResult> &results);

  // 清空内部缓存
  void reset();

signals:
  void aggregated(const DetectResult &result);
};

#endif // RESULTAGGREGATOR_H
