#include "Logger.h"
#include <QDir>
#include <QStandardPaths>
#include <QVariant>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/bundled/format.h>
#ifdef _WIN32
#include <windows.h>
#endif

namespace logging {

LoggerConfig Logger::s_config{};
std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;

static spdlog::level::level_enum parseLevel(const std::string &level) {
  if (level == "trace") return spdlog::level::trace;
  if (level == "debug") return spdlog::level::debug;
  if (level == "warn" || level == "warning") return spdlog::level::warn;
  if (level == "error" || level == "err") return spdlog::level::err;
  if (level == "critical") return spdlog::level::critical;
  if (level == "off") return spdlog::level::off;
  return spdlog::level::info;
}

bool Logger::init(const LoggerConfig &config) {
  s_config = config;

  // 默认日志目录：用户数据目录/logs
  if (s_config.logDir.empty()) {
    s_config.logDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString() + "/logs";
  }
  QDir dir(QString::fromStdString(s_config.logDir));
  if (!dir.exists()) {
    dir.mkpath(".");
  }

  std::vector<spdlog::sink_ptr> sinks;
  if (s_config.enableConsole) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");
    sinks.push_back(console_sink);
  }
  if (s_config.enableFile) {
    const auto filename =
        QString("%1/%2.log").arg(QString::fromStdString(s_config.logDir),
                                 QString::fromStdString(s_config.logPrefix))
            .toStdString();
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        filename, static_cast<size_t>(s_config.maxFileSizeMB) * 1024 * 1024, s_config.maxFileCount);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");
    sinks.push_back(file_sink);
  }

  try {
    s_logger = std::make_shared<spdlog::logger>(s_config.logPrefix, sinks.begin(), sinks.end());
    auto lvl = parseLevel(s_config.level);
    s_logger->set_level(lvl);
    s_logger->flush_on(lvl);
    spdlog::set_default_logger(s_logger);
    spdlog::set_level(lvl);
  } catch (const spdlog::spdlog_ex &) {
    return false;
  }
  return true;
}

void Logger::shutdown() {
  if (s_logger) {
    s_logger->flush();
  }
  spdlog::shutdown();
  s_logger.reset();
}

std::shared_ptr<spdlog::logger> Logger::instance() {
  if (!s_logger) {
    try {
      s_logger = spdlog::stdout_color_mt("fallback_logger");
      s_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
      spdlog::set_default_logger(s_logger);
    } catch (...) {
      return nullptr;
    }
  }
  return s_logger;
}

void Logger::setLevel(const std::string &level) {
  auto lvl = parseLevel(level);
  s_config.level = level;
  if (s_logger) {
    s_logger->set_level(lvl);
    s_logger->flush_on(lvl);
  }
  spdlog::set_level(lvl);
}

} // namespace logging
