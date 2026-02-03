/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * HistoryTableModel.cpp
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：历史表格模型实现
 *
 * 当前版本：2.0
 */

#include "HistoryTableModel.h"
#include "common/Logger.h"
#include <QColor>
#include <QBrush>
#include <QFile>
#include <QTextStream>
#include <QStringConverter>
#include <algorithm>

HistoryTableModel::HistoryTableModel(QObject *parent)
    : QAbstractTableModel{parent}
    , m_columnVisible(ColCount, true)
{
    // 默认隐藏部分列
    m_columnVisible[ColOperator] = false;
    m_columnVisible[ColBatchNo] = false;
    m_columnVisible[ColProductId] = false;
    
    LOG_DEBUG("HistoryTableModel created");
}

HistoryTableModel::~HistoryTableModel()
{
    LOG_DEBUG("HistoryTableModel destroyed");
}

void HistoryTableModel::setRepository(DefectRepository* repo)
{
    m_repo = repo;
    LOG_INFO("HistoryTableModel::setRepository - Repository set");
}

void HistoryTableModel::refresh()
{
    refresh(m_filter);
}

void HistoryTableModel::refresh(const InspectionFilter& filter)
{
    if (!m_repo) {
        LOG_WARN("HistoryTableModel::refresh - No repository set");
        emit errorOccurred(tr("未设置数据仓库"));
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    m_filter = filter;
    m_isLoading = true;
    emit loadingStateChanged(true);
    
    // 获取总记录数
    int totalCount = m_repo->countInspections(filter);
    m_pageInfo.update(totalCount);
    
    LOG_INFO("HistoryTableModel::refresh - Total records: {}, pages: {}", 
             totalCount, m_pageInfo.totalPages);
    
    // 加载当前页
    loadPage(m_pageInfo.currentPage);
    
    // 更新统计
    updateStatistics();
    
    m_isLoading = false;
    emit loadingStateChanged(false);
    emit dataLoaded(totalCount);
    emit pageChanged(m_pageInfo.currentPage, m_pageInfo.totalPages);
}

void HistoryTableModel::clear()
{
    QMutexLocker locker(&m_mutex);
    
    beginResetModel();
    m_records.clear();
    m_pageInfo = PageInfo();
    m_statistics = HistoryStatistics();
    endResetModel();
    
    emit pageChanged(0, 0);
    LOG_DEBUG("HistoryTableModel::clear - Cleared all data");
}

void HistoryTableModel::setFilter(const InspectionFilter& filter)
{
    m_filter = filter;
    m_pageInfo.currentPage = 1;  // 重置到第一页
}

void HistoryTableModel::setPageSize(int size)
{
    if (size > 0 && size != m_pageInfo.pageSize) {
        m_pageInfo.pageSize = size;
        m_pageInfo.currentPage = 1;
        LOG_DEBUG("HistoryTableModel::setPageSize - Page size set to {}", size);
    }
}

bool HistoryTableModel::gotoPage(int page)
{
    if (page < 1 || page > m_pageInfo.totalPages) {
        return false;
    }
    
    if (page == m_pageInfo.currentPage) {
        return true;
    }
    
    m_pageInfo.currentPage = page;
    
    QMutexLocker locker(&m_mutex);
    loadPage(page);
    
    emit pageChanged(m_pageInfo.currentPage, m_pageInfo.totalPages);
    return true;
}

bool HistoryTableModel::nextPage()
{
    return gotoPage(m_pageInfo.currentPage + 1);
}

bool HistoryTableModel::prevPage()
{
    return gotoPage(m_pageInfo.currentPage - 1);
}

bool HistoryTableModel::firstPage()
{
    return gotoPage(1);
}

bool HistoryTableModel::lastPage()
{
    return gotoPage(m_pageInfo.totalPages);
}

InspectionRecord HistoryTableModel::recordAt(int row) const
{
    QMutexLocker locker(&m_mutex);
    if (row >= 0 && row < m_records.size()) {
        return m_records.at(row);
    }
    return InspectionRecord();
}

qint64 HistoryTableModel::idAt(int row) const
{
    QMutexLocker locker(&m_mutex);
    if (row >= 0 && row < m_records.size()) {
        return m_records.at(row).id;
    }
    return -1;
}

QVector<InspectionRecord> HistoryTableModel::selectedRecords(const QModelIndexList& indexes) const
{
    QVector<InspectionRecord> result;
    QSet<int> rows;
    
    for (const auto& index : indexes) {
        rows.insert(index.row());
    }
    
    QMutexLocker locker(&m_mutex);
    for (int row : rows) {
        if (row >= 0 && row < m_records.size()) {
            result.append(m_records.at(row));
        }
    }
    
    return result;
}

HistoryStatistics HistoryTableModel::statistics() const
{
    return m_statistics;
}

HistoryStatistics HistoryTableModel::statisticsForRange(const QDateTime& start, const QDateTime& end) const
{
    HistoryStatistics stats;
    
    if (!m_repo) {
        return stats;
    }
    
    InspectionFilter rangeFilter = m_filter;
    rangeFilter.startTime = start;
    rangeFilter.endTime = end;
    
    // 这里可以调用 Repository 的统计方法
    // 简化实现：查询所有数据并在内存中统计
    QVector<InspectionRecord> records = m_repo->queryInspections(rangeFilter);
    
    stats.totalInspections = records.size();
    
    double totalCycleTime = 0.0;
    stats.minCycleTimeMs = std::numeric_limits<double>::max();
    stats.maxCycleTimeMs = 0.0;
    
    for (const auto& record : records) {
        if (record.result == "OK") {
            stats.passCount++;
        } else if (record.result == "NG") {
            stats.failCount++;
            stats.totalDefects += record.defectCount;
            stats.severityStats[record.severityLevel]++;
        }
        
        totalCycleTime += record.cycleTimeMs;
        stats.minCycleTimeMs = std::min(stats.minCycleTimeMs, static_cast<double>(record.cycleTimeMs));
        stats.maxCycleTimeMs = std::max(stats.maxCycleTimeMs, static_cast<double>(record.cycleTimeMs));
        
        if (!stats.firstInspectTime.isValid() || record.inspectTime < stats.firstInspectTime) {
            stats.firstInspectTime = record.inspectTime;
        }
        if (!stats.lastInspectTime.isValid() || record.inspectTime > stats.lastInspectTime) {
            stats.lastInspectTime = record.inspectTime;
        }
    }
    
    if (stats.totalInspections > 0) {
        stats.passRate = static_cast<double>(stats.passCount) / stats.totalInspections * 100.0;
        stats.avgCycleTimeMs = totalCycleTime / stats.totalInspections;
    }
    
    if (stats.failCount > 0) {
        stats.avgDefectsPerNG = static_cast<double>(stats.totalDefects) / stats.failCount;
    }
    
    if (stats.minCycleTimeMs == std::numeric_limits<double>::max()) {
        stats.minCycleTimeMs = 0.0;
    }
    
    return stats;
}

bool HistoryTableModel::deleteRecord(qint64 id)
{
    if (!m_repo) {
        return false;
    }
    
    bool success = m_repo->deleteInspection(id);
    if (success) {
        LOG_INFO("HistoryTableModel::deleteRecord - Deleted record id={}", id);
        refresh();  // 刷新数据
    }
    return success;
}

int HistoryTableModel::deleteRecords(const QVector<qint64>& ids)
{
    if (!m_repo || ids.isEmpty()) {
        return 0;
    }
    
    int deletedCount = 0;
    for (qint64 id : ids) {
        if (m_repo->deleteInspection(id)) {
            deletedCount++;
        }
    }
    
    if (deletedCount > 0) {
        LOG_INFO("HistoryTableModel::deleteRecords - Deleted {} records", deletedCount);
        refresh();
    }
    
    return deletedCount;
}

int HistoryTableModel::deleteAllFiltered()
{
    if (!m_repo) {
        return 0;
    }
    
    // 获取所有过滤后的ID
    QVector<InspectionRecord> allRecords = m_repo->queryInspections(m_filter);
    QVector<qint64> ids;
    for (const auto& record : allRecords) {
        ids.append(record.id);
    }
    
    return deleteRecords(ids);
}

bool HistoryTableModel::exportCurrentPageToCsv(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        LOG_ERROR("HistoryTableModel::exportCurrentPageToCsv - Failed to open file: {}", filePath);
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // 写入BOM（用于Excel正确识别UTF-8）
    out << "\xEF\xBB\xBF";
    
    // 写入表头
    out << exportHeaders().join(",") << "\n";
    
    // 写入数据
    QMutexLocker locker(&m_mutex);
    for (const auto& record : m_records) {
        out << exportRowData(record).join(",") << "\n";
    }
    
    file.close();
    LOG_INFO("HistoryTableModel::exportCurrentPageToCsv - Exported {} records to {}", 
             m_records.size(), filePath);
    return true;
}

bool HistoryTableModel::exportAllToCsv(const QString& filePath) const
{
    if (!m_repo) {
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        LOG_ERROR("HistoryTableModel::exportAllToCsv - Failed to open file: {}", filePath);
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // 写入BOM
    out << "\xEF\xBB\xBF";
    
    // 写入表头
    out << exportHeaders().join(",") << "\n";
    
    // 查询所有数据并写入
    QVector<InspectionRecord> allRecords = m_repo->queryInspections(m_filter);
    for (const auto& record : allRecords) {
        out << exportRowData(record).join(",") << "\n";
    }
    
    file.close();
    LOG_INFO("HistoryTableModel::exportAllToCsv - Exported {} records to {}", 
             allRecords.size(), filePath);
    return true;
}

QStringList HistoryTableModel::exportHeaders() const
{
    return {
        tr("ID"),
        tr("检测时间"),
        tr("结果"),
        tr("缺陷数"),
        tr("严重度"),
        tr("耗时(ms)"),
        tr("图像路径"),
        tr("操作员"),
        tr("批次号"),
        tr("产品ID")
    };
}

QStringList HistoryTableModel::exportRowData(const InspectionRecord& record) const
{
    auto escapeField = [](const QString& field) -> QString {
        if (field.contains(',') || field.contains('"') || field.contains('\n')) {
            return "\"" + QString(field).replace("\"", "\"\"") + "\"";
        }
        return field;
    };
    
    return {
        QString::number(record.id),
        record.inspectTime.toString("yyyy-MM-dd hh:mm:ss"),
        record.result,
        QString::number(record.defectCount),
        record.severityLevel,
        QString::number(record.cycleTimeMs),
        escapeField(record.imagePath),
        escapeField(record.operatorName),
        escapeField(record.batchNo),
        escapeField(record.productCode)
    };
}

int HistoryTableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    QMutexLocker locker(&m_mutex);
    return m_records.size();
}

int HistoryTableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return ColCount;
}

QVariant HistoryTableModel::data(const QModelIndex& index, int role) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!index.isValid() || index.row() >= m_records.size()) {
        return QVariant();
    }

    const auto& record = m_records.at(index.row());

    switch (role) {
        case Qt::DisplayRole: {
            switch (index.column()) {
                case ColId:
                    return record.id;
                case ColTime:
                    return record.inspectTime.toString("yyyy-MM-dd hh:mm:ss");
                case ColResult:
                    return record.result;
                case ColDefectCount:
                    return record.defectCount;
                case ColSeverity:
                    return record.severityLevel;
                case ColCycleTime:
                    return formatCycleTime(record.cycleTimeMs);
                case ColImagePath:
                    return record.imagePath;
                case ColOperator:
                    return record.operatorName;
                case ColBatchNo:
                    return record.batchNo;
                case ColProductId:
                    return record.productCode;
            }
            break;
        }
        
        case Qt::ForegroundRole: {
            if (index.column() == ColResult) {
                return QBrush(resultColor(record.result));
            }
            if (index.column() == ColSeverity && !record.severityLevel.isEmpty()) {
                return QBrush(severityColor(record.severityLevel));
            }
            break;
        }
        
        case Qt::BackgroundRole: {
            if (record.result == "NG") {
                return QBrush(QColor(255, 235, 238));  // 浅红背景
            }
            break;
        }
        
        case Qt::TextAlignmentRole: {
            switch (index.column()) {
                case ColId:
                case ColDefectCount:
                case ColCycleTime:
                    return Qt::AlignCenter;
                case ColResult:
                case ColSeverity:
                    return Qt::AlignCenter;
                default:
                    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
            }
            break;
        }
        
        case Qt::ToolTipRole: {
            if (index.column() == ColImagePath && !record.imagePath.isEmpty()) {
                return record.imagePath;
            }
            break;
        }
        
        case RecordIdRole:
            return record.id;
            
        case FullRecordRole:
            return QVariant::fromValue(record);
            
        case ImagePathRole:
            return record.imagePath;
            
        case SortRole:
            return sortValue(record, index.column());
    }

    return QVariant();
}

QVariant HistoryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }
    
    if (role == Qt::DisplayRole) {
        switch (section) {
            case ColId:          return tr("ID");
            case ColTime:        return tr("检测时间");
            case ColResult:      return tr("结果");
            case ColDefectCount: return tr("缺陷数");
            case ColSeverity:    return tr("严重度");
            case ColCycleTime:   return tr("耗时");
            case ColImagePath:   return tr("图像路径");
            case ColOperator:    return tr("操作员");
            case ColBatchNo:     return tr("批次号");
            case ColProductId:   return tr("产品ID");
        }
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    return QVariant();
}

Qt::ItemFlags HistoryTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void HistoryTableModel::sort(int column, Qt::SortOrder order)
{
    setSortColumn(column, order);
}

void HistoryTableModel::setSortColumn(int column, Qt::SortOrder order)
{
    if (column < 0 || column >= ColCount) {
        return;
    }
    
    m_sortColumn = column;
    m_sortOrder = order;
    
    LOG_DEBUG("HistoryTableModel::setSortColumn - column={}, order={}", 
              column, order == Qt::AscendingOrder ? "ASC" : "DESC");
    
    // 重新排序当前数据
    beginResetModel();
    
    std::sort(m_records.begin(), m_records.end(), 
              [this, column, order](const InspectionRecord& a, const InspectionRecord& b) {
        QVariant va = sortValue(a, column);
        QVariant vb = sortValue(b, column);
        
        bool less = false;
        if (va.metaType().id() == QMetaType::Int || va.metaType().id() == QMetaType::LongLong) {
            less = va.toLongLong() < vb.toLongLong();
        } else if (va.metaType().id() == QMetaType::Double) {
            less = va.toDouble() < vb.toDouble();
        } else if (va.metaType().id() == QMetaType::QDateTime) {
            less = va.toDateTime() < vb.toDateTime();
        } else {
            less = va.toString() < vb.toString();
        }
        
        return (order == Qt::AscendingOrder) ? less : !less;
    });
    
    endResetModel();
}

void HistoryTableModel::loadPage(int page)
{
    if (!m_repo) {
        return;
    }
    
    beginResetModel();
    
    // 创建分页过滤器
    InspectionFilter pageFilter = m_filter;
    pageFilter.limit = m_pageInfo.pageSize;
    pageFilter.offset = (page - 1) * m_pageInfo.pageSize;
    
    // 添加排序
    switch (m_sortColumn) {
        case ColId:
            pageFilter.orderBy = "id";
            break;
        case ColTime:
            pageFilter.orderBy = "inspect_time";
            break;
        case ColResult:
            pageFilter.orderBy = "result";
            break;
        case ColDefectCount:
            pageFilter.orderBy = "defect_count";
            break;
        case ColCycleTime:
            pageFilter.orderBy = "cycle_time_ms";
            break;
        default:
            pageFilter.orderBy = "inspect_time";
            break;
    }
    pageFilter.ascending = (m_sortOrder == Qt::AscendingOrder);
    
    m_records = m_repo->queryInspections(pageFilter);
    
    endResetModel();
    
    LOG_DEBUG("HistoryTableModel::loadPage - Loaded page {}, {} records", 
              page, m_records.size());
}

void HistoryTableModel::updateStatistics()
{
    if (!m_repo) {
        return;
    }
    
    // 使用全部数据计算统计（可以优化为数据库聚合查询）
    m_statistics = statisticsForRange(m_filter.startTime, m_filter.endTime);
    
    emit statisticsUpdated(m_statistics);
}

QColor HistoryTableModel::resultColor(const QString& result) const
{
    if (result == "OK") {
        return QColor(46, 125, 50);   // 绿色
    } else if (result == "NG") {
        return QColor(198, 40, 40);   // 红色
    }
    return QColor(117, 117, 117);     // 灰色
}

QColor HistoryTableModel::severityColor(const QString& severity) const
{
    if (severity == "Critical") {
        return QColor(183, 28, 28);   // 深红
    } else if (severity == "Major") {
        return QColor(230, 81, 0);    // 橙色
    } else if (severity == "Minor") {
        return QColor(249, 168, 37);  // 黄色
    }
    return QColor(46, 125, 50);       // 绿色
}

QString HistoryTableModel::formatCycleTime(int ms) const
{
    if (ms < 1000) {
        return QString("%1 ms").arg(ms);
    } else {
        return QString("%1 s").arg(ms / 1000.0, 0, 'f', 2);
    }
}

QVariant HistoryTableModel::sortValue(const InspectionRecord& record, int column) const
{
    switch (column) {
        case ColId:
            return record.id;
        case ColTime:
            return record.inspectTime;
        case ColResult:
            return record.result;
        case ColDefectCount:
            return record.defectCount;
        case ColSeverity: {
            // 严重度排序值
            if (record.severityLevel == "Critical") return 4;
            if (record.severityLevel == "Major") return 3;
            if (record.severityLevel == "Minor") return 2;
            return 1;
        }
        case ColCycleTime:
            return record.cycleTimeMs;
        case ColImagePath:
            return record.imagePath;
        case ColOperator:
            return record.operatorName;
        case ColBatchNo:
            return record.batchNo;
        case ColProductId:
            return record.productCode;
        default:
            return QVariant();
    }
}
