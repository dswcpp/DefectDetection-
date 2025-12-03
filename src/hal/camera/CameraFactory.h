/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * CameraFactory.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：相机工厂类定义
 * 描述：相机对象工厂，根据类型字符串(hik/daheng/gige/usb/file)创建对应相机实例
 *
 * 当前版本：1.0
 */

#ifndef CAMERAFACTORY_H
#define CAMERAFACTORY_H

#include <memory>
#include <QString>
#include "ICamera.h"
#include "hal_global.h"

class HAL_EXPORT CameraFactory {
public:
  static std::unique_ptr<ICamera> create(const QString &type);
};

#endif // CAMERAFACTORY_H
