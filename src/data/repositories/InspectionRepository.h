#ifndef INSPECTIONREPOSITORY_H
#define INSPECTIONREPOSITORY_H

#include "IRepository.h"
#include <QObject>

class InspectionRepository : public QObject, public IRepository {
  Q_OBJECT
public:
  explicit InspectionRepository(QObject *parent = nullptr);

  QString name() const override { return "InspectionRepository"; }

  // TODO: 插入检测记录
  bool insert();

  // TODO: 查询检测记录
  bool query();
};

#endif // INSPECTIONREPOSITORY_H
