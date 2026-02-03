/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * DetectPipeline.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：检测流水线模块接口定义
 * 描述：检测流程控制器，协调相机采集、图像预处理、缺陷检测、
 *       结果评分等步骤，支持连续检测和单次检测模式
 *
 * 当前版本：1.0
 */

#ifndef DETECTPIPELINE_H
#define DETECTPIPELINE_H

#include <QObject>
#include <QFutureWatcher>
#include <QMutex>
#include <memory>
#include <atomic>
#include "Types.h"
#include "Timer.h"
#include "ui_global.h"
#include <opencv2/core.hpp>  // 只需要 cv::Mat

class QTimer;
class ICamera;
class DetectorManager;
class ImagePreprocessor;
class NMSFilter;
class DefectScorer;
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
  void onDetectionFinished();

private:
  bool initCamera();
  void releaseCamera();
  bool initDetectors();
  DetectResult runDetection(const cv::Mat& frame);

  std::unique_ptr<ICamera> m_camera;
  std::unique_ptr<DetectorManager> m_detectorManager;
  std::unique_ptr<ImagePreprocessor> m_preprocessor;
  std::unique_ptr<NMSFilter> m_nmsFilter;
  std::unique_ptr<DefectScorer> m_scorer;

  QTimer* m_captureTimer = nullptr;
  QString m_imageDir;
  QString m_currentImagePath;
  int m_captureIntervalMs = 500;
  bool m_running = false;
  bool m_useRealDetection = true;

  // 异步检测
  QFutureWatcher<DetectResult> m_detectWatcher;
  std::atomic<bool> m_detecting{false};
  QMutex m_detectMutex;
  cv::Mat m_pendingFrame;

  // 性能统计
  PerfStats m_detectStats{"Detection"};
  Timer m_detectTimer;
};

#endif // DETECTPIPELINE_H
