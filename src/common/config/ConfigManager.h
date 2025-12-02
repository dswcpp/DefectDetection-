#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "common_global.h"
#include "config/AppConfig.h"
#include <QObject>
#include <QReadWriteLock>
#include <QString>

class COMMON_LIBRARY ConfigManager : public QObject {
    Q_OBJECT

public:
    // Meyers' Singleton - C++11 线程安全
    static ConfigManager& instance();

    // 禁止拷贝和移动
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = delete;
    ConfigManager& operator=(ConfigManager&&) = delete;

    // ======================== 加载/保存 ========================

    // 加载配置文件，path 为空则使用默认路径
    bool load(const QString& path = QString());

    // 保存配置到当前路径
    bool save();

    // 保存配置到指定路径
    bool saveAs(const QString& path);

    // 重新加载配置
    bool reload();

    // 是否已加载
    bool isLoaded() const;

    // ======================== 路径管理 ========================

    QString configPath() const;
    void setConfigPath(const QString& path);
    static QString defaultConfigPath();   // 用户配置路径（可写）
    static QString bundledConfigPath();   // 应用自带配置路径（只读）

    // ======================== 完整配置访问 ========================

    // 获取完整配置（线程安全，返回副本）
    AppConfig config() const;

    // 设置完整配置
    void setConfig(const AppConfig& cfg, bool autoSave = false);

    // ======================== 分模块配置访问 ========================

    CameraConfig cameraConfig() const;
    void setCameraConfig(const CameraConfig& cfg, bool autoSave = false);

    DetectionConfig detectionConfig() const;
    void setDetectionConfig(const DetectionConfig& cfg, bool autoSave = false);

    UIConfig uiConfig() const;
    void setUIConfig(const UIConfig& cfg, bool autoSave = false);

    DatabaseConfig databaseConfig() const;
    void setDatabaseConfig(const DatabaseConfig& cfg, bool autoSave = false);

    LogConfig logConfig() const;
    void setLogConfig(const LogConfig& cfg, bool autoSave = false);

signals:
    void configLoaded(const QString& path);
    void configSaved(const QString& path);
    void configChanged(const QString& section);
    void configError(const QString& error);

private:
    ConfigManager();
    ~ConfigManager() = default;

    bool loadFromFile(const QString& path);
    bool saveToFile(const QString& path);
    void emitChanged(const QString& section, bool autoSave);

    mutable QReadWriteLock m_lock;
    AppConfig m_config;
    QString m_configPath;
    bool m_loaded = false;
};

// 便捷宏 - 全局访问
#define gConfig ConfigManager::instance()

#endif // CONFIGMANAGER_H
