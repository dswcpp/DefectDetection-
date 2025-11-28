#include "HikCamera.h"

bool HikCamera::open(const CameraConfig &cfg) {
  Q_UNUSED(cfg);
  // TODO: 初始化海康 SDK，打开设备，配置参数
  return true;
}

bool HikCamera::grab(cv::Mat &frame) {
  Q_UNUSED(frame);
  // TODO: 从海康相机抓图
  return true;
}

void HikCamera::close() {
  // TODO: 关闭海康相机
}
