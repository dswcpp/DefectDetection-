#ifndef DAHENGCAMERA_H
#define DAHENGCAMERA_H

#include "ICamera.h"

class DahengCamera : public ICamera {
public:
  DahengCamera() = default;

  bool open(const CameraConfig &cfg) override;
  bool grab(cv::Mat &frame) override;
  void close() override;
};

#endif // DAHENGCAMERA_H
