#include "USBCamera.h"
#include "common/Logger.h"

bool USBCamera::open(const CameraConfig &cfg) {
  LOG_INFO("USBCamera::open - Opening camera, serial={}, resolution={}x{}", 
           cfg.serial.toStdString(), cfg.width, cfg.height);
  // TODO: 打开 USB 相机，配置分辨率/曝光/增益
  LOG_WARN("USBCamera::open - Not implemented yet");
  return true;
}

bool USBCamera::grab(cv::Mat &frame) {
  // TODO: 采集一帧图像
  frame = cv::Mat();
  return false;
}

void USBCamera::close() {
  LOG_INFO("USBCamera::close - Closing camera");
  // TODO: 关闭相机、释放资源
}
