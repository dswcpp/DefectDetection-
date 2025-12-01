#include "ConfigManager.h"
#include "Logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QReadLocker>
#include <QWriteLocker>

// ============================================================================
// Meyers' Singleton
// ============================================================================

ConfigManager& ConfigManager::instance() {
    static ConfigManager inst;
    return inst;
}

ConfigManager::ConfigManager() : QObject(nullptr) {
    m_configPath = defaultConfigPath();
}

// ============================================================================
// 路径管理
// ============================================================================

QString ConfigManager::defaultConfigPath() {
    return QCoreApplication::applicationDirPath() + "/config/app.json";
}

QString ConfigManager::configPath() const {
    QReadLocker locker(&m_lock);
    return m_configPath;
}

void ConfigManager::setConfigPath(const QString& path) {
    QWriteLocker locker(&m_lock);
    m_configPath = path;
}

bool ConfigManager::isLoaded() const {
    QReadLocker locker(&m_lock);
    return m_loaded;
}

// ============================================================================
// 加载/保存
// ============================================================================

bool ConfigManager::load(const QString& path) {
    QString targetPath = path.isEmpty() ? defaultConfigPath() : path;

    if (!loadFromFile(targetPath)) {
        LOG_WARN("ConfigManager: Failed to load {}, using defaults", targetPath);
        // 使用默认配置
        QWriteLocker locker(&m_lock);
        m_config = AppConfig();
        m_configPath = targetPath;
        m_loaded = true;
        emit configLoaded(targetPath);
        return false;
    }

    emit configLoaded(targetPath);
    return true;
}

bool ConfigManager::save() {
    QString path;
    {
        QReadLocker locker(&m_lock);
        path = m_configPath;
    }
    return saveAs(path);
}

bool ConfigManager::saveAs(const QString& path) {
    if (!saveToFile(path)) {
        emit configError("Failed to save config: " + path);
        return false;
    }

    {
        QWriteLocker locker(&m_lock);
        m_configPath = path;
    }

    emit configSaved(path);
    return true;
}

bool ConfigManager::reload() {
    QString path;
    {
        QReadLocker locker(&m_lock);
        path = m_configPath;
    }
    return load(path);
}

bool ConfigManager::loadFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_WARN("ConfigManager: Cannot open file {}", path);
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        LOG_ERROR("ConfigManager: JSON parse error at {}: {}", 
                  error.offset, error.errorString());
        return false;
    }

    if (!doc.isObject()) {
        LOG_ERROR("ConfigManager: Root is not a JSON object");
        return false;
    }

    QWriteLocker locker(&m_lock);
    m_config = AppConfig::fromJson(doc.object());
    m_configPath = path;
    m_loaded = true;

    LOG_INFO("ConfigManager: Loaded config from {}", path);
    return true;
}

bool ConfigManager::saveToFile(const QString& path) {
    // 确保目录存在
    QFileInfo fi(path);
    QDir dir = fi.dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            LOG_ERROR("ConfigManager: Cannot create directory {}", dir.path());
            return false;
        }
    }

    QJsonObject json;
    {
        QReadLocker locker(&m_lock);
        json = m_config.toJson();
    }

    QJsonDocument doc(json);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        LOG_ERROR("ConfigManager: Cannot write file {}", path);
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    LOG_INFO("ConfigManager: Saved config to {}", path);
    return true;
}

// ============================================================================
// 完整配置访问
// ============================================================================

AppConfig ConfigManager::config() const {
    QReadLocker locker(&m_lock);
    return m_config;
}

void ConfigManager::setConfig(const AppConfig& cfg, bool autoSave) {
    {
        QWriteLocker locker(&m_lock);
        m_config = cfg;
    }
    emitChanged("all", autoSave);
}

// ============================================================================
// 分模块配置访问
// ============================================================================

CameraConfig ConfigManager::cameraConfig() const {
    QReadLocker locker(&m_lock);
    return m_config.camera;
}

void ConfigManager::setCameraConfig(const CameraConfig& cfg, bool autoSave) {
    {
        QWriteLocker locker(&m_lock);
        m_config.camera = cfg;
    }
    emitChanged("camera", autoSave);
}

DetectionConfig ConfigManager::detectionConfig() const {
    QReadLocker locker(&m_lock);
    return m_config.detection;
}

void ConfigManager::setDetectionConfig(const DetectionConfig& cfg, bool autoSave) {
    {
        QWriteLocker locker(&m_lock);
        m_config.detection = cfg;
    }
    emitChanged("detection", autoSave);
}

UIConfig ConfigManager::uiConfig() const {
    QReadLocker locker(&m_lock);
    return m_config.ui;
}

void ConfigManager::setUIConfig(const UIConfig& cfg, bool autoSave) {
    {
        QWriteLocker locker(&m_lock);
        m_config.ui = cfg;
    }
    emitChanged("ui", autoSave);
}

DatabaseConfig ConfigManager::databaseConfig() const {
    QReadLocker locker(&m_lock);
    return m_config.database;
}

void ConfigManager::setDatabaseConfig(const DatabaseConfig& cfg, bool autoSave) {
    {
        QWriteLocker locker(&m_lock);
        m_config.database = cfg;
    }
    emitChanged("database", autoSave);
}

LogConfig ConfigManager::logConfig() const {
    QReadLocker locker(&m_lock);
    return m_config.log;
}

void ConfigManager::setLogConfig(const LogConfig& cfg, bool autoSave) {
    {
        QWriteLocker locker(&m_lock);
        m_config.log = cfg;
    }
    emitChanged("log", autoSave);
}

// ============================================================================
// 辅助函数
// ============================================================================

void ConfigManager::emitChanged(const QString& section, bool autoSave) {
    emit configChanged(section);
    if (autoSave) {
        save();
    }
}
