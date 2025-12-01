#ifndef ICAMERA_H
#define ICAMERA_H

#include <QString>
#include <opencv2/opencv.hpp>
#include "hal_global.h"

struct HAL_EXPORT CameraConfig {
  QString type;    // gige/usb/file/mock
  QString ip;
  int port = 0;
  QString serial;
  int width = 0;
  int height = 0;
  double exposureUs = 0.0;
  double gainDb = 0.0;
};

class HAL_EXPORT ICamera {
public:
  virtual ~ICamera() = default;

  virtual bool open(const CameraConfig &cfg) = 0;
  virtual bool grab(cv::Mat &frame) = 0;
  virtual void close() = 0;

  // 获取当前图片路径（主要用于 FileCamera）
  virtual QString currentImagePath() const { return QString(); }

  // TODO: 参数设置/心跳/事件回调接口
};

#endif // ICAMERA_H
