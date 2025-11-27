#include "DetectPipeline.h"

#include <QMetaType>

DetectPipeline::DetectPipeline(QObject* parent) : QObject(parent) {
  qRegisterMetaType<DetectResult>("DetectResult");
  qRegisterMetaType<cv::Mat>("cv::Mat");
}

void DetectPipeline::start() {
  // TODO: 这里接入相机/算法，跑起来后 emit resultReady/frameReady
}

void DetectPipeline::stop() {
  // TODO: 停止采集/推理，清理资源
}

void DetectPipeline::singleShot() {
  // TODO: 跑一次单帧检测
}
