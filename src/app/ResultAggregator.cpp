#include "ResultAggregator.h"
#include "common/Logger.h"

ResultAggregator::ResultAggregator(QObject *parent) : QObject(parent) {}

DetectResult ResultAggregator::aggregate(const std::vector<DetectResult> &results) {
  LOG_DEBUG("ResultAggregator::aggregate - Aggregating {} results", results.size());
  // TODO: 聚合多工位结果，取最高严重度
  DetectResult combined;
  LOG_WARN("ResultAggregator::aggregate - Not implemented yet");
  emit aggregated(combined);
  return combined;
}

void ResultAggregator::reset() {
  LOG_DEBUG("ResultAggregator::reset - Resetting state");
  // TODO: 清理内部状态
}
