#ifndef HISTORYTABLEMODEL_H
#define HISTORYTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include "ui_global.h"
#include "data/repositories/DefectRepository.h"

class UI_LIBRARY HistoryTableModel : public QAbstractTableModel {
  Q_OBJECT
public:
  enum Column {
    ColId = 0,
    ColTime,
    ColResult,
    ColDefectCount,
    ColSeverity,
    ColCycleTime,
    ColImagePath,
    ColCount
  };

  explicit HistoryTableModel(QObject *parent = nullptr);

  void setRepository(DefectRepository* repo);
  void refresh(const InspectionFilter& filter);
  void clear();

  InspectionRecord recordAt(int row) const;
  qint64 idAt(int row) const;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

signals:
  void dataLoaded(int count);

private:
  DefectRepository* m_repo = nullptr;
  QVector<InspectionRecord> m_records;
};

#endif // HISTORYTABLEMODEL_H
