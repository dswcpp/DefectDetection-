#ifndef ILIGHTCONTROLLER_H
#define ILIGHTCONTROLLER_H

#include <QString>

class ILightController {
public:
  virtual ~ILightController() = default;
  virtual bool connect(const QString &port) = 0;
  virtual bool setBrightness(int channel, int value) = 0;
  virtual void disconnect() = 0;

  // TODO: 心跳检测/多通道同步设置
};

#endif // ILIGHTCONTROLLER_H
