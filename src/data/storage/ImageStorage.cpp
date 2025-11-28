#include "ImageStorage.h"

bool ImageStorage::saveImage(const QString &path, const QByteArray &data) {
  Q_UNUSED(path);
  Q_UNUSED(data);
  // TODO: 写文件到磁盘
  return true;
}

QByteArray ImageStorage::loadImage(const QString &path) const {
  Q_UNUSED(path);
  // TODO: 读取磁盘文件
  return {};
}
