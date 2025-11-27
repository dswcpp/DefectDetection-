#ifndef DEFECTTABLEMODEL_H
#define DEFECTTABLEMODEL_H

#include <QObject>

class DefectTableModel : public QObject {
  Q_OBJECT
public:
  explicit DefectTableModel(QObject *parent = nullptr);

signals:
};

#endif // DEFECTTABLEMODEL_H
