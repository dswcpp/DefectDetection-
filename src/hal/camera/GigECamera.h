#ifndef GIGECAMERA_H
#define GIGECAMERA_H

#include "ICamera.h"

class GigECamera : public ICamera {
public:
  GigECamera() = default;

  bool open(const CameraConfig &cfg) override;
  bool grab(cv::Mat &frame) override;
  void close() override;
};

#endif // GIGECAMERA_H
