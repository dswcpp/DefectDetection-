#ifndef DETECTPIPELINE_H
#define DETECTPIPELINE_H

#include <QObject>
#include <memory>
#include "Types.h"
#include "ui_global.h"
#include "opencv2/opencv.hpp"

class QTimer;
class ICamera;
struct CameraConfig;

class UI_LIBRARY DetectPipeline : public QObject {
  Q_OBJECT
public:
  explicit DetectPipeline(QObject* parent = nullptr);
  ~DetectPipeline();

  void setImageDir(const QString& dir);
  void setCaptureInterval(int ms);
  bool isRunning() const;

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

private slots:
  void onCaptureTimeout();

private:
  bool initCamera();
  void releaseCamera();
  DetectResult runDetection(const cv::Mat& frame);

  std::unique_ptr<ICamera> m_camera;
  QTimer* m_captureTimer = nullptr;
  QString m_imageDir;
  int m_captureIntervalMs = 500;
  bool m_running = false;
};

#endif // DETECTPIPELINE_H
