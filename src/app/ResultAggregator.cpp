/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ResultAggregator.cpp
 *
 * 优化版本：1.1
 * 作者：Vere
 * 修改日期：2026年02月
 * 摘要：结果聚合器实现
 */

#include "ResultAggregator.h"
#include "common/Logger.h"
#include <QTimer>
#include <QDateTime>
#include <algorithm>
#include <numeric>

ResultAggregator::ResultAggregator(QObject *parent) 
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &ResultAggregator::onTimeout);
}

void ResultAggregator::setStationWeight(int stationId, double weight)
{
    QMutexLocker locker(&m_mutex);
    m_stationWeights[stationId] = qBound(0.0, weight, 1.0);
}

double ResultAggregator::stationWeight(int stationId) const
{
    QMutexLocker locker(&m_mutex);
    return m_stationWeights.value(stationId, 1.0);
}

void ResultAggregator::addResult(int stationId, const DetectResult& result)
{
    QMutexLocker locker(&m_mutex);
    
    m_pendingResults[stationId] = result;
    emit resultReceived(stationId);
    
    LOG_DEBUG("ResultAggregator: Received result from station {}, pending: {}/{}",
              stationId, m_pendingResults.size(), m_expectedCount);
    
    // 检查是否收集完毕
    if (m_pendingResults.size() >= m_expectedCount) {
        m_timer->stop();
        
        // 聚合并发送
        DetectResult aggregated = aggregateInternal(m_pendingResults);
        m_pendingResults.clear();
        
        locker.unlock();
        emit this->aggregated(aggregated);
    } else if (!m_timer->isActive() && m_timeoutMs > 0) {
        // 启动超时计时器
        m_timer->start(m_timeoutMs);
    }
}

DetectResult ResultAggregator::aggregate(const std::vector<DetectResult>& results)
{
    QMap<int, DetectResult> stationResults;
    for (size_t i = 0; i < results.size(); ++i) {
        stationResults[static_cast<int>(i)] = results[i];
    }
    return aggregateInternal(stationResults);
}

DetectResult ResultAggregator::aggregateWithStations(const QMap<int, DetectResult>& results)
{
    return aggregateInternal(results);
}

void ResultAggregator::reset()
{
    QMutexLocker locker(&m_mutex);
    m_pendingResults.clear();
    m_timer->stop();
    LOG_DEBUG("ResultAggregator: Reset");
}

bool ResultAggregator::isComplete() const
{
    QMutexLocker locker(&m_mutex);
    return m_pendingResults.size() >= m_expectedCount;
}

int ResultAggregator::pendingCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_pendingResults.size();
}

void ResultAggregator::onTimeout()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_pendingResults.isEmpty()) {
        return;
    }
    
    LOG_WARN("ResultAggregator: Timeout with {}/{} results, aggregating partial",
             m_pendingResults.size(), m_expectedCount);
    
    // 聚合已有结果
    DetectResult aggregated = aggregateInternal(m_pendingResults);
    aggregated.errorMsg = QString("Timeout: only %1/%2 stations responded")
                              .arg(m_pendingResults.size())
                              .arg(m_expectedCount);
    m_pendingResults.clear();
    
    locker.unlock();
    emit timeout();
    emit this->aggregated(aggregated);
}

DetectResult ResultAggregator::aggregateInternal(const QMap<int, DetectResult>& results)
{
    if (results.isEmpty()) {
        DetectResult empty;
        empty.isOK = true;
        empty.level = SeverityLevel::OK;
        empty.timestamp = QDateTime::currentMSecsSinceEpoch();
        return empty;
    }
    
    if (results.size() == 1) {
        return results.first();
    }
    
    DetectResult aggregated;
    
    switch (m_strategy) {
    case Strategy::Any:
        aggregated = aggregateAny(results);
        break;
    case Strategy::All:
        aggregated = aggregateAll(results);
        break;
    case Strategy::Majority:
        aggregated = aggregateMajority(results);
        break;
    case Strategy::Weighted:
        aggregated = aggregateWeighted(results);
        break;
    case Strategy::MaxSeverity:
        aggregated = aggregateMaxSeverity(results);
        break;
    }
    
    // 合并所有缺陷区域
    for (const auto& result : results) {
        mergeDefects(aggregated, result);
    }
    
    // 计算总耗时
    int totalTime = 0;
    for (const auto& result : results) {
        totalTime = std::max(totalTime, result.cycleTimeMs);
    }
    aggregated.cycleTimeMs = totalTime;
    aggregated.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    LOG_INFO("ResultAggregator: Aggregated {} results -> isOK={}, severity={:.2f}, level={}",
             results.size(), aggregated.isOK, aggregated.severity,
             static_cast<int>(aggregated.level));
    
    return aggregated;
}

DetectResult ResultAggregator::aggregateAny(const QMap<int, DetectResult>& results)
{
    // 任一 NG 则整体 NG
    DetectResult aggregated;
    aggregated.isOK = true;
    aggregated.severity = 0.0;
    aggregated.level = SeverityLevel::OK;
    
    for (const auto& result : results) {
        if (!result.isOK) {
            aggregated.isOK = false;
        }
        aggregated.severity = std::max(aggregated.severity, result.severity);
        if (result.level > aggregated.level) {
            aggregated.level = result.level;
        }
    }
    
    return aggregated;
}

DetectResult ResultAggregator::aggregateAll(const QMap<int, DetectResult>& results)
{
    // 全部 NG 才整体 NG
    DetectResult aggregated;
    aggregated.isOK = false;
    aggregated.severity = 0.0;
    aggregated.level = SeverityLevel::Critical;
    
    bool hasOK = false;
    for (const auto& result : results) {
        if (result.isOK) {
            hasOK = true;
        }
        aggregated.severity = std::max(aggregated.severity, result.severity);
        if (result.level < aggregated.level) {
            aggregated.level = result.level;
        }
    }
    
    aggregated.isOK = hasOK;
    if (hasOK) {
        aggregated.level = SeverityLevel::OK;
    }
    
    return aggregated;
}

DetectResult ResultAggregator::aggregateMajority(const QMap<int, DetectResult>& results)
{
    // 多数决定
    DetectResult aggregated;
    int okCount = 0;
    int ngCount = 0;
    double totalSeverity = 0.0;
    
    for (const auto& result : results) {
        if (result.isOK) {
            okCount++;
        } else {
            ngCount++;
        }
        totalSeverity += result.severity;
    }
    
    aggregated.isOK = (okCount >= ngCount);
    aggregated.severity = totalSeverity / results.size();
    
    // 根据平均严重度确定等级
    if (aggregated.severity < 0.3) {
        aggregated.level = SeverityLevel::OK;
    } else if (aggregated.severity < 0.5) {
        aggregated.level = SeverityLevel::Minor;
    } else if (aggregated.severity < 0.8) {
        aggregated.level = SeverityLevel::Major;
    } else {
        aggregated.level = SeverityLevel::Critical;
    }
    
    return aggregated;
}

DetectResult ResultAggregator::aggregateWeighted(const QMap<int, DetectResult>& results)
{
    // 加权评分
    DetectResult aggregated;
    double weightedScore = 0.0;
    double totalWeight = 0.0;
    double maxSeverity = 0.0;
    
    for (auto it = results.begin(); it != results.end(); ++it) {
        int stationId = it.key();
        const DetectResult& result = it.value();
        
        double weight = m_stationWeights.value(stationId, 1.0);
        double score = result.isOK ? 1.0 : 0.0;
        
        weightedScore += score * weight;
        totalWeight += weight;
        maxSeverity = std::max(maxSeverity, result.severity);
    }
    
    // 加权得分 > 0.5 则 OK
    double normalizedScore = (totalWeight > 0) ? weightedScore / totalWeight : 0.0;
    aggregated.isOK = (normalizedScore > 0.5);
    aggregated.severity = maxSeverity;
    aggregated.confidence = normalizedScore;
    
    // 根据得分确定等级
    if (normalizedScore > 0.8) {
        aggregated.level = SeverityLevel::OK;
    } else if (normalizedScore > 0.6) {
        aggregated.level = SeverityLevel::Minor;
    } else if (normalizedScore > 0.3) {
        aggregated.level = SeverityLevel::Major;
    } else {
        aggregated.level = SeverityLevel::Critical;
    }
    
    return aggregated;
}

DetectResult ResultAggregator::aggregateMaxSeverity(const QMap<int, DetectResult>& results)
{
    // 取最高严重度
    DetectResult aggregated;
    aggregated.isOK = true;
    aggregated.severity = 0.0;
    aggregated.level = SeverityLevel::OK;
    
    const DetectResult* worstResult = nullptr;
    
    for (const auto& result : results) {
        if (result.severity > aggregated.severity) {
            aggregated.severity = result.severity;
            worstResult = &result;
        }
        if (!result.isOK) {
            aggregated.isOK = false;
        }
    }
    
    if (worstResult) {
        aggregated.level = worstResult->level;
        aggregated.defectType = worstResult->defectType;
    }
    
    return aggregated;
}

void ResultAggregator::mergeDefects(DetectResult& target, const DetectResult& source)
{
    // 合并缺陷区域
    target.regions.insert(target.regions.end(), 
                          source.regions.begin(), 
                          source.regions.end());
    
    // 合并缺陷详情
    target.defects.insert(target.defects.end(),
                          source.defects.begin(),
                          source.defects.end());
    
    // 合并缺陷类型描述
    if (!source.defectType.isEmpty()) {
        if (target.defectType.isEmpty()) {
            target.defectType = source.defectType;
        } else if (!target.defectType.contains(source.defectType)) {
            target.defectType += ", " + source.defectType;
        }
    }
}
