#include "ResultAggregator.h"

ResultAggregator::ResultAggregator(QObject *parent) : QObject(parent) {}

DetectResult ResultAggregator::aggregate(const std::vector<DetectResult> &results) {
  // TODO: 聚合多工位结果，取最高严重度
  DetectResult combined;
  emit aggregated(combined);
  return combined;
}

void ResultAggregator::reset() {
  // TODO: 清理内部状态
}
