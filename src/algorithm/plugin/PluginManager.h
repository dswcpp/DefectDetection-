#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "../algorithm_global.h"
#include "IDetectorPlugin.h"
#include <QObject>
#include <QMap>
#include <QPluginLoader>
#include <memory>

// 插件信息
struct PluginInfo {
  QString name;
  QString description;
  QString author;
  QVersionNumber version;
  QString filePath;
  QString detectorType;
  bool loaded = false;
};

// 插件管理器：动态加载/卸载检测器插件
class ALGORITHM_LIBRARY PluginManager : public QObject {
  Q_OBJECT
public:
  static PluginManager& instance();
  
  // 扫描并加载插件目录
  int loadPlugins(const QString& directory);
  
  // 加载单个插件
  bool loadPlugin(const QString& filePath);
  
  // 卸载插件
  bool unloadPlugin(const QString& pluginName);
  
  // 卸载所有插件
  void unloadAll();
  
  // 获取已加载插件列表
  QList<PluginInfo> loadedPlugins() const;
  
  // 获取插件信息
  PluginInfo pluginInfo(const QString& pluginName) const;
  
  // 创建检测器实例
  std::shared_ptr<IDefectDetector> createDetector(const QString& pluginName);
  
  // 获取所有可用的检测器类型
  QStringList availableDetectorTypes() const;
  
  // 检查插件是否已加载
  bool isPluginLoaded(const QString& pluginName) const;

signals:
  void pluginLoaded(const QString& name);
  void pluginUnloaded(const QString& name);
  void pluginError(const QString& name, const QString& error);

private:
  PluginManager(QObject* parent = nullptr);
  ~PluginManager();
  PluginManager(const PluginManager&) = delete;
  PluginManager& operator=(const PluginManager&) = delete;
  
  struct LoadedPlugin {
    QPluginLoader* loader = nullptr;
    IDetectorPlugin* plugin = nullptr;
    PluginInfo info;
  };
  
  QMap<QString, LoadedPlugin> m_plugins;
};

// 便捷宏
#define gPluginManager PluginManager::instance()

#endif // PLUGINMANAGER_H
