#include "FileCamera.h"
#include <QImage>

bool FileCamera::open(const CameraConfig &cfg) {
  Q_UNUSED(cfg);
  // TODO: 准备文件列表，供调试读取
  return true;
}

bool FileCamera::grab(cv::Mat &frame) {
  Q_UNUSED(frame);
  // TODO: 从文件读取图像并转换为 cv::Mat
  return true;
}

void FileCamera::close() {
  // TODO: 清理文件句柄/列表
}
