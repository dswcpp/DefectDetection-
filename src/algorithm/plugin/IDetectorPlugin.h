/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * IDetectorPlugin.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：检测器插件接口定义
 * 描述：Qt插件接口，定义插件元数据、检测器创建方法，
 *       支持动态加载第三方检测器
 *
 * 当前版本：1.0
 */

#ifndef IDETECTORPLUGIN_H
#define IDETECTORPLUGIN_H

#include "../IDefectDetector.h"
#include <QtPlugin>
#include <QString>
#include <QVersionNumber>

// 检测器插件接口
class IDetectorPlugin {
public:
  virtual ~IDetectorPlugin() = default;
  
  // 插件信息
  virtual QString pluginName() const = 0;
  virtual QString pluginDescription() const = 0;
  virtual QString pluginAuthor() const = 0;
  virtual QVersionNumber pluginVersion() const = 0;
  
  // 创建检测器实例
  virtual IDefectDetector* createDetector() = 0;
  
  // 检测器信息
  virtual QString detectorType() const = 0;
  virtual QString detectorName() const = 0;
};

#define IDetectorPlugin_iid "com.defectdetection.IDetectorPlugin/1.0"
Q_DECLARE_INTERFACE(IDetectorPlugin, IDetectorPlugin_iid)

#endif // IDETECTORPLUGIN_H
