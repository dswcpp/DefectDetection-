#include "HistoryTableModel.h"
#include "data/repositories/DefectRepository.h"
#include <QColor>
#include <QBrush>

HistoryTableModel::HistoryTableModel(QObject *parent)
    : QAbstractTableModel{parent} {}

void HistoryTableModel::setRepository(DefectRepository* repo) {
  m_repo = repo;
}

void HistoryTableModel::refresh(const InspectionFilter& filter) {
  beginResetModel();
  if (m_repo) {
    m_records = m_repo->queryInspections(filter);
  } else {
    m_records.clear();
  }
  endResetModel();
  emit dataLoaded(m_records.size());
}

void HistoryTableModel::clear() {
  beginResetModel();
  m_records.clear();
  endResetModel();
}

InspectionRecord HistoryTableModel::recordAt(int row) const {
  if (row >= 0 && row < m_records.size()) {
    return m_records.at(row);
  }
  return InspectionRecord();
}

qint64 HistoryTableModel::idAt(int row) const {
  if (row >= 0 && row < m_records.size()) {
    return m_records.at(row).id;
  }
  return -1;
}

int HistoryTableModel::rowCount(const QModelIndex& parent) const {
  Q_UNUSED(parent)
  return m_records.size();
}

int HistoryTableModel::columnCount(const QModelIndex& parent) const {
  Q_UNUSED(parent)
  return ColCount;
}

QVariant HistoryTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() >= m_records.size()) {
    return QVariant();
  }

  const auto& record = m_records.at(index.row());

  if (role == Qt::DisplayRole) {
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
        return QString("%1 ms").arg(record.cycleTimeMs);
      case ColImagePath:
        return record.imagePath;
    }
  } else if (role == Qt::ForegroundRole) {
    if (index.column() == ColResult) {
      if (record.result == "OK") {
        return QBrush(QColor(46, 125, 50));   // green
      } else if (record.result == "NG") {
        return QBrush(QColor(198, 40, 40));   // red
      }
    }
  } else if (role == Qt::BackgroundRole) {
    if (record.result == "NG") {
      return QBrush(QColor(255, 235, 238));   // light red background
    }
  } else if (role == Qt::TextAlignmentRole) {
    if (index.column() == ColId || index.column() == ColDefectCount ||
        index.column() == ColCycleTime) {
      return Qt::AlignCenter;
    }
  } else if (role == Qt::UserRole) {
    return record.id;
  }

  return QVariant();
}

QVariant HistoryTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
    return QVariant();
  }

  switch (section) {
    case ColId:          return tr("ID");
    case ColTime:        return tr("检测时间");
    case ColResult:      return tr("结果");
    case ColDefectCount: return tr("缺陷数");
    case ColSeverity:    return tr("严重度");
    case ColCycleTime:   return tr("耗时");
    case ColImagePath:   return tr("图像路径");
  }

  return QVariant();
}
