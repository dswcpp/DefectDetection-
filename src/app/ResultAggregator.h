/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ResultAggregator.h
 *
 * 优化版本：1.1
 * 作者：Vere
 * 修改日期：2026年02月
 * 摘要：结果聚合器接口定义
 * 描述：多工位/多相机检测结果聚合器，合并各路检测结果，
 *       生成综合判定，支持多种聚合策略
 */

#ifndef RESULTAGGREGATOR_H
#define RESULTAGGREGATOR_H

#include "Types.h"
#include <QObject>
#include <QMap>
#include <QMutex>
#include <QTimer>
#include <vector>

class ResultAggregator : public QObject {
  Q_OBJECT
public:
  // 聚合策略
  enum class Strategy {
    Any,        // 任一NG则NG（最严格）
    All,        // 全部NG才NG（最宽松）
    Majority,   // 多数决定
    Weighted,   // 加权评分
    MaxSeverity // 取最高严重度
  };
  Q_ENUM(Strategy)

  explicit ResultAggregator(QObject *parent = nullptr);
  ~ResultAggregator() = default;

  // 配置
  void setStrategy(Strategy strategy) { m_strategy = strategy; }
  Strategy strategy() const { return m_strategy; }
  
  void setExpectedCount(int count) { m_expectedCount = count; }
  int expectedCount() const { return m_expectedCount; }
  
  void setStationWeight(int stationId, double weight);
  double stationWeight(int stationId) const;
  
  void setTimeout(int ms) { m_timeoutMs = ms; }
  int timeout() const { return m_timeoutMs; }

  // 添加单个工位结果（异步聚合模式）
  void addResult(int stationId, const DetectResult& result);
  
  // 聚合多工位结果（同步模式）
  DetectResult aggregate(const std::vector<DetectResult>& results);
  
  // 带工位ID的聚合
  DetectResult aggregateWithStations(const QMap<int, DetectResult>& results);

  // 清空内部缓存
  void reset();
  
  // 查询状态
  bool isComplete() const;
  int pendingCount() const;

signals:
  void resultReceived(int stationId);
  void aggregated(const DetectResult& result);
  void timeout();

private slots:
  void onTimeout();

private:
  DetectResult aggregateInternal(const QMap<int, DetectResult>& results);
  DetectResult aggregateAny(const QMap<int, DetectResult>& results);
  DetectResult aggregateAll(const QMap<int, DetectResult>& results);
  DetectResult aggregateMajority(const QMap<int, DetectResult>& results);
  DetectResult aggregateWeighted(const QMap<int, DetectResult>& results);
  DetectResult aggregateMaxSeverity(const QMap<int, DetectResult>& results);
  void mergeDefects(DetectResult& target, const DetectResult& source);

  Strategy m_strategy = Strategy::Any;
  int m_expectedCount = 1;
  int m_timeoutMs = 5000;
  
  QMap<int, double> m_stationWeights;
  QMap<int, DetectResult> m_pendingResults;
  QTimer* m_timer = nullptr;
  mutable QMutex m_mutex;
};

#endif // RESULTAGGREGATOR_H
