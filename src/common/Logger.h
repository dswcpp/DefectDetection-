#ifndef LOGGER_H
#define LOGGER_H

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
#include <vector>
#include "common_global.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

// 轻量 spdlog 封装，支持 Qt 类型
namespace logging {

struct LoggerConfig {
  std::string logDir = "logs";
  std::string logPrefix = "DefectDetection";
  std::string level = "info"; // trace/debug/info/warn/error/critical/off
  int maxFileSizeMB = 50;
  int maxFileCount = 10;
  bool enableConsole = true;
  bool enableFile = true;
};

class COMMON_LIBRARY Logger {
public:
  static bool init(const LoggerConfig &config = LoggerConfig());
  static void shutdown();

  // 获取全局 spdlog logger
  static std::shared_ptr<class spdlog::logger> instance();

  // 级别控制
  static void setLevel(const std::string &level);

private:
  static LoggerConfig s_config;
  static std::shared_ptr<class spdlog::logger> s_logger;
};

} // namespace logging

// 便捷宏：使用 spdlog，带源位置
#define LOG_TRACE(...) SPDLOG_LOGGER_CALL(logging::Logger::instance().get(), spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG_LOGGER_CALL(logging::Logger::instance().get(), spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(...)  SPDLOG_LOGGER_CALL(logging::Logger::instance().get(), spdlog::level::info,  __VA_ARGS__)
#define LOG_WARN(...)  SPDLOG_LOGGER_CALL(logging::Logger::instance().get(), spdlog::level::warn,  __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_CALL(logging::Logger::instance().get(), spdlog::level::err,   __VA_ARGS__)
#define LOG_CRIT(...)  SPDLOG_LOGGER_CALL(logging::Logger::instance().get(), spdlog::level::critical, __VA_ARGS__)

#endif // LOGGER_H
