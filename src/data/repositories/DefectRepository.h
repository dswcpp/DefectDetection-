#ifndef DEFECTREPOSITORY_H
#define DEFECTREPOSITORY_H

#include "IRepository.h"
#include <QObject>

class DefectRepository : public QObject, public IRepository {
  Q_OBJECT
public:
  explicit DefectRepository(QObject *parent = nullptr);

  QString name() const override { return "DefectRepository"; }

  // TODO: 插入缺陷记录
  bool insert();

  // TODO: 查询缺陷记录
  bool query();
};

#endif // DEFECTREPOSITORY_H
