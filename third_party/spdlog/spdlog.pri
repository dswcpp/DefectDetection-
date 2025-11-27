# spdlog 日志库
# A fast C++ logging library
# https://github.com/gabime/spdlog

INCLUDEPATH += $$PWD

# 定义 spdlog 宏
DEFINES += SPDLOG_COMPILED_LIB
DEFINES += SPDLOG_FMT_EXTERNAL_HO
DEFINES += SPDLOG_NO_EXCEPTIONS

# 源文件
SOURCES += \
    $$PWD/async.cpp \
    $$PWD/bundled_fmtlib_format.cpp \
    $$PWD/cfg.cpp \
    $$PWD/color_sinks.cpp \
    $$PWD/file_sinks.cpp \
    $$PWD/spdlog.cpp \
    $$PWD/stdout_sinks.cpp

# 头文件
HEADERS += \
    $$PWD/spdlog.h \
    $$PWD/spdlog-inl.h \
    $$PWD/async.h \
    $$PWD/async_logger.h \
    $$PWD/async_logger-inl.h \
    $$PWD/common.h \
    $$PWD/common-inl.h \
    $$PWD/formatter.h \
    $$PWD/fwd.h \
    $$PWD/logger.h \
    $$PWD/logger-inl.h \
    $$PWD/mdc.h \
    $$PWD/pattern_formatter.h \
    $$PWD/pattern_formatter-inl.h \
    $$PWD/stopwatch.h \
    $$PWD/tweakme.h \
    $$PWD/version.h

# cfg 子目录头文件
HEADERS += \
    $$PWD/cfg/argv.h \
    $$PWD/cfg/env.h \
    $$PWD/cfg/helpers.h \
    $$PWD/cfg/helpers-inl.h

# details 子目录头文件
HEADERS += \
    $$PWD/details/backtracer.h \
    $$PWD/details/backtracer-inl.h \
    $$PWD/details/circular_q.h \
    $$PWD/details/console_globals.h \
    $$PWD/details/file_helper.h \
    $$PWD/details/file_helper-inl.h \
    $$PWD/details/fmt_helper.h \
    $$PWD/details/log_msg.h \
    $$PWD/details/log_msg-inl.h \
    $$PWD/details/log_msg_buffer.h \
    $$PWD/details/log_msg_buffer-inl.h \
    $$PWD/details/mpmc_blocking_q.h \
    $$PWD/details/null_mutex.h \
    $$PWD/details/os.h \
    $$PWD/details/os-inl.h \
    $$PWD/details/periodic_worker.h \
    $$PWD/details/periodic_worker-inl.h \
    $$PWD/details/registry.h \
    $$PWD/details/registry-inl.h \
    $$PWD/details/synchronous_factory.h \
    $$PWD/details/tcp_client.h \
    $$PWD/details/tcp_client-windows.h \
    $$PWD/details/thread_pool.h \
    $$PWD/details/thread_pool-inl.h \
    $$PWD/details/udp_client.h \
    $$PWD/details/udp_client-windows.h \
    $$PWD/details/windows_include.h

# fmt 子目录头文件
HEADERS += \
    $$PWD/fmt/bin_to_hex.h \
    $$PWD/fmt/chrono.h \
    $$PWD/fmt/compile.h \
    $$PWD/fmt/fmt.h \
    $$PWD/fmt/ostr.h \
    $$PWD/fmt/ranges.h \
    $$PWD/fmt/std.h \
    $$PWD/fmt/xchar.h

# fmt/bundled 子目录头文件
HEADERS += \
    $$PWD/fmt/bundled/args.h \
    $$PWD/fmt/bundled/base.h \
    $$PWD/fmt/bundled/chrono.h \
    $$PWD/fmt/bundled/color.h \
    $$PWD/fmt/bundled/compile.h \
    $$PWD/fmt/bundled/core.h \
    $$PWD/fmt/bundled/format.h \
    $$PWD/fmt/bundled/format-inl.h \
    $$PWD/fmt/bundled/os.h \
    $$PWD/fmt/bundled/ostream.h \
    $$PWD/fmt/bundled/printf.h \
    $$PWD/fmt/bundled/ranges.h \
    $$PWD/fmt/bundled/std.h \
    $$PWD/fmt/bundled/xchar.h

# sinks 子目录头文件
HEADERS += \
    $$PWD/sinks/android_sink.h \
    $$PWD/sinks/ansicolor_sink.h \
    $$PWD/sinks/ansicolor_sink-inl.h \
    $$PWD/sinks/base_sink.h \
    $$PWD/sinks/base_sink-inl.h \
    $$PWD/sinks/basic_file_sink.h \
    $$PWD/sinks/basic_file_sink-inl.h \
    $$PWD/sinks/callback_sink.h \
    $$PWD/sinks/daily_file_sink.h \
    $$PWD/sinks/dist_sink.h \
    $$PWD/sinks/dup_filter_sink.h \
    $$PWD/sinks/hourly_file_sink.h \
    $$PWD/sinks/kafka_sink.h \
    $$PWD/sinks/mongo_sink.h \
    $$PWD/sinks/msvc_sink.h \
    $$PWD/sinks/null_sink.h \
    $$PWD/sinks/ostream_sink.h \
    $$PWD/sinks/qt_sinks.h \
    $$PWD/sinks/ringbuffer_sink.h \
    $$PWD/sinks/rotating_file_sink.h \
    $$PWD/sinks/rotating_file_sink-inl.h \
    $$PWD/sinks/sink.h \
    $$PWD/sinks/sink-inl.h \
    $$PWD/sinks/stdout_color_sinks.h \
    $$PWD/sinks/stdout_color_sinks-inl.h \
    $$PWD/sinks/stdout_sinks.h \
    $$PWD/sinks/stdout_sinks-inl.h \
    $$PWD/sinks/syslog_sink.h \
    $$PWD/sinks/systemd_sink.h \
    $$PWD/sinks/tcp_sink.h \
    $$PWD/sinks/udp_sink.h \
    $$PWD/sinks/win_eventlog_sink.h \
    $$PWD/sinks/wincolor_sink.h \
    $$PWD/sinks/wincolor_sink-inl.h

