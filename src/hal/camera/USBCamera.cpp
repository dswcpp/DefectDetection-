#include "USBCamera.h"

bool USBCamera::open(const CameraConfig &cfg) {
  Q_UNUSED(cfg);
  // TODO: 打开 USB 相机，配置分辨率/曝光/增益
  return true;
}

bool USBCamera::grab(cv::Mat &frame) {
  Q_UNUSED(frame);
  // TODO: 采集一帧图像
  return true;
}

void USBCamera::close() {
  // TODO: 关闭相机、释放资源
}
