#include "GigECamera.h"
#include "common/Logger.h"

bool GigECamera::open(const CameraConfig &cfg) {
  LOG_INFO("GigECamera::open - Opening camera, ip={}, exposure={}us, gain={}dB", 
           cfg.ip.toStdString(), cfg.exposureUs, cfg.gainDb);
  // TODO: 连接 GigE 相机，设置曝光/增益/分辨率
  LOG_WARN("GigECamera::open - Not implemented yet (GigE SDK required)");
  return true;
}

bool GigECamera::grab(cv::Mat &frame) {
  // TODO: 采集一帧图像
  frame = cv::Mat();
  return false;
}

void GigECamera::close() {
  LOG_INFO("GigECamera::close - Closing camera");
  // TODO: 关闭相机、释放资源
}
