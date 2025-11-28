#include "Logger.h"
#include <QDir>
#include <QMap>
#include <QStandardPaths>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/bundled/format.h>

// fmt formatter for Qt types
template <> struct fmt::formatter<QString> {
  constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const QString &qstr, FormatContext &ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "{}", qstr.toStdString());
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

std::shared_ptr<spdlog::logger> Logger::instance() { return s_logger; }

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
