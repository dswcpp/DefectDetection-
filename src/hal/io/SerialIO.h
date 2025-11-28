#ifndef SERIALIO_H
#define SERIALIO_H

#include "IIOController.h"

class SerialIO : public IIOController {
public:
  SerialIO() = default;

  bool init(const QString &configPath) override;
  bool writeOutput(int channel, bool value) override;
  bool readInput(int channel, bool &value) override;
};

#endif // SERIALIO_H
