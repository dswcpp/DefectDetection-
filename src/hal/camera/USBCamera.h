#ifndef USBCAMERA_H
#define USBCAMERA_H

#include "ICamera.h"

class USBCamera : public ICamera {
public:
  USBCamera() = default;

  bool open(const CameraConfig &cfg) override;
  bool grab(cv::Mat &frame) override;
  void close() override;
};

#endif // USBCAMERA_H
