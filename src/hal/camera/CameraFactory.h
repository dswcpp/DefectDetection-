#ifndef CAMERAFACTORY_H
#define CAMERAFACTORY_H

#include <memory>
#include <QString>
#include "ICamera.h"

class CameraFactory {
public:
  static std::unique_ptr<ICamera> create(const QString &type);
};

#endif // CAMERAFACTORY_H
