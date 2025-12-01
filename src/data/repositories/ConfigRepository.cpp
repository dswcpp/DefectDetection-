#include "ConfigRepository.h"
#include "config/ConfigManager.h"
#include "config/AppConfig.h"

ConfigRepository::ConfigRepository(QObject* parent)
    : QObject(parent) {
  connectSignals();
}

void ConfigRepository::connectSignals() {
  // 转发 ConfigManager 的信号
  connect(&gConfig, &ConfigManager::configLoaded,
          this, &ConfigRepository::configLoaded);
  connect(&gConfig, &ConfigManager::configSaved,
          this, &ConfigRepository::configSaved);
  connect(&gConfig, &ConfigManager::configChanged,
          this, [this](const QString& section) {
            emit configChanged(section);
            if (section == "database") {
              emit databaseConfigChanged();
            } else if (section == "log") {
              emit logConfigChanged();
            }
          });
}

// ======================== 配置文件操作 ========================

bool ConfigRepository::load(const QString& path) {
  return gConfig.load(path);
}

bool ConfigRepository::save() {
  return gConfig.save();
}

bool ConfigRepository::saveAs(const QString& path) {
  return gConfig.saveAs(path);
}

bool ConfigRepository::reload() {
  return gConfig.reload();
}

QString ConfigRepository::configPath() const {
  return gConfig.configPath();
}

// ======================== 数据库配置 ========================

QString ConfigRepository::databasePath() const {
  return gConfig.databaseConfig().path;
}

void ConfigRepository::setDatabasePath(const QString& path) {
  auto cfg = gConfig.databaseConfig();
  cfg.path = path;
  gConfig.setDatabaseConfig(cfg);
}

int ConfigRepository::maxRecords() const {
  return gConfig.databaseConfig().maxRecords;
}

void ConfigRepository::setMaxRecords(int count) {
  auto cfg = gConfig.databaseConfig();
  cfg.maxRecords = count;
  gConfig.setDatabaseConfig(cfg);
}

bool ConfigRepository::autoCleanup() const {
  return gConfig.databaseConfig().autoCleanup;
}

void ConfigRepository::setAutoCleanup(bool enabled) {
  auto cfg = gConfig.databaseConfig();
  cfg.autoCleanup = enabled;
  gConfig.setDatabaseConfig(cfg);
}

DatabaseConfig ConfigRepository::databaseConfig() const {
  return gConfig.databaseConfig();
}

void ConfigRepository::setDatabaseConfig(const DatabaseConfig& config) {
  gConfig.setDatabaseConfig(config);
}

// ======================== 日志配置 ========================

QString ConfigRepository::logLevel() const {
  return gConfig.logConfig().level;
}

void ConfigRepository::setLogLevel(const QString& level) {
  auto cfg = gConfig.logConfig();
  cfg.level = level;
  gConfig.setLogConfig(cfg);
}

QString ConfigRepository::logDir() const {
  return gConfig.logConfig().dir;
}

void ConfigRepository::setLogDir(const QString& dir) {
  auto cfg = gConfig.logConfig();
  cfg.dir = dir;
  gConfig.setLogConfig(cfg);
}

LogConfig ConfigRepository::logConfig() const {
  return gConfig.logConfig();
}

void ConfigRepository::setLogConfig(const LogConfig& config) {
  gConfig.setLogConfig(config);
}

// ======================== 通用配置访问 ========================

QVariant ConfigRepository::getValue(const QString& key, const QVariant& defaultValue) const {
  QStringList parts = key.split('.');
  if (parts.size() < 2) {
    return defaultValue;
  }

  QString section = parts[0];
  QString field = parts[1];

  if (section == "database") {
    auto cfg = gConfig.databaseConfig();
    if (field == "path") return cfg.path;
    if (field == "maxRecords") return cfg.maxRecords;
    if (field == "autoCleanup") return cfg.autoCleanup;
  } else if (section == "log") {
    auto cfg = gConfig.logConfig();
    if (field == "level") return cfg.level;
    if (field == "dir") return cfg.dir;
    if (field == "maxFileSizeMB") return cfg.maxFileSizeMB;
    if (field == "maxFileCount") return cfg.maxFileCount;
    if (field == "enableConsole") return cfg.enableConsole;
  } else if (section == "camera") {
    auto cfg = gConfig.cameraConfig();
    if (field == "type") return cfg.type;
    if (field == "imageDir") return cfg.imageDir;
    if (field == "ip") return cfg.ip;
    if (field == "captureIntervalMs") return cfg.captureIntervalMs;
    if (field == "loop") return cfg.loop;
    if (field == "exposureUs") return cfg.exposureUs;
    if (field == "gainDb") return cfg.gainDb;
  } else if (section == "detection") {
    auto cfg = gConfig.detectionConfig();
    if (field == "enabled") return cfg.enabled;
    if (field == "confidenceThreshold") return cfg.confidenceThreshold;
    if (field == "modelPath") return cfg.modelPath;
    if (field == "batchSize") return cfg.batchSize;
    if (field == "useGPU") return cfg.useGPU;
  } else if (section == "ui") {
    auto cfg = gConfig.uiConfig();
    if (field == "theme") return cfg.theme;
    if (field == "language") return cfg.language;
    if (field == "showFPS") return cfg.showFPS;
    if (field == "fullscreen") return cfg.fullscreen;
  }

  return defaultValue;
}

void ConfigRepository::setValue(const QString& key, const QVariant& value) {
  QStringList parts = key.split('.');
  if (parts.size() < 2) {
    return;
  }

  QString section = parts[0];
  QString field = parts[1];

  if (section == "database") {
    auto cfg = gConfig.databaseConfig();
    if (field == "path") cfg.path = value.toString();
    else if (field == "maxRecords") cfg.maxRecords = value.toInt();
    else if (field == "autoCleanup") cfg.autoCleanup = value.toBool();
    gConfig.setDatabaseConfig(cfg);
  } else if (section == "log") {
    auto cfg = gConfig.logConfig();
    if (field == "level") cfg.level = value.toString();
    else if (field == "dir") cfg.dir = value.toString();
    else if (field == "maxFileSizeMB") cfg.maxFileSizeMB = value.toInt();
    else if (field == "maxFileCount") cfg.maxFileCount = value.toInt();
    else if (field == "enableConsole") cfg.enableConsole = value.toBool();
    gConfig.setLogConfig(cfg);
  } else if (section == "camera") {
    auto cfg = gConfig.cameraConfig();
    if (field == "type") cfg.type = value.toString();
    else if (field == "imageDir") cfg.imageDir = value.toString();
    else if (field == "ip") cfg.ip = value.toString();
    else if (field == "captureIntervalMs") cfg.captureIntervalMs = value.toInt();
    else if (field == "loop") cfg.loop = value.toBool();
    else if (field == "exposureUs") cfg.exposureUs = value.toDouble();
    else if (field == "gainDb") cfg.gainDb = value.toDouble();
    gConfig.setCameraConfig(cfg);
  } else if (section == "detection") {
    auto cfg = gConfig.detectionConfig();
    if (field == "enabled") cfg.enabled = value.toBool();
    else if (field == "confidenceThreshold") cfg.confidenceThreshold = value.toDouble();
    else if (field == "modelPath") cfg.modelPath = value.toString();
    else if (field == "batchSize") cfg.batchSize = value.toInt();
    else if (field == "useGPU") cfg.useGPU = value.toBool();
    gConfig.setDetectionConfig(cfg);
  } else if (section == "ui") {
    auto cfg = gConfig.uiConfig();
    if (field == "theme") cfg.theme = value.toString();
    else if (field == "language") cfg.language = value.toString();
    else if (field == "showFPS") cfg.showFPS = value.toBool();
    else if (field == "fullscreen") cfg.fullscreen = value.toBool();
    gConfig.setUIConfig(cfg);
  }
}
