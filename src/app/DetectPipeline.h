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
};

#endif // DETECTPIPELINE_H
