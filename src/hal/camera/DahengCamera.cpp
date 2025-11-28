#include "DahengCamera.h"

bool DahengCamera::open(const CameraConfig &cfg) {
  Q_UNUSED(cfg);
  // TODO: 初始化大恒 SDK，打开设备，配置参数
  return true;
}

bool DahengCamera::grab(cv::Mat &frame) {
  Q_UNUSED(frame);
  // TODO: 从大恒相机抓图
  return true;
}

void DahengCamera::close() {
  // TODO: 关闭大恒相机
}
