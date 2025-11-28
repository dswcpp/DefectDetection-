#ifndef IMAGESTORAGE_H
#define IMAGESTORAGE_H

#include <QString>

class ImageStorage {
public:
  ImageStorage() = default;

  // TODO: 保存原图/标注图
  bool saveImage(const QString &path, const QByteArray &data);

  // TODO: 加载图像
  QByteArray loadImage(const QString &path) const;
};

#endif // IMAGESTORAGE_H
