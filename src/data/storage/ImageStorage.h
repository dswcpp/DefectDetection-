/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ImageStorage.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：图像存储模块接口定义
 * 描述：管理检测图像的存储，支持原图/标注图保存、按日期分目录、
 *       自动清理过期文件等功能
 *
 * 当前版本：1.0
 */

#ifndef IMAGESTORAGE_H
#define IMAGESTORAGE_H

#include <QString>
#include <QByteArray>

class ImageStorage {
public:
  ImageStorage() = default;

  // TODO: 保存原图/标注图
  bool saveImage(const QString &path, const QByteArray &data);

  // TODO: 加载图像
  QByteArray loadImage(const QString &path) const;
};

#endif // IMAGESTORAGE_H
