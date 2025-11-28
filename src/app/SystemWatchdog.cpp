#include "SystemWatchdog.h"

SystemWatchdog::SystemWatchdog(QObject *parent) : QObject(parent) {}

void SystemWatchdog::start() {
  // TODO: 启动定时检查心跳
}

void SystemWatchdog::stop() {
  // TODO: 停止监控
}

void SystemWatchdog::feed(const QString &module) {
  Q_UNUSED(module);
  // TODO: 更新模块心跳时间
}
