#include "SystemWatchdog.h"
#include "common/Logger.h"

SystemWatchdog::SystemWatchdog(QObject *parent) : QObject(parent) {}

void SystemWatchdog::start() {
  LOG_INFO("SystemWatchdog::start - Starting watchdog monitoring");
  // TODO: 启动定时检查心跳
  LOG_WARN("SystemWatchdog::start - Not implemented yet");
}

void SystemWatchdog::stop() {
  LOG_INFO("SystemWatchdog::stop - Stopping watchdog monitoring");
  // TODO: 停止监控
  LOG_WARN("SystemWatchdog::stop - Not implemented yet");
}

void SystemWatchdog::feed(const QString &module) {
  LOG_DEBUG("SystemWatchdog::feed - Heartbeat from module: {}", module.toStdString());
  // TODO: 更新模块心跳时间
}
