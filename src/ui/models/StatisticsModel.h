/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * StatisticsModel.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：统计数据模型定义
 * 描述：用于图表展示的统计数据模型，支持时间序列、分类统计、
 *       趋势分析等多种数据格式
 *
 * 当前版本：1.0
 */

#ifndef STATISTICSMODEL_H
#define STATISTICSMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QDateTime>
#include <QVector>
#include <QMap>
#include <QVariant>
#include <QPair>
#include <QColor>
#include "ui_global.h"

/**
 * @brief 时间粒度
 */
enum class TimeGranularity {
    Hour,       // 小时
    Day,        // 天
    Week,       // 周
    Month       // 月
};

/**
 * @brief 时间序列数据点
 */
struct UI_LIBRARY TimeSeriesPoint {
    QDateTime timestamp;    // 时间点
    double value;           // 值
    QString label;          // 标签（可选）
    
    TimeSeriesPoint() : value(0.0) {}
    TimeSeriesPoint(const QDateTime& t, double v, const QString& l = QString())
        : timestamp(t), value(v), label(l) {}
};

/**
 * @brief 分类数据点
 */
struct UI_LIBRARY CategoryDataPoint {
    QString category;       // 类别名称
    double value;           // 值
    QColor color;           // 显示颜色
    double percentage;      // 百分比
    
    CategoryDataPoint() : value(0.0), percentage(0.0) {}
    CategoryDataPoint(const QString& c, double v, const QColor& col = QColor())
        : category(c), value(v), color(col), percentage(0.0) {}
};

/**
 * @brief 趋势数据
 */
struct UI_LIBRARY TrendData {
    QVector<TimeSeriesPoint> points;    // 数据点
    double min;                          // 最小值
    double max;                          // 最大值
    double average;                      // 平均值
    double trend;                        // 趋势斜率（正=上升，负=下降）
    QString trendDescription;            // 趋势描述
    
    TrendData() : min(0), max(0), average(0), trend(0) {}
};

/**
 * @brief 统计数据模型
 *
 * 提供多种统计数据格式，用于图表展示：
 * - 时间序列（折线图、面积图）
 * - 分类统计（饼图、柱状图）
 * - 趋势分析
 * - 对比数据
 *
 * 用法：
 * @code
 * StatisticsModel model;
 * model.setRepository(defectRepo);
 *
 * // 获取检测趋势
 * TrendData trend = model.getInspectionTrend(
 *     QDateTime::currentDateTime().addDays(-7),
 *     QDateTime::currentDateTime(),
 *     TimeGranularity::Day
 * );
 *
 * // 获取缺陷类型分布
 * QVector<CategoryDataPoint> dist = model.getDefectTypeDistribution(filter);
 *
 * // 设置到图表
 * chart->setSeries(trend.points);
 * @endcode
 */
class UI_LIBRARY StatisticsModel : public QObject {
    Q_OBJECT

public:
    explicit StatisticsModel(QObject* parent = nullptr);
    ~StatisticsModel() override;

    // ======================== 数据源 ========================

    /**
     * @brief 设置数据仓库
     */
    void setRepository(class DefectRepository* repo);

    // ======================== 时间序列数据 ========================

    /**
     * @brief 获取检测数量趋势
     * @param start 开始时间
     * @param end 结束时间
     * @param granularity 时间粒度
     * @return 趋势数据
     */
    TrendData getInspectionTrend(const QDateTime& start, const QDateTime& end,
                                  TimeGranularity granularity = TimeGranularity::Day);

    /**
     * @brief 获取合格率趋势
     */
    TrendData getPassRateTrend(const QDateTime& start, const QDateTime& end,
                                TimeGranularity granularity = TimeGranularity::Day);

    /**
     * @brief 获取缺陷数量趋势
     */
    TrendData getDefectCountTrend(const QDateTime& start, const QDateTime& end,
                                   TimeGranularity granularity = TimeGranularity::Day);

    /**
     * @brief 获取检测耗时趋势
     */
    TrendData getCycleTimeTrend(const QDateTime& start, const QDateTime& end,
                                 TimeGranularity granularity = TimeGranularity::Day);

    // ======================== 分类统计 ========================

    /**
     * @brief 获取缺陷类型分布
     * @param start 开始时间（可选）
     * @param end 结束时间（可选）
     * @return 分类数据
     */
    QVector<CategoryDataPoint> getDefectTypeDistribution(
        const QDateTime& start = QDateTime(),
        const QDateTime& end = QDateTime());

    /**
     * @brief 获取严重度分布
     */
    QVector<CategoryDataPoint> getSeverityDistribution(
        const QDateTime& start = QDateTime(),
        const QDateTime& end = QDateTime());

    /**
     * @brief 获取结果分布 (OK/NG)
     */
    QVector<CategoryDataPoint> getResultDistribution(
        const QDateTime& start = QDateTime(),
        const QDateTime& end = QDateTime());

    /**
     * @brief 获取按小时的检测分布
     */
    QVector<CategoryDataPoint> getHourlyDistribution(const QDate& date);

    // ======================== 对比数据 ========================

    /**
     * @brief 获取今日与昨日对比
     */
    QMap<QString, QPair<double, double>> getTodayVsYesterday();

    /**
     * @brief 获取本周与上周对比
     */
    QMap<QString, QPair<double, double>> getThisWeekVsLastWeek();

    /**
     * @brief 获取本月与上月对比
     */
    QMap<QString, QPair<double, double>> getThisMonthVsLastMonth();

    // ======================== 实时数据 ========================

    /**
     * @brief 添加实时数据点
     * @param seriesName 系列名称
     * @param point 数据点
     */
    void addRealtimePoint(const QString& seriesName, const TimeSeriesPoint& point);

    /**
     * @brief 获取实时数据系列
     */
    QVector<TimeSeriesPoint> getRealtimeSeries(const QString& seriesName) const;

    /**
     * @brief 清除实时数据
     */
    void clearRealtimeData(const QString& seriesName = QString());

    /**
     * @brief 设置实时数据保留时长（秒）
     */
    void setRealtimeRetention(int seconds);

    // ======================== 汇总统计 ========================

    /**
     * @brief 获取今日汇总
     */
    QVariantMap getTodaySummary();

    /**
     * @brief 获取指定时间范围的汇总
     */
    QVariantMap getSummary(const QDateTime& start, const QDateTime& end);

    // ======================== 辅助方法 ========================

    /**
     * @brief 计算趋势
     */
    static double calculateTrend(const QVector<TimeSeriesPoint>& points);

    /**
     * @brief 获取趋势描述
     */
    static QString getTrendDescription(double trend);

    /**
     * @brief 生成时间刻度
     */
    static QVector<QDateTime> generateTimeScale(const QDateTime& start, 
                                                 const QDateTime& end,
                                                 TimeGranularity granularity);

signals:
    /**
     * @brief 数据更新
     */
    void dataUpdated();

    /**
     * @brief 实时数据更新
     */
    void realtimeDataUpdated(const QString& seriesName);

    /**
     * @brief 错误发生
     */
    void errorOccurred(const QString& error);

private:
    // 内部方法
    QString formatTimeLabel(const QDateTime& dt, TimeGranularity granularity) const;
    QDateTime truncateTime(const QDateTime& dt, TimeGranularity granularity) const;
    void calculatePercentages(QVector<CategoryDataPoint>& data) const;
    void trimRealtimeData();

    class DefectRepository* m_repo = nullptr;
    
    // 实时数据缓存
    QMap<QString, QVector<TimeSeriesPoint>> m_realtimeData;
    int m_realtimeRetentionSeconds = 3600;  // 默认保留1小时
    
    // 默认颜色
    static const QVector<QColor> s_defaultColors;
};

/**
 * @brief 统计表格模型
 *
 * 将统计数据以表格形式展示，用于 QTableView
 */
class UI_LIBRARY StatisticsTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        ColCategory = 0,    // 类别
        ColValue,           // 值
        ColPercentage,      // 百分比
        ColCount
    };

    explicit StatisticsTableModel(QObject* parent = nullptr);

    /**
     * @brief 设置分类数据
     */
    void setData(const QVector<CategoryDataPoint>& data);

    /**
     * @brief 清空数据
     */
    void clear();

    /**
     * @brief 获取指定行的数据
     */
    CategoryDataPoint dataAt(int row) const;

    // QAbstractTableModel 接口
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QVector<CategoryDataPoint> m_data;
};

// 注册元类型
Q_DECLARE_METATYPE(TimeSeriesPoint)
Q_DECLARE_METATYPE(CategoryDataPoint)
Q_DECLARE_METATYPE(TrendData)
Q_DECLARE_METATYPE(TimeGranularity)

#endif // STATISTICSMODEL_H
