#include "GigECamera.h"

bool GigECamera::open(const CameraConfig &cfg) {
  Q_UNUSED(cfg);
  // TODO: 连接 GigE 相机，设置曝光/增益/分辨率
  return true;
}

bool GigECamera::grab(cv::Mat &frame) {
  Q_UNUSED(frame);
  // TODO: 采集一帧图像
  return true;
}

void GigECamera::close() {
  // TODO: 关闭相机、释放资源
}
