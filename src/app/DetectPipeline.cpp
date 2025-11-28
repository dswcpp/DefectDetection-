#include "DetectPipeline.h"

#include <QMetaType>

DetectPipeline::DetectPipeline(QObject* parent) : QObject(parent) {
  qRegisterMetaType<DetectResult>("DetectResult");
  qRegisterMetaType<cv::Mat>("cv::Mat");
}

void DetectPipeline::start() {
  // TODO: 这里接入相机/算法，跑起来后 emit resultReady/frameReady
  if (!initPipeline()) {
    emit error("pipeline", "init failed");
    return;
  }
  emit started();
}

void DetectPipeline::stop() {
  // TODO: 停止采集/推理，清理资源
  teardown();
  emit stopped();
}

void DetectPipeline::singleShot() {
  // TODO: 跑一次单帧检测
}

bool DetectPipeline::initPipeline() {
  // TODO: 初始化相机/检测器/队列/线程
  return true;
}

void DetectPipeline::teardown() {
  // TODO: 释放相机/检测器/线程
}
