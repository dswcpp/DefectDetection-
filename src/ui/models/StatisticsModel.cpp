/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * StatisticsModel.cpp
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：统计数据模型实现
 *
 * 当前版本：1.0
 */

#include "StatisticsModel.h"
#include "data/repositories/DefectRepository.h"
#include "common/Logger.h"
#include <QBrush>
#include <algorithm>
#include <cmath>

// 默认颜色调色板
const QVector<QColor> StatisticsModel::s_defaultColors = {
    QColor("#e53935"),  // 红
    QColor("#fb8c00"),  // 橙
    QColor("#fdd835"),  // 黄
    QColor("#43a047"),  // 绿
    QColor("#1e88e5"),  // 蓝
    QColor("#8e24aa"),  // 紫
    QColor("#00acc1"),  // 青
    QColor("#6d4c41"),  // 棕
    QColor("#546e7a"),  // 灰蓝
    QColor("#d81b60"),  // 粉
};

StatisticsModel::StatisticsModel(QObject* parent)
    : QObject(parent)
{
    LOG_DEBUG("StatisticsModel created");
}

StatisticsModel::~StatisticsModel()
{
    LOG_DEBUG("StatisticsModel destroyed");
}

void StatisticsModel::setRepository(DefectRepository* repo)
{
    m_repo = repo;
}

TrendData StatisticsModel::getInspectionTrend(const QDateTime& start, const QDateTime& end,
                                               TimeGranularity granularity)
{
    TrendData trend;
    
    if (!m_repo) {
        LOG_WARN("StatisticsModel::getInspectionTrend - No repository set");
        return trend;
    }
    
    // 生成时间刻度
    QVector<QDateTime> timeScale = generateTimeScale(start, end, granularity);
    
    if (timeScale.isEmpty()) {
        return trend;
    }
    
    // 查询每个时间段的数据
    double totalValue = 0.0;
    trend.min = std::numeric_limits<double>::max();
    trend.max = std::numeric_limits<double>::min();
    
    for (int i = 0; i < timeScale.size() - 1; ++i) {
        InspectionFilter filter;
        filter.startTime = timeScale[i];
        filter.endTime = timeScale[i + 1];
        
        int count = m_repo->countInspections(filter);
        
        TimeSeriesPoint point;
        point.timestamp = timeScale[i];
        point.value = count;
        point.label = formatTimeLabel(timeScale[i], granularity);
        
        trend.points.append(point);
        
        totalValue += count;
        trend.min = std::min(trend.min, static_cast<double>(count));
        trend.max = std::max(trend.max, static_cast<double>(count));
    }
    
    if (trend.points.isEmpty()) {
        trend.min = 0;
        trend.max = 0;
    } else {
        trend.average = totalValue / trend.points.size();
    }
    
    trend.trend = calculateTrend(trend.points);
    trend.trendDescription = getTrendDescription(trend.trend);
    
    LOG_DEBUG("StatisticsModel::getInspectionTrend - {} points, avg={:.1f}, trend={}",
              trend.points.size(), trend.average, trend.trendDescription.toStdString());
    
    return trend;
}

TrendData StatisticsModel::getPassRateTrend(const QDateTime& start, const QDateTime& end,
                                             TimeGranularity granularity)
{
    TrendData trend;
    
    if (!m_repo) {
        return trend;
    }
    
    QVector<QDateTime> timeScale = generateTimeScale(start, end, granularity);
    
    if (timeScale.isEmpty()) {
        return trend;
    }
    
    double totalValue = 0.0;
    trend.min = 100.0;
    trend.max = 0.0;
    
    for (int i = 0; i < timeScale.size() - 1; ++i) {
        InspectionFilter filter;
        filter.startTime = timeScale[i];
        filter.endTime = timeScale[i + 1];
        
        int total = m_repo->countInspections(filter);
        
        filter.result = "OK";
        int passCount = m_repo->countInspections(filter);
        
        double passRate = (total > 0) ? (static_cast<double>(passCount) / total * 100.0) : 0.0;
        
        TimeSeriesPoint point;
        point.timestamp = timeScale[i];
        point.value = passRate;
        point.label = formatTimeLabel(timeScale[i], granularity);
        
        trend.points.append(point);
        
        totalValue += passRate;
        trend.min = std::min(trend.min, passRate);
        trend.max = std::max(trend.max, passRate);
    }
    
    if (trend.points.isEmpty()) {
        trend.min = 0;
        trend.max = 0;
    } else {
        trend.average = totalValue / trend.points.size();
    }
    
    trend.trend = calculateTrend(trend.points);
    trend.trendDescription = getTrendDescription(trend.trend);
    
    return trend;
}

TrendData StatisticsModel::getDefectCountTrend(const QDateTime& start, const QDateTime& end,
                                                TimeGranularity granularity)
{
    TrendData trend;
    
    if (!m_repo) {
        return trend;
    }
    
    QVector<QDateTime> timeScale = generateTimeScale(start, end, granularity);
    
    if (timeScale.isEmpty()) {
        return trend;
    }
    
    double totalValue = 0.0;
    trend.min = std::numeric_limits<double>::max();
    trend.max = std::numeric_limits<double>::min();
    
    for (int i = 0; i < timeScale.size() - 1; ++i) {
        InspectionFilter filter;
        filter.startTime = timeScale[i];
        filter.endTime = timeScale[i + 1];
        
        QVector<InspectionRecord> records = m_repo->queryInspections(filter);
        
        int defectSum = 0;
        for (const auto& record : records) {
            defectSum += record.defectCount;
        }
        
        TimeSeriesPoint point;
        point.timestamp = timeScale[i];
        point.value = defectSum;
        point.label = formatTimeLabel(timeScale[i], granularity);
        
        trend.points.append(point);
        
        totalValue += defectSum;
        trend.min = std::min(trend.min, static_cast<double>(defectSum));
        trend.max = std::max(trend.max, static_cast<double>(defectSum));
    }
    
    if (trend.points.isEmpty()) {
        trend.min = 0;
        trend.max = 0;
    } else {
        trend.average = totalValue / trend.points.size();
    }
    
    trend.trend = calculateTrend(trend.points);
    trend.trendDescription = getTrendDescription(trend.trend);
    
    return trend;
}

TrendData StatisticsModel::getCycleTimeTrend(const QDateTime& start, const QDateTime& end,
                                              TimeGranularity granularity)
{
    TrendData trend;
    
    if (!m_repo) {
        return trend;
    }
    
    QVector<QDateTime> timeScale = generateTimeScale(start, end, granularity);
    
    if (timeScale.isEmpty()) {
        return trend;
    }
    
    double totalValue = 0.0;
    trend.min = std::numeric_limits<double>::max();
    trend.max = std::numeric_limits<double>::min();
    
    for (int i = 0; i < timeScale.size() - 1; ++i) {
        InspectionFilter filter;
        filter.startTime = timeScale[i];
        filter.endTime = timeScale[i + 1];
        
        QVector<InspectionRecord> records = m_repo->queryInspections(filter);
        
        double avgCycleTime = 0.0;
        if (!records.isEmpty()) {
            double sum = 0.0;
            for (const auto& record : records) {
                sum += record.cycleTimeMs;
            }
            avgCycleTime = sum / records.size();
        }
        
        TimeSeriesPoint point;
        point.timestamp = timeScale[i];
        point.value = avgCycleTime;
        point.label = formatTimeLabel(timeScale[i], granularity);
        
        trend.points.append(point);
        
        totalValue += avgCycleTime;
        if (avgCycleTime > 0) {
            trend.min = std::min(trend.min, avgCycleTime);
            trend.max = std::max(trend.max, avgCycleTime);
        }
    }
    
    if (trend.points.isEmpty() || trend.min == std::numeric_limits<double>::max()) {
        trend.min = 0;
        trend.max = 0;
    } else {
        trend.average = totalValue / trend.points.size();
    }
    
    trend.trend = calculateTrend(trend.points);
    trend.trendDescription = getTrendDescription(trend.trend);
    
    return trend;
}

QVector<CategoryDataPoint> StatisticsModel::getDefectTypeDistribution(
    const QDateTime& start, const QDateTime& end)
{
    QVector<CategoryDataPoint> data;
    
    if (!m_repo) {
        return data;
    }
    
    InspectionFilter filter;
    filter.startTime = start;
    filter.endTime = end;
    
    // 查询缺陷类型统计
    QMap<QString, int> typeStats = m_repo->getDefectTypeStatistics(filter);
    
    int colorIndex = 0;
    for (auto it = typeStats.begin(); it != typeStats.end(); ++it) {
        CategoryDataPoint point;
        point.category = it.key();
        point.value = it.value();
        point.color = s_defaultColors[colorIndex % s_defaultColors.size()];
        data.append(point);
        colorIndex++;
    }
    
    // 计算百分比
    calculatePercentages(data);
    
    // 按值排序
    std::sort(data.begin(), data.end(), [](const CategoryDataPoint& a, const CategoryDataPoint& b) {
        return a.value > b.value;
    });
    
    return data;
}

QVector<CategoryDataPoint> StatisticsModel::getSeverityDistribution(
    const QDateTime& start, const QDateTime& end)
{
    QVector<CategoryDataPoint> data;
    
    if (!m_repo) {
        return data;
    }
    
    InspectionFilter filter;
    filter.startTime = start;
    filter.endTime = end;
    
    QVector<InspectionRecord> records = m_repo->queryInspections(filter);
    
    QMap<QString, int> severityCount;
    for (const auto& record : records) {
        if (!record.severityLevel.isEmpty()) {
            severityCount[record.severityLevel]++;
        }
    }
    
    // 使用固定颜色
    QMap<QString, QColor> severityColors = {
        {"Critical", QColor("#c62828")},
        {"Major", QColor("#ef6c00")},
        {"Minor", QColor("#f9a825")},
        {"Low", QColor("#2e7d32")}
    };
    
    for (auto it = severityCount.begin(); it != severityCount.end(); ++it) {
        CategoryDataPoint point;
        point.category = it.key();
        point.value = it.value();
        point.color = severityColors.value(it.key(), QColor("#757575"));
        data.append(point);
    }
    
    calculatePercentages(data);
    
    return data;
}

QVector<CategoryDataPoint> StatisticsModel::getResultDistribution(
    const QDateTime& start, const QDateTime& end)
{
    QVector<CategoryDataPoint> data;
    
    if (!m_repo) {
        return data;
    }
    
    InspectionFilter filter;
    filter.startTime = start;
    filter.endTime = end;
    
    int total = m_repo->countInspections(filter);
    
    filter.result = "OK";
    int okCount = m_repo->countInspections(filter);
    
    filter.result = "NG";
    int ngCount = m_repo->countInspections(filter);
    
    if (okCount > 0) {
        CategoryDataPoint okPoint;
        okPoint.category = "OK";
        okPoint.value = okCount;
        okPoint.color = QColor("#43a047");
        okPoint.percentage = (total > 0) ? (okCount * 100.0 / total) : 0;
        data.append(okPoint);
    }
    
    if (ngCount > 0) {
        CategoryDataPoint ngPoint;
        ngPoint.category = "NG";
        ngPoint.value = ngCount;
        ngPoint.color = QColor("#e53935");
        ngPoint.percentage = (total > 0) ? (ngCount * 100.0 / total) : 0;
        data.append(ngPoint);
    }
    
    return data;
}

QVector<CategoryDataPoint> StatisticsModel::getHourlyDistribution(const QDate& date)
{
    QVector<CategoryDataPoint> data;
    
    if (!m_repo) {
        return data;
    }
    
    for (int hour = 0; hour < 24; ++hour) {
        QDateTime start(date, QTime(hour, 0, 0));
        QDateTime end(date, QTime(hour, 59, 59));
        
        InspectionFilter filter;
        filter.startTime = start;
        filter.endTime = end;
        
        int count = m_repo->countInspections(filter);
        
        CategoryDataPoint point;
        point.category = QString("%1:00").arg(hour, 2, 10, QChar('0'));
        point.value = count;
        point.color = s_defaultColors[hour % s_defaultColors.size()];
        data.append(point);
    }
    
    calculatePercentages(data);
    
    return data;
}

QMap<QString, QPair<double, double>> StatisticsModel::getTodayVsYesterday()
{
    QMap<QString, QPair<double, double>> comparison;
    
    QDate today = QDate::currentDate();
    QDate yesterday = today.addDays(-1);
    
    QDateTime todayStart(today, QTime(0, 0, 0));
    QDateTime todayEnd(today, QTime(23, 59, 59));
    QDateTime yesterdayStart(yesterday, QTime(0, 0, 0));
    QDateTime yesterdayEnd(yesterday, QTime(23, 59, 59));
    
    // 今日数据
    InspectionFilter todayFilter;
    todayFilter.startTime = todayStart;
    todayFilter.endTime = todayEnd;
    
    int todayTotal = m_repo ? m_repo->countInspections(todayFilter) : 0;
    todayFilter.result = "OK";
    int todayOk = m_repo ? m_repo->countInspections(todayFilter) : 0;
    
    // 昨日数据
    InspectionFilter yesterdayFilter;
    yesterdayFilter.startTime = yesterdayStart;
    yesterdayFilter.endTime = yesterdayEnd;
    
    int yesterdayTotal = m_repo ? m_repo->countInspections(yesterdayFilter) : 0;
    yesterdayFilter.result = "OK";
    int yesterdayOk = m_repo ? m_repo->countInspections(yesterdayFilter) : 0;
    
    comparison["inspections"] = {todayTotal, yesterdayTotal};
    comparison["passRate"] = {
        todayTotal > 0 ? (todayOk * 100.0 / todayTotal) : 0,
        yesterdayTotal > 0 ? (yesterdayOk * 100.0 / yesterdayTotal) : 0
    };
    
    return comparison;
}

QMap<QString, QPair<double, double>> StatisticsModel::getThisWeekVsLastWeek()
{
    QMap<QString, QPair<double, double>> comparison;
    
    QDate today = QDate::currentDate();
    int dayOfWeek = today.dayOfWeek();
    
    QDate thisWeekStart = today.addDays(-dayOfWeek + 1);
    QDate lastWeekStart = thisWeekStart.addDays(-7);
    QDate lastWeekEnd = thisWeekStart.addDays(-1);
    
    // 本周数据
    InspectionFilter thisWeekFilter;
    thisWeekFilter.startTime = QDateTime(thisWeekStart, QTime(0, 0, 0));
    thisWeekFilter.endTime = QDateTime(today, QTime(23, 59, 59));
    
    int thisWeekTotal = m_repo ? m_repo->countInspections(thisWeekFilter) : 0;
    thisWeekFilter.result = "OK";
    int thisWeekOk = m_repo ? m_repo->countInspections(thisWeekFilter) : 0;
    
    // 上周数据
    InspectionFilter lastWeekFilter;
    lastWeekFilter.startTime = QDateTime(lastWeekStart, QTime(0, 0, 0));
    lastWeekFilter.endTime = QDateTime(lastWeekEnd, QTime(23, 59, 59));
    
    int lastWeekTotal = m_repo ? m_repo->countInspections(lastWeekFilter) : 0;
    lastWeekFilter.result = "OK";
    int lastWeekOk = m_repo ? m_repo->countInspections(lastWeekFilter) : 0;
    
    comparison["inspections"] = {thisWeekTotal, lastWeekTotal};
    comparison["passRate"] = {
        thisWeekTotal > 0 ? (thisWeekOk * 100.0 / thisWeekTotal) : 0,
        lastWeekTotal > 0 ? (lastWeekOk * 100.0 / lastWeekTotal) : 0
    };
    
    return comparison;
}

QMap<QString, QPair<double, double>> StatisticsModel::getThisMonthVsLastMonth()
{
    QMap<QString, QPair<double, double>> comparison;
    
    QDate today = QDate::currentDate();
    QDate thisMonthStart(today.year(), today.month(), 1);
    QDate lastMonthEnd = thisMonthStart.addDays(-1);
    QDate lastMonthStart(lastMonthEnd.year(), lastMonthEnd.month(), 1);
    
    // 本月数据
    InspectionFilter thisMonthFilter;
    thisMonthFilter.startTime = QDateTime(thisMonthStart, QTime(0, 0, 0));
    thisMonthFilter.endTime = QDateTime(today, QTime(23, 59, 59));
    
    int thisMonthTotal = m_repo ? m_repo->countInspections(thisMonthFilter) : 0;
    thisMonthFilter.result = "OK";
    int thisMonthOk = m_repo ? m_repo->countInspections(thisMonthFilter) : 0;
    
    // 上月数据
    InspectionFilter lastMonthFilter;
    lastMonthFilter.startTime = QDateTime(lastMonthStart, QTime(0, 0, 0));
    lastMonthFilter.endTime = QDateTime(lastMonthEnd, QTime(23, 59, 59));
    
    int lastMonthTotal = m_repo ? m_repo->countInspections(lastMonthFilter) : 0;
    lastMonthFilter.result = "OK";
    int lastMonthOk = m_repo ? m_repo->countInspections(lastMonthFilter) : 0;
    
    comparison["inspections"] = {thisMonthTotal, lastMonthTotal};
    comparison["passRate"] = {
        thisMonthTotal > 0 ? (thisMonthOk * 100.0 / thisMonthTotal) : 0,
        lastMonthTotal > 0 ? (lastMonthOk * 100.0 / lastMonthTotal) : 0
    };
    
    return comparison;
}

void StatisticsModel::addRealtimePoint(const QString& seriesName, const TimeSeriesPoint& point)
{
    m_realtimeData[seriesName].append(point);
    trimRealtimeData();
    emit realtimeDataUpdated(seriesName);
}

QVector<TimeSeriesPoint> StatisticsModel::getRealtimeSeries(const QString& seriesName) const
{
    return m_realtimeData.value(seriesName);
}

void StatisticsModel::clearRealtimeData(const QString& seriesName)
{
    if (seriesName.isEmpty()) {
        m_realtimeData.clear();
    } else {
        m_realtimeData.remove(seriesName);
    }
}

void StatisticsModel::setRealtimeRetention(int seconds)
{
    m_realtimeRetentionSeconds = seconds;
}

QVariantMap StatisticsModel::getTodaySummary()
{
    QDate today = QDate::currentDate();
    return getSummary(QDateTime(today, QTime(0, 0, 0)), 
                      QDateTime(today, QTime(23, 59, 59)));
}

QVariantMap StatisticsModel::getSummary(const QDateTime& start, const QDateTime& end)
{
    QVariantMap summary;
    
    if (!m_repo) {
        return summary;
    }
    
    InspectionFilter filter;
    filter.startTime = start;
    filter.endTime = end;
    
    int total = m_repo->countInspections(filter);
    filter.result = "OK";
    int okCount = m_repo->countInspections(filter);
    filter.result = "NG";
    int ngCount = m_repo->countInspections(filter);
    
    summary["totalInspections"] = total;
    summary["passCount"] = okCount;
    summary["failCount"] = ngCount;
    summary["passRate"] = total > 0 ? (okCount * 100.0 / total) : 0.0;
    
    // 缺陷统计
    filter.result.clear();
    QVector<InspectionRecord> records = m_repo->queryInspections(filter);
    
    int totalDefects = 0;
    double totalCycleTime = 0.0;
    
    for (const auto& record : records) {
        totalDefects += record.defectCount;
        totalCycleTime += record.cycleTimeMs;
    }
    
    summary["totalDefects"] = totalDefects;
    summary["avgCycleTime"] = total > 0 ? (totalCycleTime / total) : 0.0;
    summary["avgDefectsPerNG"] = ngCount > 0 ? (static_cast<double>(totalDefects) / ngCount) : 0.0;
    
    return summary;
}

double StatisticsModel::calculateTrend(const QVector<TimeSeriesPoint>& points)
{
    if (points.size() < 2) {
        return 0.0;
    }
    
    // 简单线性回归计算斜率
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    int n = points.size();
    
    for (int i = 0; i < n; ++i) {
        sumX += i;
        sumY += points[i].value;
        sumXY += i * points[i].value;
        sumX2 += i * i;
    }
    
    double denominator = n * sumX2 - sumX * sumX;
    if (std::abs(denominator) < 1e-10) {
        return 0.0;
    }
    
    return (n * sumXY - sumX * sumY) / denominator;
}

QString StatisticsModel::getTrendDescription(double trend)
{
    if (trend > 0.5) {
        return tr("显著上升");
    } else if (trend > 0.1) {
        return tr("轻微上升");
    } else if (trend < -0.5) {
        return tr("显著下降");
    } else if (trend < -0.1) {
        return tr("轻微下降");
    }
    return tr("基本稳定");
}

QVector<QDateTime> StatisticsModel::generateTimeScale(const QDateTime& start, 
                                                       const QDateTime& end,
                                                       TimeGranularity granularity)
{
    QVector<QDateTime> scale;
    
    if (!start.isValid() || !end.isValid() || start >= end) {
        return scale;
    }
    
    QDateTime current = start;
    
    while (current <= end) {
        scale.append(current);
        
        switch (granularity) {
            case TimeGranularity::Hour:
                current = current.addSecs(3600);
                break;
            case TimeGranularity::Day:
                current = current.addDays(1);
                break;
            case TimeGranularity::Week:
                current = current.addDays(7);
                break;
            case TimeGranularity::Month:
                current = current.addMonths(1);
                break;
        }
    }
    
    // 确保包含结束时间
    if (scale.isEmpty() || scale.last() < end) {
        scale.append(end);
    }
    
    return scale;
}

QString StatisticsModel::formatTimeLabel(const QDateTime& dt, TimeGranularity granularity) const
{
    switch (granularity) {
        case TimeGranularity::Hour:
            return dt.toString("HH:00");
        case TimeGranularity::Day:
            return dt.toString("MM-dd");
        case TimeGranularity::Week:
            return tr("第%1周").arg(dt.date().weekNumber());
        case TimeGranularity::Month:
            return dt.toString("yyyy-MM");
    }
    return dt.toString();
}

QDateTime StatisticsModel::truncateTime(const QDateTime& dt, TimeGranularity granularity) const
{
    QDate date = dt.date();
    
    switch (granularity) {
        case TimeGranularity::Hour:
            return QDateTime(date, QTime(dt.time().hour(), 0, 0));
        case TimeGranularity::Day:
            return QDateTime(date, QTime(0, 0, 0));
        case TimeGranularity::Week: {
            int dayOfWeek = date.dayOfWeek();
            return QDateTime(date.addDays(-dayOfWeek + 1), QTime(0, 0, 0));
        }
        case TimeGranularity::Month:
            return QDateTime(QDate(date.year(), date.month(), 1), QTime(0, 0, 0));
    }
    return dt;
}

void StatisticsModel::calculatePercentages(QVector<CategoryDataPoint>& data) const
{
    double total = 0.0;
    for (const auto& point : data) {
        total += point.value;
    }
    
    if (total > 0) {
        for (auto& point : data) {
            point.percentage = point.value / total * 100.0;
        }
    }
}

void StatisticsModel::trimRealtimeData()
{
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-m_realtimeRetentionSeconds);
    
    for (auto& series : m_realtimeData) {
        while (!series.isEmpty() && series.first().timestamp < cutoff) {
            series.removeFirst();
        }
    }
}

// ==================== StatisticsTableModel ====================

StatisticsTableModel::StatisticsTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

void StatisticsTableModel::setData(const QVector<CategoryDataPoint>& data)
{
    beginResetModel();
    m_data = data;
    endResetModel();
}

void StatisticsTableModel::clear()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
}

CategoryDataPoint StatisticsTableModel::dataAt(int row) const
{
    if (row >= 0 && row < m_data.size()) {
        return m_data.at(row);
    }
    return CategoryDataPoint();
}

int StatisticsTableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_data.size();
}

int StatisticsTableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return ColCount;
}

QVariant StatisticsTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.size()) {
        return QVariant();
    }
    
    const auto& point = m_data.at(index.row());
    
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case ColCategory:
                return point.category;
            case ColValue:
                return static_cast<int>(point.value);
            case ColPercentage:
                return QString("%1%").arg(point.percentage, 0, 'f', 1);
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() != ColCategory) {
            return Qt::AlignCenter;
        }
        return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (role == Qt::ForegroundRole) {
        if (index.column() == ColCategory && point.color.isValid()) {
            return QBrush(point.color.darker(120));
        }
    } else if (role == Qt::BackgroundRole) {
        if (index.column() == ColCategory && point.color.isValid()) {
            return QBrush(point.color.lighter(180));
        }
    } else if (role == Qt::DecorationRole) {
        if (index.column() == ColCategory && point.color.isValid()) {
            return point.color;
        }
    }
    
    return QVariant();
}

QVariant StatisticsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }
    
    switch (section) {
        case ColCategory:   return tr("类别");
        case ColValue:      return tr("数量");
        case ColPercentage: return tr("占比");
    }
    
    return QVariant();
}
