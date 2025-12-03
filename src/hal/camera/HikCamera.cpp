#include "HikCamera.h"
#include "common/Logger.h"

bool HikCamera::open(const CameraConfig &cfg) {
  LOG_INFO("HikCamera::open - Opening camera, ip={}, serial={}", 
           cfg.ip.toStdString(), cfg.serial.toStdString());
  // TODO: 初始化海康 SDK，打开设备，配置参数
  LOG_WARN("HikCamera::open - Not implemented yet (HIK SDK required)");
  return true;
}

bool HikCamera::grab(cv::Mat &frame) {
  // TODO: 从海康相机抓图
  frame = cv::Mat();
  return false;
}

void HikCamera::close() {
  LOG_INFO("HikCamera::close - Closing camera");
  // TODO: 关闭海康相机
}
