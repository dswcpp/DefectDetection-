/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * HistoryTableModel.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：历史表格模型定义
 * 描述：Qt Model/View架构的历史记录数据模型，供QTableView显示检测历史
 *       支持分页加载、排序、导出、统计等功能
 *
 * 当前版本：2.0
 */

#ifndef HISTORYTABLEMODEL_H
#define HISTORYTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QDateTime>
#include <QMutex>
#include "ui_global.h"
#include "data/repositories/DefectRepository.h"

/**
 * @brief 分页信息
 */
struct UI_LIBRARY PageInfo {
    int currentPage = 1;        // 当前页码（从1开始）
    int pageSize = 50;          // 每页记录数
    int totalRecords = 0;       // 总记录数
    int totalPages = 0;         // 总页数
    
    void update(int total) {
        totalRecords = total;
        totalPages = (total + pageSize - 1) / pageSize;
        if (currentPage > totalPages && totalPages > 0) {
            currentPage = totalPages;
        }
    }
    
    int offset() const {
        return (currentPage - 1) * pageSize;
    }
    
    bool hasNextPage() const {
        return currentPage < totalPages;
    }
    
    bool hasPrevPage() const {
        return currentPage > 1;
    }
};

/**
 * @brief 历史统计信息
 */
struct UI_LIBRARY HistoryStatistics {
    int totalInspections = 0;       // 总检测数
    int passCount = 0;              // 合格数 (OK)
    int failCount = 0;              // 不合格数 (NG)
    double passRate = 0.0;          // 合格率
    int totalDefects = 0;           // 总缺陷数
    double avgDefectsPerNG = 0.0;   // 平均每个NG的缺陷数
    double avgCycleTimeMs = 0.0;    // 平均检测耗时
    double minCycleTimeMs = 0.0;    // 最短耗时
    double maxCycleTimeMs = 0.0;    // 最长耗时
    QDateTime firstInspectTime;     // 最早检测时间
    QDateTime lastInspectTime;      // 最新检测时间
    QMap<QString, int> defectTypeStats;  // 各类缺陷统计
    QMap<QString, int> severityStats;    // 各严重度统计
};

/**
 * @brief 历史记录表格模型
 *
 * 提供检测历史数据的展示，支持：
 * - 分页加载（适合大数据量）
 * - 列排序
 * - 数据过滤
 * - 统计分析
 * - 数据导出
 * - 批量操作
 *
 * 用法：
 * @code
 * HistoryTableModel model;
 * model.setRepository(defectRepo);
 * model.setPageSize(100);
 *
 * // 设置过滤条件
 * InspectionFilter filter;
 * filter.startTime = QDateTime::currentDateTime().addDays(-7);
 * filter.result = "NG";
 * model.setFilter(filter);
 *
 * // 刷新数据
 * model.refresh();
 *
 * // 分页导航
 * model.nextPage();
 * model.prevPage();
 * model.gotoPage(5);
 *
 * // 获取统计信息
 * HistoryStatistics stats = model.statistics();
 * @endcode
 */
class UI_LIBRARY HistoryTableModel : public QAbstractTableModel {
    Q_OBJECT
    Q_PROPERTY(int currentPage READ currentPage NOTIFY pageChanged)
    Q_PROPERTY(int totalPages READ totalPages NOTIFY pageChanged)
    Q_PROPERTY(int totalRecords READ totalRecords NOTIFY dataLoaded)

public:
    /**
     * @brief 列定义
     */
    enum Column {
        ColId = 0,          // ID
        ColTime,            // 检测时间
        ColResult,          // 结果 (OK/NG)
        ColDefectCount,     // 缺陷数量
        ColSeverity,        // 最高严重度
        ColCycleTime,       // 检测耗时
        ColImagePath,       // 图像路径
        ColOperator,        // 操作员
        ColBatchNo,         // 批次号
        ColProductId,       // 产品ID
        ColCount
    };
    Q_ENUM(Column)

    /**
     * @brief 自定义角色
     */
    enum CustomRole {
        RecordIdRole = Qt::UserRole,        // 记录ID
        FullRecordRole = Qt::UserRole + 1,  // 完整记录
        ImagePathRole = Qt::UserRole + 2,   // 图像路径
        DefectsRole = Qt::UserRole + 3,     // 缺陷列表
        SortRole = Qt::UserRole + 100       // 排序值
    };

    explicit HistoryTableModel(QObject *parent = nullptr);
    ~HistoryTableModel() override;

    // ======================== 数据源 ========================

    /**
     * @brief 设置数据仓库
     */
    void setRepository(DefectRepository* repo);

    // ======================== 数据刷新 ========================

    /**
     * @brief 刷新数据（使用当前过滤条件）
     */
    void refresh();

    /**
     * @brief 使用指定过滤条件刷新
     */
    void refresh(const InspectionFilter& filter);

    /**
     * @brief 清空数据
     */
    void clear();

    /**
     * @brief 设置过滤条件
     */
    void setFilter(const InspectionFilter& filter);

    /**
     * @brief 获取当前过滤条件
     */
    InspectionFilter filter() const { return m_filter; }

    // ======================== 分页控制 ========================

    /**
     * @brief 设置每页记录数
     */
    void setPageSize(int size);

    /**
     * @brief 获取分页信息
     */
    PageInfo pageInfo() const { return m_pageInfo; }

    /**
     * @brief 当前页码
     */
    int currentPage() const { return m_pageInfo.currentPage; }

    /**
     * @brief 总页数
     */
    int totalPages() const { return m_pageInfo.totalPages; }

    /**
     * @brief 总记录数
     */
    int totalRecords() const { return m_pageInfo.totalRecords; }

    /**
     * @brief 跳转到指定页
     */
    bool gotoPage(int page);

    /**
     * @brief 下一页
     */
    bool nextPage();

    /**
     * @brief 上一页
     */
    bool prevPage();

    /**
     * @brief 第一页
     */
    bool firstPage();

    /**
     * @brief 最后一页
     */
    bool lastPage();

    // ======================== 数据访问 ========================

    /**
     * @brief 获取指定行的记录
     */
    InspectionRecord recordAt(int row) const;

    /**
     * @brief 获取指定行的ID
     */
    qint64 idAt(int row) const;

    /**
     * @brief 获取所有当前页记录
     */
    QVector<InspectionRecord> currentPageRecords() const { return m_records; }

    /**
     * @brief 获取选中行的记录
     */
    QVector<InspectionRecord> selectedRecords(const QModelIndexList& indexes) const;

    // ======================== 统计分析 ========================

    /**
     * @brief 获取当前过滤条件下的统计信息
     */
    HistoryStatistics statistics() const;

    /**
     * @brief 计算指定时间范围的统计
     */
    HistoryStatistics statisticsForRange(const QDateTime& start, const QDateTime& end) const;

    // ======================== 批量操作 ========================

    /**
     * @brief 删除指定记录
     */
    bool deleteRecord(qint64 id);

    /**
     * @brief 批量删除记录
     */
    int deleteRecords(const QVector<qint64>& ids);

    /**
     * @brief 删除当前过滤条件下的所有记录
     */
    int deleteAllFiltered();

    // ======================== 导出功能 ========================

    /**
     * @brief 导出当前页数据为CSV
     */
    bool exportCurrentPageToCsv(const QString& filePath) const;

    /**
     * @brief 导出所有过滤数据为CSV
     */
    bool exportAllToCsv(const QString& filePath) const;

    /**
     * @brief 获取导出的表头
     */
    QStringList exportHeaders() const;

    /**
     * @brief 获取导出的行数据
     */
    QStringList exportRowData(const InspectionRecord& record) const;

    // ======================== QAbstractTableModel 接口 ========================

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // ======================== 排序支持 ========================

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    /**
     * @brief 设置排序列和顺序
     */
    void setSortColumn(int column, Qt::SortOrder order = Qt::AscendingOrder);

    /**
     * @brief 获取当前排序列
     */
    int sortColumn() const { return m_sortColumn; }

    /**
     * @brief 获取当前排序顺序
     */
    Qt::SortOrder sortOrder() const { return m_sortOrder; }

signals:
    /**
     * @brief 数据加载完成
     */
    void dataLoaded(int count);

    /**
     * @brief 页码改变
     */
    void pageChanged(int currentPage, int totalPages);

    /**
     * @brief 统计更新
     */
    void statisticsUpdated(const HistoryStatistics& stats);

    /**
     * @brief 加载中状态
     */
    void loadingStateChanged(bool isLoading);

    /**
     * @brief 错误发生
     */
    void errorOccurred(const QString& error);

private:
    // 内部方法
    void loadPage(int page);
    void updateStatistics();
    QColor resultColor(const QString& result) const;
    QColor severityColor(const QString& severity) const;
    QString formatCycleTime(int ms) const;
    QVariant sortValue(const InspectionRecord& record, int column) const;

    DefectRepository* m_repo = nullptr;
    QVector<InspectionRecord> m_records;
    InspectionFilter m_filter;
    PageInfo m_pageInfo;
    HistoryStatistics m_statistics;
    
    // 排序
    int m_sortColumn = ColTime;
    Qt::SortOrder m_sortOrder = Qt::DescendingOrder;
    
    // 线程安全 - 使用递归互斥量避免同一线程重入死锁
    mutable QRecursiveMutex m_mutex;
    bool m_isLoading = false;
    
    // 列可见性
    QVector<bool> m_columnVisible;
};

// 注册元类型
Q_DECLARE_METATYPE(PageInfo)
Q_DECLARE_METATYPE(HistoryStatistics)

#endif // HISTORYTABLEMODEL_H
