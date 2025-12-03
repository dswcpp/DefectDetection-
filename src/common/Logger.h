/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * Logger.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：日志记录模块接口定义
 * 描述：基于spdlog封装的日志系统，支持多级别日志(DEBUG/INFO/WARN/ERROR)，
 *       支持控制台和文件输出，提供LOG_DEBUG/LOG_INFO等便捷宏
 *
 * 当前版本：1.0
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <QByteArray>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QMap>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "common_global.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

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
