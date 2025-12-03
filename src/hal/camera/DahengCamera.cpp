#include "DahengCamera.h"
#include "common/Logger.h"

bool DahengCamera::open(const CameraConfig &cfg) {
  LOG_INFO("DahengCamera::open - Opening camera, ip={}, serial={}", 
           cfg.ip.toStdString(), cfg.serial.toStdString());
  // TODO: 初始化大恒 SDK，打开设备，配置参数
  LOG_WARN("DahengCamera::open - Not implemented yet (Daheng SDK required)");
  return true;
}

bool DahengCamera::grab(cv::Mat &frame) {
  // TODO: 从大恒相机抓图
  frame = cv::Mat();
  return false;
}

void DahengCamera::close() {
  LOG_INFO("DahengCamera::close - Closing camera");
  // TODO: 关闭大恒相机
}
