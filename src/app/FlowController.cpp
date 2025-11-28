#include "FlowController.h"

FlowController::FlowController(QObject *parent) : QObject(parent) {}

void FlowController::setOnline(bool online) {
  if (m_online == online) {
    return;
  }
  m_online = online;
  emit statusChanged(m_online);
}

bool FlowController::isOnline() const { return m_online; }

void FlowController::start() {
  // TODO: 启动流程状态机
  emit started();
}

void FlowController::stop() {
  // TODO: 停止流程状态机
  emit stopped();
}

void FlowController::triggerOnce() {
  // TODO: 触发单次检测
  emit singleShotRequested();
}
