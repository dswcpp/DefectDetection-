#ifndef SPDLOG_WRAPPER_H
#define SPDLOG_WRAPPER_H

/**
 * @file fzspdlog.h
 * @brief DefectDetection 项目 spdlog 封装 - 原生支持 Qt 类型
 * @version 4.2.6
 * @date 2025-11-03
 * 
 * 特性：
 * - ✅ 原生支持 QString、QByteArray、QStringList、QMap 等 Qt 类型
 * - ✅ 三种日志方式：格式化、流式、便捷宏
 * - ✅ 自动文件滚动、中文支持、线程安全
 * - ✅ 完整的源文件位置信息（文件名、行号、函数名）
 * - ✅ 控制台彩色输出（Windows UTF-8 支持）
 * 
 * 使用方式：
 * 1. 格式化：QSPDLOG_INFO("用户 {} 端口 {}", username, port)
 * 2. 流式：  spdInfo() << "用户" << username << "端口" << port
 * 3. 标准：  SPDLOG_INFO("消息 {}", value)
 * 
 * Qt 类型无需转换：
 *   QString path = "/opt/app.ini";
 *   QSPDLOG_INFO("路径: {}", path);  // ✅ 直接传递，自动转换
 */

// Qt 头文件
#include <QString>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QDateTime>
#include <QCoreApplication>

// 标准库头文件
#include <iostream>
#include <string>
#include <memory>
#include <sstream>

// Windows 头文件（用于设置控制台 UTF-8 编码）
#ifdef _WIN32
#include <windows.h>
// 取消 Windows 宏定义，避免与 Qt 类名冲突
#ifdef MessageBox
#undef MessageBox
#endif
#endif

// spdlog 相关头文件
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

// ==================== 为 Qt 类型添加 fmt formatter 支持 ====================
/**
 * 通过为 Qt 类型添加 fmt::formatter 特化，使 spdlog 能够直接格式化 Qt 类型
 * 
 * 实现原理：
 * 1. fmt 库通过 formatter 模板特化来支持自定义类型
 * 2. 我们为 QString、QByteArray 等类型特化 formatter
 * 3. 在 format() 方法中将 Qt 类型转换为 std::string
 * 
 * 性能说明：
 * - 转换开销：toStdString() 会进行内存拷贝
 * - 对于频繁日志场景，建议使用 spdInfo() << 流式语法（按需转换）
 * - 对于关键路径，确保生产环境的日志级别过滤掉 DEBUG/TRACE
 */

namespace fmt {

// QString formatter
template <>
struct formatter<QString> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    
    template <typename FormatContext>
    auto format(const QString& qstr, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", qstr.toStdString());
    }
};

// QByteArray formatter  
template <>
struct formatter<QByteArray> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    
    template <typename FormatContext>
    auto format(const QByteArray& qba, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", std::string(qba.constData(), qba.size()));
    }
};

// QStringList formatter
template <>
struct formatter<QStringList> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    
    template <typename FormatContext>
    auto format(const QStringList& qsl, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", qsl.join(", ").toStdString());
    }
};

// QVariantMap formatter（简化表示，显示元素数量）
using QVariantMap = QMap<QString, QVariant>;

template <>
struct formatter<QVariantMap> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    
    template <typename FormatContext>
    auto format(const QVariantMap& qvm, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "[QVariantMap:{}]", qvm.size());
    }
};

// QMap<QString, QString> formatter（简化表示，显示元素数量）
template <>
struct formatter<QMap<QString, QString>> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    
    template <typename FormatContext>
    auto format(const QMap<QString, QString>& qm, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "[QMap:{}]", qm.size());
    }
};

} // namespace fmt

namespace DefectDetection {

/**
 * spdlog 日志配置
 */
struct SpdlogConfig {
    QString logDir;             // 日志目录
    QString logPrefix;          // 日志文件前缀
    int maxFileSize;            // 单个日志文件最大大小（MB）
    int maxFileCount;           // 最大日志文件数量
    std::string logLevel;       // 日志级别：trace/debug/info/warn/error
    bool enableConsole;         // 是否启用控制台输出
    bool enableFile;            // 是否启用文件输出
    
    // 默认配置
    SpdlogConfig()
        : logDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs")
        , logPrefix("DefectDetection")
        , maxFileSize(50)       // 50MB
        , maxFileCount(10)      // 10个文件
        , logLevel("info")
#ifdef _DEBUG
        , enableConsole(true)   // Debug模式启用控制台
#else
        , enableConsole(false)  // Release模式禁用控制台
#endif
        , enableFile(true)
    {
    }
    
    // 从 QSettings 加载配置
    void loadFromSettings() {
        QSettings settings(QCoreApplication::organizationName(), 
                          QCoreApplication::applicationName());
        
        settings.beginGroup("Spdlog");
        logDir = settings.value("LogDir", logDir).toString();
        logPrefix = settings.value("LogPrefix", logPrefix).toString();
        maxFileSize = settings.value("MaxFileSize", maxFileSize).toInt();
        maxFileCount = settings.value("MaxFileCount", maxFileCount).toInt();
        logLevel = settings.value("LogLevel", QString::fromStdString(logLevel)).toString().toStdString();
        enableConsole = settings.value("EnableConsole", enableConsole).toBool();
        enableFile = settings.value("EnableFile", enableFile).toBool();
        settings.endGroup();
    }
    
    // 保存配置到 QSettings
    void saveToSettings() const {
        QSettings settings(QCoreApplication::organizationName(), 
                          QCoreApplication::applicationName());
        
        settings.beginGroup("Spdlog");
        settings.setValue("LogDir", logDir);
        settings.setValue("LogPrefix", logPrefix);
        settings.setValue("MaxFileSize", maxFileSize);
        settings.setValue("MaxFileCount", maxFileCount);
        settings.setValue("LogLevel", QString::fromStdString(logLevel));
        settings.setValue("EnableConsole", enableConsole);
        settings.setValue("EnableFile", enableFile);
        settings.endGroup();
    }
};

/**
 * spdlog 日志管理器
 * 使用 Meyers' Singleton 模式（C++11 线程安全）
 */
class SpdlogManager {
public:
    // 获取单例实例
    static SpdlogManager& getInstance() {
        static SpdlogManager instance;
        return instance;
    }
    
    // 初始化日志系统
    bool initialize(const SpdlogConfig& config = SpdlogConfig()) {
        if (m_initialized) {
            std::cout << "[SpdlogManager] Already initialized." << std::endl;
            return true;
        }
        
        try {
            m_config = config;
            
            // 创建日志目录
            QDir logDir(m_config.logDir);
            if (!logDir.exists()) {
                if (!logDir.mkpath(".")) {
                    std::cerr << "[SpdlogManager] Failed to create log directory: " 
                             << m_config.logDir.toStdString() << std::endl;
                    return false;
                }
            }
            
            // 创建日志器
            createLoggers();
            
            // 设置日志级别和格式
            configureLoggers();
            
            m_initialized = true;
            
            SPDLOG_INFO("Initialized successfully");
            SPDLOG_INFO("Log directory: {}", m_config.logDir.toStdString());
            SPDLOG_INFO("Log level: {}", m_config.logLevel);
            
            return true;
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Initialization failed: " << ex.what() << std::endl;
            return false;
        }
    }
    
    // 获取默认日志器
    std::shared_ptr<spdlog::logger> getLogger() {
        if (!m_logger) {
            // 如果未初始化，创建一个备用控制台日志器
            try {
                m_logger = spdlog::stdout_color_mt("backup_logger");
                m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%^%l%$] [%s:%#] [%!] %v");
            }
            catch (...) {
                // 忽略
            }
        }
        return m_logger;
    }
    
    // 关闭日志系统
    void shutdown() {
        if (m_logger) {
            m_logger->flush();
        }
        spdlog::shutdown();
        m_initialized = false;
    }
    
    // 设置日志级别
    void setLevel(const std::string& level) {
        spdlog::level::level_enum spdLevel = parseLogLevel(level);
        if (m_logger) {
            m_logger->set_level(spdLevel);
            m_logger->flush_on(spdLevel);
        }
        spdlog::set_level(spdLevel);
        m_config.logLevel = level;
    }
    
    // 刷新日志
    void flush() {
        if (m_logger) {
            m_logger->flush();
        }
    }
    
    // 获取配置
    const SpdlogConfig& getConfig() const {
        return m_config;
    }
    
    // 禁用复制和赋值
    SpdlogManager(const SpdlogManager&) = delete;
    SpdlogManager& operator=(const SpdlogManager&) = delete;
    
private:
    SpdlogManager() : m_initialized(false) {}
    
    ~SpdlogManager() {
        shutdown();
    }
    
    // 创建日志器
    void createLoggers() {
        std::vector<spdlog::sink_ptr> sinks;
        
        // 控制台输出
        if (m_config.enableConsole) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            // 格式：[时间] [线程] [级别] [文件:行号] [函数] 消息
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%^%l%$] [%s:%#] [%!] %v");
            sinks.push_back(console_sink);
        }
        
        // 文件输出（按日期滚动）
        if (m_config.enableFile) {
            QString logFileName = QString("%1/%2_%3.log")
                .arg(m_config.logDir)
                .arg(m_config.logPrefix)
                .arg(QDate::currentDate().toString("yyyyMMdd"));
            
            // 使用滚动文件 sink（按大小滚动）
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logFileName.toStdString(),
                m_config.maxFileSize * 1024 * 1024,  // 转换为字节
                m_config.maxFileCount
            );
            // 格式：[时间] [线程] [级别] [文件:行号] [函数] 消息
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%l] [%s:%#] [%!] %v");
            sinks.push_back(file_sink);
        }
        
        // 创建多 sink 日志器
        if (!sinks.empty()) {
            m_logger = std::make_shared<spdlog::logger>(
                m_config.logPrefix.toStdString(),
                sinks.begin(),
                sinks.end()
            );
            
            // 注册为默认日志器
            spdlog::set_default_logger(m_logger);
        }
    }
    
    // 配置日志器
    void configureLoggers() {
        if (!m_logger) {
            return;
        }
        
        // 设置日志级别
        spdlog::level::level_enum level = parseLogLevel(m_config.logLevel);
        m_logger->set_level(level);
        m_logger->flush_on(level);
        
        // 设置全局日志级别
        spdlog::set_level(level);
    }
    
    // 解析日志级别
    spdlog::level::level_enum parseLogLevel(const std::string& level) {
        if (level == "trace") return spdlog::level::trace;
        if (level == "debug") return spdlog::level::debug;
        if (level == "info") return spdlog::level::info;
        if (level == "warn" || level == "warning") return spdlog::level::warn;
        if (level == "error" || level == "err") return spdlog::level::err;
        if (level == "critical") return spdlog::level::critical;
        if (level == "off") return spdlog::level::off;
        
        // 默认返回 info
        return spdlog::level::info;
    }
    
private:
    std::shared_ptr<spdlog::logger> m_logger;
    SpdlogConfig m_config;
    bool m_initialized;
};

/**
 * Qt 风格的流式日志类
 * 类似 qDebug() << "Hello" << value 的用法
 */
class SpdlogStream {
public:
    SpdlogStream(spdlog::level::level_enum level, const char* file, int line)
        : m_level(level)
        , m_file(file)
        , m_line(line)
    {
    }
    
    ~SpdlogStream() {
        // 析构时输出日志
        auto logger = SpdlogManager::getInstance().getLogger();
        if (logger) {
            logger->log(spdlog::source_loc{m_file, m_line, ""}, m_level, "{}", m_stream.str());
        }
    }
    
    // 重载 << 运算符 - QString
    SpdlogStream& operator<<(const QString& msg) {
        m_stream << msg.toStdString();
        return *this;
    }
    
    // 重载 << 运算符 - QByteArray
    SpdlogStream& operator<<(const QByteArray& msg) {
        m_stream << msg.toStdString();
        return *this;
    }
    
    // 重载 << 运算符 - const char*
    SpdlogStream& operator<<(const char* msg) {
        m_stream << msg;
        return *this;
    }
    
    // 重载 << 运算符 - std::string
    SpdlogStream& operator<<(const std::string& msg) {
        m_stream << msg;
        return *this;
    }
    
    // 重载 << 运算符 - int
    SpdlogStream& operator<<(int value) {
        m_stream << value;
        return *this;
    }
    
    // 重载 << 运算符 - long
    SpdlogStream& operator<<(long value) {
        m_stream << value;
        return *this;
    }
    
    // 重载 << 运算符 - long long
    SpdlogStream& operator<<(long long value) {
        m_stream << value;
        return *this;
    }
    
    // 重载 << 运算符 - unsigned int
    SpdlogStream& operator<<(unsigned int value) {
        m_stream << value;
        return *this;
    }
    
    // 重载 << 运算符 - unsigned long
    SpdlogStream& operator<<(unsigned long value) {
        m_stream << value;
        return *this;
    }
    
    // 重载 << 运算符 - unsigned long long
    SpdlogStream& operator<<(unsigned long long value) {
        m_stream << value;
        return *this;
    }
    
    // 重载 << 运算符 - float
    SpdlogStream& operator<<(float value) {
        m_stream << value;
        return *this;
    }
    
    // 重载 << 运算符 - double
    SpdlogStream& operator<<(double value) {
        m_stream << value;
        return *this;
    }
    
    // 重载 << 运算符 - bool
    SpdlogStream& operator<<(bool value) {
        m_stream << (value ? "true" : "false");
        return *this;
    }
    
    // 重载 << 运算符 - void*
    SpdlogStream& operator<<(const void* ptr) {
        m_stream << ptr;
        return *this;
    }
    
    // 禁用复制
    SpdlogStream(const SpdlogStream&) = delete;
    SpdlogStream& operator=(const SpdlogStream&) = delete;
    
private:
    spdlog::level::level_enum m_level;
    const char* m_file;
    int m_line;
    std::ostringstream m_stream;
};

} // namespace DefectDetection

// ==================== 格式化字符串风格宏定义（带文件名和行号）====================
// 先取消 spdlog.h 中的默认定义，避免警告
#ifdef SPDLOG_TRACE
#undef SPDLOG_TRACE
#endif
#ifdef SPDLOG_DEBUG
#undef SPDLOG_DEBUG
#endif
#ifdef SPDLOG_INFO
#undef SPDLOG_INFO
#endif
#ifdef SPDLOG_WARN
#undef SPDLOG_WARN
#endif
#ifdef SPDLOG_ERROR
#undef SPDLOG_ERROR
#endif
#ifdef SPDLOG_CRITICAL
#undef SPDLOG_CRITICAL
#endif

// 使用 SPDLOG_LOGGER_CALL 可以自动传递源文件位置信息
#define SPDLOG_TRACE(...) SPDLOG_LOGGER_CALL(DefectDetection::SpdlogManager::getInstance().getLogger().get(), spdlog::level::trace, __VA_ARGS__)
#define SPDLOG_DEBUG(...) SPDLOG_LOGGER_CALL(DefectDetection::SpdlogManager::getInstance().getLogger().get(), spdlog::level::debug, __VA_ARGS__)
#define SPDLOG_INFO(...) SPDLOG_LOGGER_CALL(DefectDetection::SpdlogManager::getInstance().getLogger().get(), spdlog::level::info, __VA_ARGS__)
#define SPDLOG_WARN(...) SPDLOG_LOGGER_CALL(DefectDetection::SpdlogManager::getInstance().getLogger().get(), spdlog::level::warn, __VA_ARGS__)
#define SPDLOG_ERROR(...) SPDLOG_LOGGER_CALL(DefectDetection::SpdlogManager::getInstance().getLogger().get(), spdlog::level::err, __VA_ARGS__)
#define SPDLOG_CRITICAL(...) SPDLOG_LOGGER_CALL(DefectDetection::SpdlogManager::getInstance().getLogger().get(), spdlog::level::critical, __VA_ARGS__)

// ==================== Qt 风格流式宏定义（类似 qDebug()）====================
#define spdTrace() DefectDetection::SpdlogStream(spdlog::level::trace, __FILE__, __LINE__)
#define spdDebug() DefectDetection::SpdlogStream(spdlog::level::debug, __FILE__, __LINE__)
#define spdInfo() DefectDetection::SpdlogStream(spdlog::level::info, __FILE__, __LINE__)
#define spdWarn() DefectDetection::SpdlogStream(spdlog::level::warn, __FILE__, __LINE__)
#define spdError() DefectDetection::SpdlogStream(spdlog::level::err, __FILE__, __LINE__)
#define spdCritical() DefectDetection::SpdlogStream(spdlog::level::critical, __FILE__, __LINE__)

// ==================== Qt 类型转换辅助工具（可选使用）====================
/**
 * 提供显式转换函数，用于特殊场景
 * 一般情况下不需要使用，fmt formatter 会自动处理
 */
namespace DefectDetection {
namespace QSpdlog {
    
    /// QString → std::string 显式转换
    inline std::string cvt(const QString& v) { return v.toStdString(); }
    
    /// QByteArray → std::string 显式转换
    inline std::string cvt(const QByteArray& v) { return std::string(v.constData(), v.size()); }
    
    /// QStringList → std::string 显式转换（逗号分隔）
    inline std::string cvt(const QStringList& v) { return v.join(", ").toStdString(); }
    
    /// QVariantMap → std::string 显式转换（简化表示）
    inline std::string cvt(const QVariantMap& v) { 
        return QString("[QVariantMap:%1]").arg(v.size()).toStdString(); 
    }
    
    /// QMap<QString, QString> → std::string 显式转换（简化表示）
    inline std::string cvt(const QMap<QString, QString>& v) { 
        return QString("[QMap:%1]").arg(v.size()).toStdString(); 
    }
    
    /// 基本类型直接返回（模板）
    template<typename T>
    inline typename std::enable_if<std::is_arithmetic<T>::value, T>::type 
    cvt(T v) { return v; }
    
    /// const char* 直接返回
    inline const char* cvt(const char* v) { return v; }
    
    /// std::string 直接返回
    inline const std::string& cvt(const std::string& v) { return v; }
    
} // namespace QSpdlog
} // namespace DefectDetection

// ==================== 日志宏定义 ====================

#define QSPDLOG_TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#define QSPDLOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define QSPDLOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define QSPDLOG_WARN(...) SPDLOG_WARN(__VA_ARGS__)
#define QSPDLOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#define QSPDLOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)


// ==================== 兼容性宏 ====================
// Q(...) 辅助宏：向后兼容（现在不再需要，但保留支持）
// 旧代码：QSPDLOG_INFO("path {}", Q(path))
// 新代码：QSPDLOG_INFO("path {}", path)  // 直接传递即可
#define Q(arg) (arg)

#endif // SPDLOG_WRAPPER_H

