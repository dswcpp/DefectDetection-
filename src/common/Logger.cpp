/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * Logger.cpp
 *
 * 版本：2.0
 * 作者：Vere
 * 修改日期：2025年12月04日
 */

#include "Logger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDateTime>
#include <QDirIterator>

#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

namespace logging {

// 静态成员初始化
std::mutex Logger::s_mutex;
std::atomic<bool> Logger::s_initialized{false};
LoggerConfig Logger::s_config{};
std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;
LoggerStats Logger::s_stats{};
Logger::ErrorCallback Logger::s_errorCallback = nullptr;
std::string Logger::s_lastError;

std::mutex Logger::s_moduleMutex;
std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> Logger::s_moduleLoggers;

std::atomic<bool> Logger::s_flushTimerRunning{false};
std::unique_ptr<std::thread> Logger::s_flushThread = nullptr;

// ==================== LoggerConfig ====================

LoggerConfig LoggerConfig::fromJsonFile(const std::string& filePath) {
    LoggerConfig config;
    
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly)) {
        return config;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();
    
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return config;
    }
    
    QJsonObject obj = doc.object();
    
    // 基本配置
    if (obj.contains("logDir")) config.logDir = obj["logDir"].toString().toStdString();
    if (obj.contains("logPrefix")) config.logPrefix = obj["logPrefix"].toString().toStdString();
    if (obj.contains("level")) config.level = obj["level"].toString().toStdString();
    if (obj.contains("pattern")) config.pattern = obj["pattern"].toString().toStdString();
    
    // 输出配置
    if (obj.contains("enableConsole")) config.enableConsole = obj["enableConsole"].toBool();
    if (obj.contains("enableFile")) config.enableFile = obj["enableFile"].toBool();
    
    // 异步配置
    if (obj.contains("asyncMode")) config.asyncMode = obj["asyncMode"].toBool();
    if (obj.contains("asyncQueueSize")) config.asyncQueueSize = static_cast<size_t>(obj["asyncQueueSize"].toInt());
    if (obj.contains("asyncThreadCount")) config.asyncThreadCount = static_cast<size_t>(obj["asyncThreadCount"].toInt());
    
    // 轮转配置
    if (obj.contains("rotationPolicy")) {
        QString policy = obj["rotationPolicy"].toString().toLower();
        if (policy == "daily") config.rotationPolicy = RotationPolicy::Daily;
        else if (policy == "hourly") config.rotationPolicy = RotationPolicy::Hourly;
        else config.rotationPolicy = RotationPolicy::Size;
    }
    if (obj.contains("maxFileSizeMB")) config.maxFileSizeMB = static_cast<size_t>(obj["maxFileSizeMB"].toInt());
    if (obj.contains("maxFileCount")) config.maxFileCount = static_cast<size_t>(obj["maxFileCount"].toInt());
    if (obj.contains("rotationHour")) config.rotationHour = obj["rotationHour"].toInt();
    if (obj.contains("rotationMinute")) config.rotationMinute = obj["rotationMinute"].toInt();
    
    // 刷新配置
    if (obj.contains("flushIntervalMs")) config.flushIntervalMs = static_cast<size_t>(obj["flushIntervalMs"].toInt());
    
    // 清理配置
    if (obj.contains("enableAutoClean")) config.enableAutoClean = obj["enableAutoClean"].toBool();
    if (obj.contains("keepDays")) config.keepDays = obj["keepDays"].toInt();
    
    // 模块过滤
    if (obj.contains("enabledModules")) {
        QJsonArray arr = obj["enabledModules"].toArray();
        for (const auto& v : arr) {
            config.enabledModules.insert(v.toString().toStdString());
        }
    }
    if (obj.contains("disabledModules")) {
        QJsonArray arr = obj["disabledModules"].toArray();
        for (const auto& v : arr) {
            config.disabledModules.insert(v.toString().toStdString());
        }
    }
    
    return config;
}

bool LoggerConfig::saveToJsonFile(const std::string& filePath) const {
    QJsonObject obj;
    
    // 基本配置
    obj["logDir"] = QString::fromStdString(logDir);
    obj["logPrefix"] = QString::fromStdString(logPrefix);
    obj["level"] = QString::fromStdString(level);
    obj["pattern"] = QString::fromStdString(pattern);
    
    // 输出配置
    obj["enableConsole"] = enableConsole;
    obj["enableFile"] = enableFile;
    
    // 异步配置
    obj["asyncMode"] = asyncMode;
    obj["asyncQueueSize"] = static_cast<int>(asyncQueueSize);
    obj["asyncThreadCount"] = static_cast<int>(asyncThreadCount);
    
    // 轮转配置
    switch (rotationPolicy) {
        case RotationPolicy::Daily: obj["rotationPolicy"] = "daily"; break;
        case RotationPolicy::Hourly: obj["rotationPolicy"] = "hourly"; break;
        default: obj["rotationPolicy"] = "size"; break;
    }
    obj["maxFileSizeMB"] = static_cast<int>(maxFileSizeMB);
    obj["maxFileCount"] = static_cast<int>(maxFileCount);
    obj["rotationHour"] = rotationHour;
    obj["rotationMinute"] = rotationMinute;
    
    // 刷新配置
    obj["flushIntervalMs"] = static_cast<int>(flushIntervalMs);
    
    // 清理配置
    obj["enableAutoClean"] = enableAutoClean;
    obj["keepDays"] = keepDays;
    
    // 模块过滤
    QJsonArray enabledArr, disabledArr;
    for (const auto& m : enabledModules) {
        enabledArr.append(QString::fromStdString(m));
    }
    for (const auto& m : disabledModules) {
        disabledArr.append(QString::fromStdString(m));
    }
    obj["enabledModules"] = enabledArr;
    obj["disabledModules"] = disabledArr;
    
    QJsonDocument doc(obj);
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

// ==================== LoggerStats ====================

void LoggerStats::reset() {
    traceCount = 0;
    debugCount = 0;
    infoCount = 0;
    warnCount = 0;
    errorCount = 0;
    criticalCount = 0;
    droppedCount = 0;
}

uint64_t LoggerStats::totalCount() const {
    return traceCount + debugCount + infoCount + warnCount + errorCount + criticalCount;
}

std::string LoggerStats::toString() const {
    std::ostringstream oss;
    oss << "LoggerStats { "
        << "trace=" << traceCount.load()
        << ", debug=" << debugCount.load()
        << ", info=" << infoCount.load()
        << ", warn=" << warnCount.load()
        << ", error=" << errorCount.load()
        << ", critical=" << criticalCount.load()
        << ", dropped=" << droppedCount.load()
        << ", total=" << totalCount()
        << " }";
    return oss.str();
}

// ==================== Logger ====================

spdlog::level::level_enum Logger::parseLevel(const std::string& level) {
    std::string lower = level;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "trace") return spdlog::level::trace;
    if (lower == "debug") return spdlog::level::debug;
    if (lower == "info") return spdlog::level::info;
    if (lower == "warn" || lower == "warning") return spdlog::level::warn;
    if (lower == "error" || lower == "err") return spdlog::level::err;
    if (lower == "critical" || lower == "fatal") return spdlog::level::critical;
    if (lower == "off") return spdlog::level::off;
    return spdlog::level::info;
}

std::string Logger::levelToString(spdlog::level::level_enum level) {
    switch (level) {
        case spdlog::level::trace: return "trace";
        case spdlog::level::debug: return "debug";
        case spdlog::level::info: return "info";
        case spdlog::level::warn: return "warn";
        case spdlog::level::err: return "error";
        case spdlog::level::critical: return "critical";
        case spdlog::level::off: return "off";
        default: return "info";
    }
}

void Logger::setLastError(const std::string& error) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_lastError = error;
    if (s_errorCallback) {
        s_errorCallback(error);
    }
}

bool Logger::createLogger() {
    try {
        std::vector<spdlog::sink_ptr> sinks;
        
        // 控制台输出
        if (s_config.enableConsole) {
#ifdef _WIN32
            SetConsoleOutputCP(CP_UTF8);
            SetConsoleCP(CP_UTF8);
#endif
            auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            consoleSink->set_pattern(s_config.pattern);
            sinks.push_back(consoleSink);
        }
        
        // 文件输出
        if (s_config.enableFile) {
            spdlog::sink_ptr fileSink;
            
            std::string filename = s_config.logDir + "/" + s_config.logPrefix;
            
            switch (s_config.rotationPolicy) {
                case RotationPolicy::Daily:
                    fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
                        filename + ".log",
                        s_config.rotationHour,
                        s_config.rotationMinute,
                        false,  // truncate
                        static_cast<uint16_t>(s_config.maxFileCount)
                    );
                    break;
                    
                case RotationPolicy::Hourly:
                    // spdlog没有hourly sink，使用daily模拟（每小时一个文件需要自定义）
                    fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
                        filename + ".log",
                        0, 0, false,
                        static_cast<uint16_t>(s_config.maxFileCount)
                    );
                    break;
                    
                case RotationPolicy::Size:
                default:
                    fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                        filename + ".log",
                        s_config.maxFileSizeMB * 1024 * 1024,
                        s_config.maxFileCount
                    );
                    break;
            }
            
            fileSink->set_pattern(s_config.pattern);
            sinks.push_back(fileSink);
        }
        
        if (sinks.empty()) {
            setLastError("No sinks configured (both console and file disabled)");
            return false;
        }
        
        // 创建 logger
        if (s_config.asyncMode) {
            spdlog::init_thread_pool(s_config.asyncQueueSize, s_config.asyncThreadCount);
            s_logger = std::make_shared<spdlog::async_logger>(
                s_config.logPrefix,
                sinks.begin(), sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::overrun_oldest
            );
        } else {
            s_logger = std::make_shared<spdlog::logger>(
                s_config.logPrefix,
                sinks.begin(), sinks.end()
            );
        }
        
        // 设置级别
        auto level = parseLevel(s_config.level);
        s_logger->set_level(level);
        s_logger->flush_on(s_config.flushLevel);
        
        // 设置为默认 logger
        spdlog::set_default_logger(s_logger);
        spdlog::set_level(level);
        
        return true;
        
    } catch (const spdlog::spdlog_ex& ex) {
        setLastError(std::string("spdlog exception: ") + ex.what());
        return false;
    } catch (const std::exception& ex) {
        setLastError(std::string("Exception: ") + ex.what());
        return false;
    }
}

void Logger::startFlushTimer() {
    if (s_config.flushIntervalMs == 0) {
        return;
    }
    
    s_flushTimerRunning = true;
    try {
        s_flushThread = std::make_unique<std::thread>([]() {
            while (s_flushTimerRunning.load()) {
                // 使用较短的睡眠间隔，以便更快响应停止信号
                for (size_t i = 0; i < s_config.flushIntervalMs / 100 && s_flushTimerRunning.load(); ++i) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                if (s_flushTimerRunning.load() && s_logger) {
                    try {
                        s_logger->flush();
                    } catch (...) {
                        // 忽略刷新错误
                    }
                }
            }
        });
    } catch (const std::exception& e) {
        s_flushTimerRunning = false;
        setLastError(std::string("Failed to start flush timer: ") + e.what());
    }
}

void Logger::stopFlushTimer() {
    s_flushTimerRunning = false;
    if (s_flushThread) {
        try {
            if (s_flushThread->joinable()) {
                // 设置超时，避免无限等待
                s_flushThread->join();
            }
        } catch (...) {
            // 忽略 join 错误
        }
        s_flushThread.reset();
    }
}

bool Logger::init(const LoggerConfig& config) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    if (s_initialized) {
        setLastError("Logger already initialized");
        return false;
    }
    
    s_config = config;
    
    // 处理默认日志目录
    if (s_config.logDir.empty() || s_config.logDir == "logs") {
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (appDataPath.isEmpty()) {
            appDataPath = QDir::currentPath();
        }
        s_config.logDir = (appDataPath + "/logs").toStdString();
    }
    
    // 创建日志目录
    QDir dir(QString::fromStdString(s_config.logDir));
    if (!dir.exists() && !dir.mkpath(".")) {
        setLastError("Failed to create log directory: " + s_config.logDir);
        return false;
    }
    
    // 创建 logger
    if (!createLogger()) {
        return false;
    }
    
    // 启动定期刷新
    startFlushTimer();
    
    // 自动清理旧日志
    if (s_config.enableAutoClean) {
        cleanOldLogs(s_config.keepDays);
    }
    
    s_initialized = true;
    return true;
}

bool Logger::initFromFile(const std::string& configPath) {
    LoggerConfig config = LoggerConfig::fromJsonFile(configPath);
    return init(config);
}

void Logger::shutdown() {
    // 先标记为未初始化，阻止新的日志调用
    bool wasInitialized = s_initialized.exchange(false);
    
    if (!wasInitialized) {
        return;
    }
    
    // 停止定时器（在锁外执行，避免死锁）
    stopFlushTimer();
    
    std::lock_guard<std::mutex> lock(s_mutex);
    
    // 刷新并关闭
    if (s_logger) {
        try {
            s_logger->flush();
        } catch (...) {
            // 忽略刷新错误
        }
    }
    
    // 清理模块 logger
    {
        std::lock_guard<std::mutex> moduleLock(s_moduleMutex);
        s_moduleLoggers.clear();
    }
    
    // 重置 logger
    s_logger.reset();
    
    // 关闭 spdlog（可能会有全局状态问题，用 try-catch 保护）
    try {
        spdlog::shutdown();
    } catch (...) {
        // 忽略关闭错误
    }
}

bool Logger::isInitialized() {
    return s_initialized;
}

std::shared_ptr<spdlog::logger> Logger::instance() {
    // 快速路径：已初始化
    if (s_initialized.load()) {
        return s_logger;
    }
    
    // 慢路径：需要创建 fallback logger
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_initialized.load() && !s_logger) {
        // 创建一个临时的控制台 logger
        try {
            // 先检查是否已存在同名 logger
            auto existing = spdlog::get("fallback");
            if (existing) {
                s_logger = existing;
            } else {
                s_logger = spdlog::stdout_color_mt("fallback");
                s_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
                s_logger->set_level(spdlog::level::debug);
            }
        } catch (const std::exception&) {
            // 如果创建失败，返回 nullptr
            return nullptr;
        }
    }
    return s_logger;
}

std::shared_ptr<spdlog::logger> Logger::getModuleLogger(const std::string& module) {
    std::lock_guard<std::mutex> lock(s_moduleMutex);
    
    auto it = s_moduleLoggers.find(module);
    if (it != s_moduleLoggers.end()) {
        return it->second;
    }
    
    // 创建模块 logger（共享相同的 sinks）
    if (s_logger) {
        auto moduleLogger = s_logger->clone(module);
        // 修改 pattern 添加模块名
        std::string modulePattern = s_config.pattern;
        size_t pos = modulePattern.find("%v");
        if (pos != std::string::npos) {
            modulePattern.insert(pos, "[" + module + "] ");
        }
        for (auto& sink : moduleLogger->sinks()) {
            sink->set_pattern(modulePattern);
        }
        s_moduleLoggers[module] = moduleLogger;
        return moduleLogger;
    }
    
    return s_logger;
}

void Logger::setLevel(const std::string& level) {
    setLevel(parseLevel(level));
}

void Logger::setLevel(spdlog::level::level_enum level) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_config.level = levelToString(level);
    if (s_logger) {
        s_logger->set_level(level);
    }
    spdlog::set_level(level);
    
    // 更新所有模块 logger
    std::lock_guard<std::mutex> moduleLock(s_moduleMutex);
    for (auto& [name, logger] : s_moduleLoggers) {
        logger->set_level(level);
    }
}

spdlog::level::level_enum Logger::getLevel() {
    if (s_logger) {
        return s_logger->level();
    }
    return parseLevel(s_config.level);
}

std::string Logger::getLevelString() {
    return levelToString(getLevel());
}

void Logger::enableModule(const std::string& module) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_config.enabledModules.insert(module);
    s_config.disabledModules.erase(module);
}

void Logger::disableModule(const std::string& module) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_config.disabledModules.insert(module);
    s_config.enabledModules.erase(module);
}

void Logger::clearModuleFilters() {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_config.enabledModules.clear();
    s_config.disabledModules.clear();
}

bool Logger::isModuleEnabled(const std::string& module) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    // 如果在禁用列表中，返回 false
    if (s_config.disabledModules.count(module) > 0) {
        return false;
    }
    
    // 如果启用列表为空，默认全部启用
    if (s_config.enabledModules.empty()) {
        return true;
    }
    
    // 否则只有在启用列表中的才启用
    return s_config.enabledModules.count(module) > 0;
}

void Logger::flush() {
    if (s_logger) {
        s_logger->flush();
    }
}

int Logger::cleanOldLogs(int keepDays) {
    if (keepDays < 0) {
        keepDays = s_config.keepDays;
    }
    
    if (keepDays <= 0) {
        return 0;
    }
    
    int deletedCount = 0;
    QDateTime threshold = QDateTime::currentDateTime().addDays(-keepDays);
    
    QDir logDir(QString::fromStdString(s_config.logDir));
    if (!logDir.exists()) {
        return 0;
    }
    
    QStringList filters;
    filters << "*.log" << "*.log.*";
    
    QDirIterator it(logDir.absolutePath(), filters, QDir::Files);
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        
        if (fileInfo.lastModified() < threshold) {
            if (QFile::remove(filePath)) {
                deletedCount++;
            }
        }
    }
    
    return deletedCount;
}

const LoggerStats& Logger::stats() {
    return s_stats;
}

void Logger::resetStats() {
    s_stats.reset();
}

LoggerConfig Logger::currentConfig() {
    std::lock_guard<std::mutex> lock(s_mutex);
    return s_config;
}

bool Logger::reloadConfig(const LoggerConfig& config) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    // 保存旧配置以便回滚
    LoggerConfig oldConfig = s_config;
    bool wasInitialized = s_initialized.load();
    
    // 停止当前 logger
    stopFlushTimer();
    if (s_logger) {
        s_logger->flush();
    }
    
    // 清理模块 logger
    {
        std::lock_guard<std::mutex> moduleLock(s_moduleMutex);
        s_moduleLoggers.clear();
    }
    
    s_logger.reset();
    spdlog::shutdown();
    
    // 应用新配置
    s_config = config;
    
    // 确保日志目录存在
    QDir dir(QString::fromStdString(s_config.logDir));
    if (!dir.exists() && !dir.mkpath(".")) {
        // 回滚
        s_config = oldConfig;
        createLogger();
        startFlushTimer();
        setLastError("Failed to create new log directory");
        return false;
    }
    
    // 创建新 logger
    if (!createLogger()) {
        // 回滚
        s_config = oldConfig;
        createLogger();
        startFlushTimer();
        return false;
    }
    
    // 启动定时器
    startFlushTimer();
    
    s_initialized = wasInitialized;
    return true;
}

void Logger::setErrorCallback(ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_errorCallback = std::move(callback);
}

std::string Logger::lastError() {
    std::lock_guard<std::mutex> lock(s_mutex);
    return s_lastError;
}

} // namespace logging
