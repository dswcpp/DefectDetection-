#ifndef IREPOSITORY_H
#define IREPOSITORY_H

#include <QString>

class IRepository {
public:
  virtual ~IRepository() = default;

  // TODO: 定义通用接口，如 migrate/clear/cache 控制
  virtual QString name() const = 0;
};

#endif // IREPOSITORY_H
