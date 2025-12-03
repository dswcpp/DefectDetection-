/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * DetectorFactory.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：检测器工厂类定义
 * 描述：检测器对象工厂，根据类型字符串创建对应检测器实例，
 *       支持注册自定义检测器类型
 *
 * 当前版本：1.0
 */

#ifndef DETECTORFACTORY_H
#define DETECTORFACTORY_H

#include "algorithm_global.h"
#include "IDefectDetector.h"
#include <QString>
#include <QStringList>
#include <functional>
#include <map>

// 检测器工厂类
class ALGORITHM_LIBRARY DetectorFactory {
public:
  // 创建函数类型
  using CreatorFunc = std::function<DetectorPtr()>;

  // 获取单例
  static DetectorFactory& instance();

  // 注册检测器类型
  void registerDetector(const QString& type, CreatorFunc creator);

  // 创建检测器
  DetectorPtr create(const QString& type) const;

  // 获取已注册的检测器类型列表
  QStringList registeredTypes() const;

  // 检查类型是否已注册
  bool isRegistered(const QString& type) const;

  // 预定义的检测器类型常量
  static const QString TYPE_SCRATCH;
  static const QString TYPE_CRACK;
  static const QString TYPE_FOREIGN;
  static const QString TYPE_DIMENSION;
  static const QString TYPE_YOLO;

private:
  DetectorFactory() = default;
  ~DetectorFactory() = default;
  DetectorFactory(const DetectorFactory&) = delete;
  DetectorFactory& operator=(const DetectorFactory&) = delete;

  std::map<QString, CreatorFunc> m_creators;
};

// 便捷宏：注册检测器
#define REGISTER_DETECTOR(type, className) \
  namespace { \
    struct className##Registrar { \
      className##Registrar() { \
        DetectorFactory::instance().registerDetector(type, []() { \
          return std::make_shared<className>(); \
        }); \
      } \
    } g_##className##Registrar; \
  }

#endif // DETECTORFACTORY_H
