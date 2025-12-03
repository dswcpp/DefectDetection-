#include "ImageStorage.h"
#include "common/Logger.h"

bool ImageStorage::saveImage(const QString &path, const QByteArray &data) {
  LOG_DEBUG("ImageStorage::saveImage - path={}, size={} bytes", path.toStdString(), data.size());
  // TODO: 写文件到磁盘
  LOG_WARN("ImageStorage::saveImage - Not implemented yet");
  return true;
}

QByteArray ImageStorage::loadImage(const QString &path) const {
  LOG_DEBUG("ImageStorage::loadImage - path={}", path.toStdString());
  // TODO: 读取磁盘文件
  LOG_WARN("ImageStorage::loadImage - Not implemented yet");
  return {};
}
