#include "ConfigManager.h"
#include "Logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QReadLocker>
#include <QWriteLocker>
#include <QStandardPaths>

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
    // 用户配置保存到用户数据目录，避免被构建覆盖
    QString userDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return userDataPath + "/config/app.json";
}

QString ConfigManager::bundledConfigPath() {
    // 应用程序自带的默认配置（只读）
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

    // 如果用户配置不存在，从应用程序目录复制默认配置
    if (!QFile::exists(targetPath)) {
        QString bundled = bundledConfigPath();
        if (QFile::exists(bundled)) {
            // 确保目标目录存在
            QFileInfo fi(targetPath);
            QDir dir = fi.dir();
            if (!dir.exists()) {
                dir.mkpath(".");
            }
            // 复制默认配置到用户目录
            if (QFile::copy(bundled, targetPath)) {
                LOG_INFO("ConfigManager: Copied default config to {}", targetPath);
            } else {
                LOG_WARN("ConfigManager: Failed to copy default config");
            }
        }
    }

    if (!loadFromFile(targetPath)) {
        // 尝试从应用程序目录加载
        QString bundled = bundledConfigPath();
        if (targetPath != bundled && loadFromFile(bundled)) {
            LOG_INFO("ConfigManager: Loaded bundled config from {}", bundled);
            // 保存到用户目录
            m_configPath = targetPath;
            save();
            emit configLoaded(targetPath);
            return true;
        }
        
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

DetectorsConfig ConfigManager::detectorsConfig() const {
    QReadLocker locker(&m_lock);
    return m_config.detectors;
}

void ConfigManager::setDetectorsConfig(const DetectorsConfig& cfg, bool autoSave) {
    {
        QWriteLocker locker(&m_lock);
        m_config.detectors = cfg;
    }
    emitChanged("detectors", autoSave);
}

ScratchDetectorConfig ConfigManager::scratchConfig() const {
    QReadLocker locker(&m_lock);
    return m_config.detectors.scratch;
}

void ConfigManager::setScratchConfig(const ScratchDetectorConfig& cfg, bool autoSave) {
    {
        QWriteLocker locker(&m_lock);
        m_config.detectors.scratch = cfg;
    }
    emitChanged("detectors.scratch", autoSave);
}

CrackDetectorConfig ConfigManager::crackConfig() const {
    QReadLocker locker(&m_lock);
    return m_config.detectors.crack;
}

void ConfigManager::setCrackConfig(const CrackDetectorConfig& cfg, bool autoSave) {
    {
        QWriteLocker locker(&m_lock);
        m_config.detectors.crack = cfg;
    }
    emitChanged("detectors.crack", autoSave);
}

ForeignDetectorConfig ConfigManager::foreignConfig() const {
    QReadLocker locker(&m_lock);
    return m_config.detectors.foreign;
}

void ConfigManager::setForeignConfig(const ForeignDetectorConfig& cfg, bool autoSave) {
    {
        QWriteLocker locker(&m_lock);
        m_config.detectors.foreign = cfg;
    }
    emitChanged("detectors.foreign", autoSave);
}

DimensionDetectorConfig ConfigManager::dimensionConfig() const {
    QReadLocker locker(&m_lock);
    return m_config.detectors.dimension;
}

void ConfigManager::setDimensionConfig(const DimensionDetectorConfig& cfg, bool autoSave) {
    {
        QWriteLocker locker(&m_lock);
        m_config.detectors.dimension = cfg;
    }
    emitChanged("detectors.dimension", autoSave);
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
