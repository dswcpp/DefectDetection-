#ifndef GPIOCONTROLLER_H
#define GPIOCONTROLLER_H

#include "IIOController.h"

class GPIOController : public IIOController {
public:
  GPIOController() = default;

  bool init(const QString &configPath) override;
  bool writeOutput(int channel, bool value) override;
  bool readInput(int channel, bool &value) override;
};

#endif // GPIOCONTROLLER_H
