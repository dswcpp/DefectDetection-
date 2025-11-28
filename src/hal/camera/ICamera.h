#ifndef ICAMERA_H
#define ICAMERA_H

#include <QString>
#include <opencv2/opencv.hpp>

struct CameraConfig {
  QString type;    // gige/usb/file/mock
  QString ip;
  int port = 0;
  QString serial;
  int width = 0;
  int height = 0;
  double exposureUs = 0.0;
  double gainDb = 0.0;
};

class ICamera {
public:
  virtual ~ICamera() = default;

  virtual bool open(const CameraConfig &cfg) = 0;
  virtual bool grab(cv::Mat &frame) = 0;
  virtual void close() = 0;

  // TODO: 参数设置/心跳/事件回调接口
};

#endif // ICAMERA_H
