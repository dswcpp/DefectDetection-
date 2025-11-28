#ifndef DETECTPIPELINE_H
#define DETECTPIPELINE_H

#include <QObject>
#include "Types.h"
#include "opencv2/opencv.hpp"

// 这个类专心做业务编排，别把 UI/算法细节塞进来
class DetectPipeline : public QObject {
  Q_OBJECT
public:
  explicit DetectPipeline(QObject* parent = nullptr);

public slots:
  void start();
  void stop();
  void singleShot();

signals:
  void resultReady(const DetectResult& result);
  void frameReady(const cv::Mat& frame);
  void error(const QString& module, const QString& message);

  void started();
  void stopped();

private:
  // TODO: 初始化相机/检测器
  bool initPipeline();
  // TODO: 释放资源
  void teardown();
};

#endif // DETECTPIPELINE_H
