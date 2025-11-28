#ifndef SERIALLIGHTCONTROLLER_H
#define SERIALLIGHTCONTROLLER_H

#include "ILightController.h"

class SerialLightController : public ILightController {
public:
  SerialLightController() = default;

  bool connect(const QString &port) override;
  bool setBrightness(int channel, int value) override;
  void disconnect() override;
};

#endif // SERIALLIGHTCONTROLLER_H
