#ifndef HISTORYTABLEMODEL_H
#define HISTORYTABLEMODEL_H

#include <QObject>

class HistoryTableModel : public QObject {
  Q_OBJECT
public:
  explicit HistoryTableModel(QObject *parent = nullptr);

signals:
};

#endif // HISTORYTABLEMODEL_H
