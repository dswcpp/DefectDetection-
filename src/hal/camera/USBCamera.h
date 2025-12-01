#ifndef USBCAMERA_H
#define USBCAMERA_H

#include "ICamera.h"
#include "hal_global.h"

class HAL_EXPORT USBCamera : public ICamera {
public:
  USBCamera() = default;

  bool open(const CameraConfig &cfg) override;
  bool grab(cv::Mat &frame) override;
  void close() override;
};

#endif // USBCAMERA_H
