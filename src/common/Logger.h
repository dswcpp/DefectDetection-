/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * Logger.h
 *
 * 版本：2.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 修改日期：2025年12月04日
 * 摘要：健壮的日志记录模块
 * 描述：基于spdlog封装的日志系统
 *   - 线程安全的单例模式
 *   - 支持同步/异步日志
 *   - 支持按大小/按日期轮转
 *   - 支持模块化日志过滤
 *   - 支持从JSON配置文件加载
 *   - 支持定期刷新和日志清理
 */

#ifndef LOGGER_H
#define LOGGER_H

#include "common_global.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QMap>

#include <memory>
#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <unordered_set>
#include <functional>

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

// fmt 对 Qt 类型的 formatter 特化
template <> struct fmt::formatter<QString> {
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const QString &qstr, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", qstr.toUtf8().constData());
    }
};

template <> struct fmt::formatter<QByteArray> {
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const QByteArray &qba, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", std::string(qba.constData(), qba.size()));
    }
};

template <> struct fmt::formatter<QStringList> {
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const QStringList &qsl, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", qsl.join(", ").toStdString());
    }
};

template <> struct fmt::formatter<QMap<QString, QString>> {
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const QMap<QString, QString> &qm, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "[QMap:{}]", qm.size());
    }
};

namespace logging {

// 日志轮转策略
enum class RotationPolicy {
    Size,       // 按大小轮转
    Daily,      // 按日期轮转（每天）
    Hourly      // 按小时轮转
};

// 日志配置结构
struct COMMON_LIBRARY LoggerConfig {
    // 基本配置
    std::string logDir = "logs";
    std::string logPrefix = "DefectDetection";
    std::string level = "info";  // trace/debug/info/warn/error/critical/off
    std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v";
    
    // 输出配置
    bool enableConsole = true;
    bool enableFile = true;
    
    // 异步配置
    bool asyncMode = false;
    size_t asyncQueueSize = 8192;
    size_t asyncThreadCount = 1;
    
    // 轮转配置
    RotationPolicy rotationPolicy = RotationPolicy::Size;
    size_t maxFileSizeMB = 50;
    size_t maxFileCount = 10;
    int rotationHour = 0;   // 日期轮转时的小时（0-23）
    int rotationMinute = 0; // 日期轮转时的分钟（0-59）
    
    // 刷新配置
    size_t flushIntervalMs = 0;  // 定期刷新间隔（毫秒），0表示禁用（默认禁用避免DLL卸载问题）
    spdlog::level::level_enum flushLevel = spdlog::level::warn;  // 达到此级别立即刷新
    
    // 清理配置
    bool enableAutoClean = true;
    int keepDays = 30;  // 保留最近N天的日志
    
    // 模块过滤（空表示不过滤）
    std::unordered_set<std::string> enabledModules;
    std::unordered_set<std::string> disabledModules;
    
    // 从JSON文件加载配置
    static LoggerConfig fromJsonFile(const std::string& filePath);
    
    // 保存配置到JSON文件
    bool saveToJsonFile(const std::string& filePath) const;
};

// 日志统计信息
struct COMMON_LIBRARY LoggerStats {
    std::atomic<uint64_t> traceCount{0};
    std::atomic<uint64_t> debugCount{0};
    std::atomic<uint64_t> infoCount{0};
    std::atomic<uint64_t> warnCount{0};
    std::atomic<uint64_t> errorCount{0};
    std::atomic<uint64_t> criticalCount{0};
    std::atomic<uint64_t> droppedCount{0};  // 异步模式下丢弃的日志数
    
    void reset();
    uint64_t totalCount() const;
    std::string toString() const;
};

// Logger 主类
class COMMON_LIBRARY Logger {
public:
    // 禁止拷贝和移动
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    
    // 初始化和关闭
    static bool init(const LoggerConfig& config = LoggerConfig());
    static bool initFromFile(const std::string& configPath);
    static void shutdown();
    static bool isInitialized();
    
    // 获取 logger 实例
    static std::shared_ptr<spdlog::logger> instance();
    
    // 获取指定模块的 logger
    static std::shared_ptr<spdlog::logger> getModuleLogger(const std::string& module);
    
    // 级别控制
    static void setLevel(const std::string& level);
    static void setLevel(spdlog::level::level_enum level);
    static spdlog::level::level_enum getLevel();
    static std::string getLevelString();
    
    // 模块过滤
    static void enableModule(const std::string& module);
    static void disableModule(const std::string& module);
    static void clearModuleFilters();
    static bool isModuleEnabled(const std::string& module);
    
    // 手动刷新
    static void flush();
    
    // 日志清理
    static int cleanOldLogs(int keepDays = -1);  // 返回删除的文件数，-1使用配置值
    
    // 统计信息
    static const LoggerStats& stats();
    static void resetStats();
    
    // 获取当前配置
    static LoggerConfig currentConfig();
    
    // 重新加载配置
    static bool reloadConfig(const LoggerConfig& config);
    
    // 设置错误回调
    using ErrorCallback = std::function<void(const std::string& msg)>;
    static void setErrorCallback(ErrorCallback callback);
    
    // 获取最后的错误信息
    static std::string lastError();

private:
    Logger() = default;
    ~Logger() = default;
    
    static spdlog::level::level_enum parseLevel(const std::string& level);
    static std::string levelToString(spdlog::level::level_enum level);
    static bool createLogger();
    static void startFlushTimer();
    static void stopFlushTimer();
    static void setLastError(const std::string& error);
    
    // 静态成员
    static std::mutex s_mutex;
    static std::atomic<bool> s_initialized;
    static LoggerConfig s_config;
    static std::shared_ptr<spdlog::logger> s_logger;
    static LoggerStats s_stats;
    static ErrorCallback s_errorCallback;
    static std::string s_lastError;
    
    // 模块 logger 缓存
    static std::mutex s_moduleMutex;
    static std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> s_moduleLoggers;
    
    // 刷新定时器（使用线程实现）
    static std::atomic<bool> s_flushTimerRunning;
    static std::unique_ptr<std::thread> s_flushThread;
};

} // namespace logging

// ==================== 便捷日志宏 ====================

// 基础日志宏
#define LOG_TRACE(...) \
    do { \
        if (auto logger = logging::Logger::instance()) { \
            SPDLOG_LOGGER_CALL(logger.get(), spdlog::level::trace, __VA_ARGS__); \
        } \
    } while(0)

#define LOG_DEBUG(...) \
    do { \
        if (auto logger = logging::Logger::instance()) { \
            SPDLOG_LOGGER_CALL(logger.get(), spdlog::level::debug, __VA_ARGS__); \
        } \
    } while(0)

#define LOG_INFO(...) \
    do { \
        if (auto logger = logging::Logger::instance()) { \
            SPDLOG_LOGGER_CALL(logger.get(), spdlog::level::info, __VA_ARGS__); \
        } \
    } while(0)

#define LOG_WARN(...) \
    do { \
        if (auto logger = logging::Logger::instance()) { \
            SPDLOG_LOGGER_CALL(logger.get(), spdlog::level::warn, __VA_ARGS__); \
        } \
    } while(0)

#define LOG_ERROR(...) \
    do { \
        if (auto logger = logging::Logger::instance()) { \
            SPDLOG_LOGGER_CALL(logger.get(), spdlog::level::err, __VA_ARGS__); \
        } \
    } while(0)

#define LOG_CRIT(...) \
    do { \
        if (auto logger = logging::Logger::instance()) { \
            SPDLOG_LOGGER_CALL(logger.get(), spdlog::level::critical, __VA_ARGS__); \
        } \
    } while(0)

// 模块化日志宏
#define LOG_MODULE_TRACE(module, ...) \
    do { \
        if (logging::Logger::isModuleEnabled(module)) { \
            if (auto logger = logging::Logger::getModuleLogger(module)) { \
                SPDLOG_LOGGER_CALL(logger.get(), spdlog::level::trace, __VA_ARGS__); \
            } \
        } \
    } while(0)

#define LOG_MODULE_DEBUG(module, ...) \
    do { \
        if (logging::Logger::isModuleEnabled(module)) { \
            if (auto logger = logging::Logger::getModuleLogger(module)) { \
                SPDLOG_LOGGER_CALL(logger.get(), spdlog::level::debug, __VA_ARGS__); \
            } \
        } \
    } while(0)

#define LOG_MODULE_INFO(module, ...) \
    do { \
        if (logging::Logger::isModuleEnabled(module)) { \
            if (auto logger = logging::Logger::getModuleLogger(module)) { \
                SPDLOG_LOGGER_CALL(logger.get(), spdlog::level::info, __VA_ARGS__); \
            } \
        } \
    } while(0)

#define LOG_MODULE_WARN(module, ...) \
    do { \
        if (logging::Logger::isModuleEnabled(module)) { \
            if (auto logger = logging::Logger::getModuleLogger(module)) { \
                SPDLOG_LOGGER_CALL(logger.get(), spdlog::level::warn, __VA_ARGS__); \
            } \
        } \
    } while(0)

#define LOG_MODULE_ERROR(module, ...) \
    do { \
        if (logging::Logger::isModuleEnabled(module)) { \
            if (auto logger = logging::Logger::getModuleLogger(module)) { \
                SPDLOG_LOGGER_CALL(logger.get(), spdlog::level::err, __VA_ARGS__); \
            } \
        } \
    } while(0)

// 条件日志宏
#define LOG_IF(condition, level, ...) \
    do { \
        if (condition) { \
            LOG_##level(__VA_ARGS__); \
        } \
    } while(0)

// 仅Debug构建输出
#ifdef NDEBUG
    #define LOG_DEBUG_ONLY(...) ((void)0)
#else
    #define LOG_DEBUG_ONLY(...) LOG_DEBUG(__VA_ARGS__)
#endif

// 带频率限制的日志（每N次调用输出一次）
#define LOG_EVERY_N(n, level, ...) \
    do { \
        static std::atomic<int> _log_count{0}; \
        if (++_log_count % (n) == 0) { \
            LOG_##level(__VA_ARGS__); \
        } \
    } while(0)

// 首次调用输出
#define LOG_FIRST_N(n, level, ...) \
    do { \
        static std::atomic<int> _log_count{0}; \
        if (_log_count.fetch_add(1) < (n)) { \
            LOG_##level(__VA_ARGS__); \
        } \
    } while(0)

#endif // LOGGER_H
