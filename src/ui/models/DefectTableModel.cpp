#include "DefectTableModel.h"
#include "common/Logger.h"
#include <QBrush>
#include <algorithm>
#include <numeric>

// 默认类别颜色
const QVector<QColor> DefectTableModel::s_defaultColors = {
    QColor("#e53935"),  // 红色
    QColor("#fb8c00"),  // 橙色
    QColor("#fdd835"),  // 黄色
    QColor("#43a047"),  // 绿色
    QColor("#1e88e5"),  // 蓝色
    QColor("#8e24aa"),  // 紫色
    QColor("#00acc1"),  // 青色
    QColor("#6d4c41"),  // 棕色
    QColor("#546e7a"),  // 灰色
    QColor("#d81b60"),  // 粉色
};

DefectItem::DefectItem(const DefectRegion& region, int idx) {
    id = idx;
    classId = region.classId;
    bbox = QRect(region.bbox.x, region.bbox.y, region.bbox.width, region.bbox.height);
    confidence = region.confidence;
    area = region.bbox.width * region.bbox.height;
    
    // 根据置信度设置严重度
    if (confidence >= 0.9) {
        severity = "Critical";
    } else if (confidence >= 0.7) {
        severity = "Major";
    } else if (confidence >= 0.5) {
        severity = "Minor";
    } else {
        severity = "Low";
    }
}

DefectTableModel::DefectTableModel(QObject *parent)
    : QAbstractTableModel{parent}
{
    // 初始化默认类别名称
    m_classNames[0] = tr("划痕");
    m_classNames[1] = tr("裂纹");
    m_classNames[2] = tr("异物");
    m_classNames[3] = tr("尺寸异常");
    m_classNames[4] = tr("污渍");
    m_classNames[5] = tr("气泡");
    m_classNames[6] = tr("凹陷");
    m_classNames[7] = tr("凸起");
}

void DefectTableModel::setDefects(const std::vector<DefectRegion>& defects) {
    beginResetModel();
    m_defects.clear();
    m_defects.reserve(static_cast<int>(defects.size()));
    
    int idx = 1;
    for (const auto& region : defects) {
        DefectItem item(region, idx++);
        item.className = className(region.classId);
        item.color = classColor(region.classId);
        m_defects.append(item);
    }
    
    endResetModel();
    
    LOG_DEBUG("DefectTableModel::setDefects - Loaded {} defects", m_defects.size());
    emit defectsChanged();
}

void DefectTableModel::setDefects(const QVector<DefectItem>& defects) {
    beginResetModel();
    m_defects = defects;
    
    // 更新ID和颜色
    for (int i = 0; i < m_defects.size(); ++i) {
        m_defects[i].id = i + 1;
        if (m_defects[i].color == QColor()) {
            m_defects[i].color = classColor(m_defects[i].classId);
        }
        if (m_defects[i].className.isEmpty()) {
            m_defects[i].className = className(m_defects[i].classId);
        }
    }
    
    endResetModel();
    
    LOG_DEBUG("DefectTableModel::setDefects - Set {} defects", m_defects.size());
    emit defectsChanged();
}

void DefectTableModel::addDefect(const DefectItem& defect) {
    int row = m_defects.size();
    beginInsertRows(QModelIndex(), row, row);
    
    DefectItem item = defect;
    item.id = row + 1;
    if (item.color == QColor()) {
        item.color = classColor(item.classId);
    }
    if (item.className.isEmpty()) {
        item.className = className(item.classId);
    }
    m_defects.append(item);
    
    endInsertRows();
    
    LOG_DEBUG("DefectTableModel::addDefect - Added defect id={}, class={}", 
              item.id, item.className.toStdString());
    emit defectsChanged();
}

void DefectTableModel::removeDefect(int row) {
    if (row < 0 || row >= m_defects.size()) {
        return;
    }
    
    beginRemoveRows(QModelIndex(), row, row);
    int removedId = m_defects[row].id;
    m_defects.removeAt(row);
    
    // 重新编号
    for (int i = 0; i < m_defects.size(); ++i) {
        m_defects[i].id = i + 1;
    }
    
    endRemoveRows();
    
    LOG_DEBUG("DefectTableModel::removeDefect - Removed defect at row {}, id={}", row, removedId);
    emit defectsChanged();
}

void DefectTableModel::updateDefect(int row, const DefectItem& defect) {
    if (row < 0 || row >= m_defects.size()) {
        return;
    }
    
    m_defects[row] = defect;
    m_defects[row].id = row + 1;
    
    emit dataChanged(index(row, 0), index(row, ColCount - 1));
    emit defectsChanged();
}

void DefectTableModel::clear() {
    if (m_defects.isEmpty()) {
        return;
    }
    
    beginResetModel();
    m_defects.clear();
    endResetModel();
    
    LOG_DEBUG("DefectTableModel::clear - Cleared all defects");
    emit defectsChanged();
}

DefectItem DefectTableModel::defectAt(int row) const {
    if (row >= 0 && row < m_defects.size()) {
        return m_defects.at(row);
    }
    return DefectItem();
}

double DefectTableModel::totalArea() const {
    double total = 0.0;
    for (const auto& defect : m_defects) {
        total += defect.area;
    }
    return total;
}

double DefectTableModel::maxConfidence() const {
    if (m_defects.isEmpty()) return 0.0;
    
    double maxVal = 0.0;
    for (const auto& defect : m_defects) {
        maxVal = std::max(maxVal, defect.confidence);
    }
    return maxVal;
}

double DefectTableModel::minConfidence() const {
    if (m_defects.isEmpty()) return 0.0;
    
    double minVal = 1.0;
    for (const auto& defect : m_defects) {
        minVal = std::min(minVal, defect.confidence);
    }
    return minVal;
}

double DefectTableModel::avgConfidence() const {
    if (m_defects.isEmpty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& defect : m_defects) {
        sum += defect.confidence;
    }
    return sum / m_defects.size();
}

QMap<QString, int> DefectTableModel::classStatistics() const {
    QMap<QString, int> stats;
    for (const auto& defect : m_defects) {
        stats[defect.className]++;
    }
    return stats;
}

int DefectTableModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return m_defects.size();
}

int DefectTableModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return ColCount;
}

QVariant DefectTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_defects.size()) {
        return QVariant();
    }

    const auto& defect = m_defects.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case ColId:
                return defect.id;
            case ColClass:
                return defect.className;
            case ColConfidence:
                return QString("%1%").arg(defect.confidence * 100, 0, 'f', 1);
            case ColArea:
                return QString("%1 px²").arg(static_cast<int>(defect.area));
            case ColPosition:
                return formatPosition(defect.bbox);
            case ColSize:
                return formatSize(defect.bbox);
            case ColSeverity:
                return defect.severity;
        }
    } else if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
            case ColId:
            case ColConfidence:
            case ColArea:
                return Qt::AlignCenter;
            case ColPosition:
            case ColSize:
                return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
            default:
                return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        }
    } else if (role == Qt::ForegroundRole) {
        if (index.column() == ColSeverity) {
            return QBrush(severityColor(defect.severity));
        }
        if (index.column() == ColClass) {
            return QBrush(defect.color.darker(120));
        }
    } else if (role == Qt::BackgroundRole) {
        if (index.column() == ColClass) {
            return QBrush(defect.color.lighter(180));
        }
        if (index.column() == ColSeverity) {
            return QBrush(severityColor(defect.severity).lighter(180));
        }
    } else if (role == Qt::DecorationRole) {
        if (index.column() == ColClass) {
            // 返回一个小色块作为图标
            return defect.color;
        }
    } else if (role == Qt::UserRole) {
        return defect.id;
    } else if (role == Qt::UserRole + 1) {
        // 返回完整的 bbox 信息
        return QVariant::fromValue(defect.bbox);
    }

    return QVariant();
}

QVariant DefectTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
        case ColId:         return tr("序号");
        case ColClass:      return tr("类别");
        case ColConfidence: return tr("置信度");
        case ColArea:       return tr("面积");
        case ColPosition:   return tr("位置");
        case ColSize:       return tr("尺寸");
        case ColSeverity:   return tr("严重度");
    }

    return QVariant();
}

Qt::ItemFlags DefectTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void DefectTableModel::setClassColor(int classId, const QColor& color) {
    m_classColors[classId] = color;
}

void DefectTableModel::setClassColor(const QString& className, const QColor& color) {
    m_classColorsByName[className] = color;
}

QColor DefectTableModel::classColor(int classId) const {
    if (m_classColors.contains(classId)) {
        return m_classColors[classId];
    }
    // 使用默认颜色循环
    int idx = classId % s_defaultColors.size();
    return s_defaultColors[idx];
}

void DefectTableModel::setClassName(int classId, const QString& name) {
    m_classNames[classId] = name;
}

QString DefectTableModel::className(int classId) const {
    if (m_classNames.contains(classId)) {
        return m_classNames[classId];
    }
    return tr("未知类型 %1").arg(classId);
}

QColor DefectTableModel::severityColor(const QString& severity) const {
    if (severity == "Critical") {
        return QColor("#c62828");  // 深红
    } else if (severity == "Major") {
        return QColor("#ef6c00");  // 橙色
    } else if (severity == "Minor") {
        return QColor("#f9a825");  // 黄色
    } else {
        return QColor("#2e7d32");  // 绿色
    }
}

QString DefectTableModel::formatPosition(const QRect& bbox) const {
    return QString("(%1, %2)").arg(bbox.x()).arg(bbox.y());
}

QString DefectTableModel::formatSize(const QRect& bbox) const {
    return QString("%1 × %2").arg(bbox.width()).arg(bbox.height());
}
