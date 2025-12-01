#ifndef HIKCAMERA_H
#define HIKCAMERA_H

#include "ICamera.h"
#include "hal_global.h"

class HAL_EXPORT HikCamera : public ICamera {
public:
  HikCamera() = default;

  bool open(const CameraConfig &cfg) override;
  bool grab(cv::Mat &frame) override;
  void close() override;
};

#endif // HIKCAMERA_H
