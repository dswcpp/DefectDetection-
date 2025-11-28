#ifndef FILECAMERA_H
#define FILECAMERA_H

#include "ICamera.h"

class FileCamera : public ICamera {
public:
  FileCamera() = default;

  bool open(const CameraConfig &cfg) override;
  bool grab(cv::Mat &frame) override;
  void close() override;
};

#endif // FILECAMERA_H
