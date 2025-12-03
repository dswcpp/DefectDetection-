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
