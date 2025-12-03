#include "PluginManager.h"
#include "../common/Logger.h"
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

PluginManager& PluginManager::instance() {
  static PluginManager inst;
  return inst;
}

PluginManager::PluginManager(QObject* parent) : QObject(parent) {
}

PluginManager::~PluginManager() {
  unloadAll();
}

int PluginManager::loadPlugins(const QString& directory) {
  QDir dir(directory);
  if (!dir.exists()) {
    LOG_WARN("PluginManager: Plugin directory does not exist: {}", 
             directory.toStdString());
    return 0;
  }
  
  int count = 0;
  
  // 查找所有动态库文件
#ifdef Q_OS_WIN
  QStringList filters = {"*.dll"};
#elif defined(Q_OS_LINUX)
  QStringList filters = {"*.so"};
#elif defined(Q_OS_MAC)
  QStringList filters = {"*.dylib"};
#endif
  
  QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
  
  for (const QFileInfo& fileInfo : files) {
    if (loadPlugin(fileInfo.absoluteFilePath())) {
      count++;
    }
  }
  
  LOG_INFO("PluginManager: Loaded {} plugins from {}", count, directory.toStdString());
  return count;
}

bool PluginManager::loadPlugin(const QString& filePath) {
  QFileInfo fileInfo(filePath);
  
  if (!fileInfo.exists()) {
    emit pluginError(filePath, "File not found");
    return false;
  }
  
  // 检查是否已加载
  for (const auto& loaded : m_plugins) {
    if (loaded.info.filePath == filePath) {
      LOG_DEBUG("PluginManager: Plugin already loaded: {}", filePath.toStdString());
      return true;
    }
  }
  
  // 加载插件
  QPluginLoader* loader = new QPluginLoader(filePath);
  
  if (!loader->load()) {
    QString error = loader->errorString();
    LOG_ERROR("PluginManager: Failed to load plugin {}: {}", 
              filePath.toStdString(), error.toStdString());
    emit pluginError(fileInfo.fileName(), error);
    delete loader;
    return false;
  }
  
  // 获取插件接口
  QObject* instance = loader->instance();
  if (!instance) {
    LOG_ERROR("PluginManager: Failed to get plugin instance from {}", 
              filePath.toStdString());
    emit pluginError(fileInfo.fileName(), "Failed to get plugin instance");
    loader->unload();
    delete loader;
    return false;
  }
  
  IDetectorPlugin* plugin = qobject_cast<IDetectorPlugin*>(instance);
  if (!plugin) {
    LOG_ERROR("PluginManager: Invalid plugin interface in {}", 
              filePath.toStdString());
    emit pluginError(fileInfo.fileName(), "Invalid plugin interface");
    loader->unload();
    delete loader;
    return false;
  }
  
  // 保存插件信息
  LoadedPlugin loaded;
  loaded.loader = loader;
  loaded.plugin = plugin;
  loaded.info.name = plugin->pluginName();
  loaded.info.description = plugin->pluginDescription();
  loaded.info.author = plugin->pluginAuthor();
  loaded.info.version = plugin->pluginVersion();
  loaded.info.filePath = filePath;
  loaded.info.detectorType = plugin->detectorType();
  loaded.info.loaded = true;
  
  m_plugins[loaded.info.name] = loaded;
  
  LOG_INFO("PluginManager: Loaded plugin '{}' v{} by {}", 
           loaded.info.name.toStdString(),
           loaded.info.version.toString().toStdString(),
           loaded.info.author.toStdString());
  
  emit pluginLoaded(loaded.info.name);
  return true;
}

bool PluginManager::unloadPlugin(const QString& pluginName) {
  auto it = m_plugins.find(pluginName);
  if (it == m_plugins.end()) {
    return false;
  }
  
  LoadedPlugin& loaded = it.value();
  
  if (loaded.loader) {
    loaded.loader->unload();
    delete loaded.loader;
  }
  
  m_plugins.erase(it);
  
  LOG_INFO("PluginManager: Unloaded plugin '{}'", pluginName.toStdString());
  emit pluginUnloaded(pluginName);
  
  return true;
}

void PluginManager::unloadAll() {
  QList<QString> names = m_plugins.keys();
  for (const QString& name : names) {
    unloadPlugin(name);
  }
}

QList<PluginInfo> PluginManager::loadedPlugins() const {
  QList<PluginInfo> result;
  for (const auto& loaded : m_plugins) {
    result.append(loaded.info);
  }
  return result;
}

PluginInfo PluginManager::pluginInfo(const QString& pluginName) const {
  auto it = m_plugins.find(pluginName);
  if (it != m_plugins.end()) {
    return it->info;
  }
  return PluginInfo();
}

std::shared_ptr<IDefectDetector> PluginManager::createDetector(const QString& pluginName) {
  auto it = m_plugins.find(pluginName);
  if (it == m_plugins.end()) {
    LOG_WARN("PluginManager: Plugin not found: {}", pluginName.toStdString());
    return nullptr;
  }
  
  IDefectDetector* detector = it->plugin->createDetector();
  if (!detector) {
    LOG_ERROR("PluginManager: Failed to create detector from plugin: {}", 
              pluginName.toStdString());
    return nullptr;
  }
  
  return std::shared_ptr<IDefectDetector>(detector);
}

QStringList PluginManager::availableDetectorTypes() const {
  QStringList types;
  for (const auto& loaded : m_plugins) {
    types.append(loaded.info.detectorType);
  }
  return types;
}

bool PluginManager::isPluginLoaded(const QString& pluginName) const {
  return m_plugins.contains(pluginName);
}
