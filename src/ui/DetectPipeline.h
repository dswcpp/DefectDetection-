#ifndef DETECTPIPELINE_H
#define DETECTPIPELINE_H

#include <QObject>
#include <memory>
#include "Types.h"
#include "Timer.h"
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
  QString currentImagePath() const { return m_currentImagePath; }

  // 性能统计
  const PerfStats& detectStats() const { return m_detectStats; }
  double lastDetectTimeMs() const { return m_detectStats.last(); }
  double avgDetectTimeMs() const { return m_detectStats.avg(); }

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
  QString m_currentImagePath;
  int m_captureIntervalMs = 500;
  bool m_running = false;

  // 性能统计
  PerfStats m_detectStats{"Detection"};
  Timer m_detectTimer;
};

#endif // DETECTPIPELINE_H
