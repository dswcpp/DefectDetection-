#include "DetectPipeline.h"
#include "hal/camera/CameraFactory.h"
#include "hal/camera/ICamera.h"
#include "DetectorManager.h"
#include "preprocess/ImagePreprocessor.h"
#include "postprocess/NMSFilter.h"
#include "scoring/DefectScorer.h"
#include "common/Logger.h"

#include <QMetaType>
#include <QTimer>
#include <QDateTime>
#include <QFileInfo>
#include <QtConcurrent/QtConcurrent>

DetectPipeline::DetectPipeline(QObject* parent) : QObject(parent) {
  qRegisterMetaType<DetectResult>("DetectResult");
  qRegisterMetaType<cv::Mat>("cv::Mat");

  m_captureTimer = new QTimer(this);
  connect(m_captureTimer, &QTimer::timeout, this, &DetectPipeline::onCaptureTimeout);

  // 连接异步检测完成信号
  connect(&m_detectWatcher, &QFutureWatcher<DetectResult>::finished,
          this, &DetectPipeline::onDetectionFinished);

  initDetectors();
}

DetectPipeline::~DetectPipeline() {
  stop();
  
  // 等待异步检测完成
  if (m_detectWatcher.isRunning()) {
    m_detectWatcher.waitForFinished();
  }
  
  if (m_detectorManager) {
    m_detectorManager->release();
  }
}

bool DetectPipeline::initDetectors() {
  m_detectorManager = std::make_unique<DetectorManager>();
  if (!m_detectorManager->initialize()) {
    LOG_WARN("Failed to initialize DetectorManager, using simulated detection");
    m_useRealDetection = false;
  }

  m_preprocessor = std::make_unique<ImagePreprocessor>();
  m_preprocessor->setDenoiseStrength(20);

  m_nmsFilter = std::make_unique<NMSFilter>();
  m_nmsFilter->setIoUThreshold(0.5);
  m_nmsFilter->setConfidenceThreshold(0.3);

  m_scorer = std::make_unique<DefectScorer>();

  LOG_INFO("DetectPipeline: Detection components initialized, useRealDetection={}", m_useRealDetection);
  return true;
}

void DetectPipeline::setImageDir(const QString& dir) {
  m_imageDir = dir;
}

void DetectPipeline::setCaptureInterval(int ms) {
  m_captureIntervalMs = ms;
  if (m_captureTimer->isActive()) {
    m_captureTimer->setInterval(ms);
  }
}

bool DetectPipeline::isRunning() const {
  return m_running;
}

void DetectPipeline::start() {
  if (m_running) {
    LOG_WARN("DetectPipeline already running");
    return;
  }

  if (!initCamera()) {
    emit error("pipeline", "Failed to init camera");
    return;
  }

  m_running = true;
  m_captureTimer->start(m_captureIntervalMs);
  LOG_INFO("DetectPipeline started, interval={}ms", m_captureIntervalMs);
  emit started();
}

void DetectPipeline::stop() {
  if (!m_running) {
    return;
  }

  m_captureTimer->stop();
  
  // 等待异步检测完成
  if (m_detectWatcher.isRunning()) {
    m_detectWatcher.waitForFinished();
  }
  m_detecting.store(false);
  
  releaseCamera();
  m_running = false;
  LOG_INFO("DetectPipeline stopped");
  emit stopped();
}

void DetectPipeline::singleShot() {
  if (m_running) {
    LOG_WARN("Cannot single shot while running");
    return;
  }

  if (!m_camera) {
    if (!initCamera()) {
      emit error("pipeline", "Failed to init camera for single shot");
      return;
    }
  }

  cv::Mat frame;
  if (m_camera->grab(frame) && !frame.empty()) {
    m_currentImagePath = m_camera->currentImagePath();
    emit frameReady(frame);
    DetectResult result = runDetection(frame);
    emit resultReady(result);
  } else {
    emit error("camera", "Failed to grab frame");
  }
}

void DetectPipeline::onCaptureTimeout() {
  if (!m_camera) {
    return;
  }

  // 如果上一次检测还未完成，跳过本次
  if (m_detecting.load()) {
    return;
  }

  m_detecting.store(true);
  
  // 整个抓取+检测流程都在后台线程执行
  QFuture<DetectResult> future = QtConcurrent::run([this]() {
    cv::Mat frame;
    if (!m_camera || !m_camera->grab(frame) || frame.empty()) {
      DetectResult result;
      result.isOK = true;
      return result;
    }
    
    m_currentImagePath = m_camera->currentImagePath();
    
    // 缩小图片用于显示，避免大图阻塞UI
    cv::Mat displayFrame = frame;
    const int MAX_DISPLAY = 1280;
    if (frame.cols > MAX_DISPLAY || frame.rows > MAX_DISPLAY) {
      double scale = std::min(static_cast<double>(MAX_DISPLAY) / frame.cols,
                              static_cast<double>(MAX_DISPLAY) / frame.rows);
      cv::resize(frame, displayFrame, cv::Size(), scale, scale, cv::INTER_AREA);
    }
    m_pendingFrame = displayFrame;
    
    return runDetection(frame);
  });
  m_detectWatcher.setFuture(future);
}

void DetectPipeline::onDetectionFinished() {
  m_detecting.store(false);
  
  if (m_detectWatcher.future().isValid()) {
    // 发射缩小后的图片用于显示
    if (!m_pendingFrame.empty()) {
      emit frameReady(m_pendingFrame);
    }
    
    DetectResult result = m_detectWatcher.result();
    emit resultReady(result);
  }
}

bool DetectPipeline::initCamera() {
  m_camera = CameraFactory::create("file");
  if (!m_camera) {
    LOG_ERROR("Failed to create FileCamera");
    return false;
  }

  CameraConfig config;
  config.type = "file";
  config.ip = m_imageDir;  // 复用 ip 字段存储图片目录

  if (!m_camera->open(config)) {
    LOG_ERROR("Failed to open camera with dir: {}", QFileInfo(m_imageDir).absoluteFilePath().toStdString());
    m_camera.reset();
    return false;
  }

  return true;
}

void DetectPipeline::releaseCamera() {
  if (m_camera) {
    m_camera->close();
    m_camera.reset();
  }
}

DetectResult DetectPipeline::runDetection(const cv::Mat& frame) {
  // 开始计时
  m_detectTimer.restart();

  DetectResult result;
  result.timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

  if (m_useRealDetection && m_detectorManager) {
    // 使用真实检测
    
    // 0. 缩放大图以避免处理过慢
    cv::Mat resized = frame;
    const int MAX_DIM = 1920;  // 最大边长限制
    if (frame.cols > MAX_DIM || frame.rows > MAX_DIM) {
      double scale = std::min(static_cast<double>(MAX_DIM) / frame.cols,
                              static_cast<double>(MAX_DIM) / frame.rows);
      cv::resize(frame, resized, cv::Size(), scale, scale, cv::INTER_AREA);
      LOG_DEBUG("DetectPipeline: Resized image from {}x{} to {}x{}",
                frame.cols, frame.rows, resized.cols, resized.rows);
    }
    
    // 1. 预处理
    cv::Mat processed = m_preprocessor ? m_preprocessor->process(resized) : resized;

    // 2. 执行检测
    auto detectResult = m_detectorManager->detectAll(processed);

    // 3. NMS 去重
    std::vector<DefectInfo> filteredDefects = detectResult.allDefects;
    if (m_nmsFilter && !filteredDefects.empty()) {
      filteredDefects = m_nmsFilter->filterByClass(filteredDefects);
    }

    // 3.5 限制缺陷数量，避免过多缺陷导致性能问题
    const size_t MAX_DEFECTS = 50;
    if (filteredDefects.size() > MAX_DEFECTS) {
      // 按置信度排序，保留最高的
      std::sort(filteredDefects.begin(), filteredDefects.end(),
                [](const DefectInfo& a, const DefectInfo& b) {
                  return a.confidence > b.confidence;
                });
      filteredDefects.resize(MAX_DEFECTS);
      LOG_WARN("DetectPipeline: Too many defects ({}), keeping top {}", 
               detectResult.allDefects.size(), MAX_DEFECTS);
    }

    // 4. 评分
    ScoringResult scoreResult;
    if (m_scorer) {
      scoreResult = m_scorer->score(filteredDefects);
    }

    // 5. 转换结果
    result.isOK = filteredDefects.empty();
    
    if (!filteredDefects.empty()) {
      // 使用评分结果确定严重度等级
      switch (scoreResult.grade) {
        case SeverityGrade::OK:
          result.level = SeverityLevel::OK;
          break;
        case SeverityGrade::Minor:
          result.level = SeverityLevel::Minor;
          break;
        case SeverityGrade::Major:
          result.level = SeverityLevel::Major;
          break;
        case SeverityGrade::Critical:
          result.level = SeverityLevel::Critical;
          break;
      }

      // 找最高严重度的缺陷作为主要缺陷类型
      double maxSeverity = 0;
      for (const auto& defect : filteredDefects) {
        if (defect.severity > maxSeverity) {
          maxSeverity = defect.severity;
          result.defectType = defect.className;
        }

        // 转换为 DefectRegion
        DefectRegion region;
        region.bbox = defect.bbox;
        region.confidence = defect.confidence;
        region.classId = defect.classId;
        result.defects.push_back(region);
        result.regions.push_back(defect.bbox);
      }

      result.severity = maxSeverity;
      result.confidence = filteredDefects[0].confidence;
    } else {
      result.level = SeverityLevel::OK;
      result.severity = 0.0;
      result.confidence = 1.0;
    }

  } else {
    // 使用模拟检测（后备方案）
    result.isOK = (rand() % 10) > 3;  // 70% OK率

    if (!result.isOK) {
      QStringList defectTypes = {"Scratch", "Crack", "Foreign", "Dimension"};
      result.defectType = defectTypes[rand() % defectTypes.size()];

      int severityIdx = rand() % 3;
      if (severityIdx == 0) {
        result.level = SeverityLevel::Minor;
        result.severity = 0.3;
      } else if (severityIdx == 1) {
        result.level = SeverityLevel::Major;
        result.severity = 0.6;
      } else {
        result.level = SeverityLevel::Critical;
        result.severity = 0.9;
      }

      int numDefects = 1 + rand() % 3;
      for (int i = 0; i < numDefects; ++i) {
        DefectRegion defect;
        defect.bbox = cv::Rect(
          rand() % std::max(1, frame.cols - 100),
          rand() % std::max(1, frame.rows - 100),
          50 + rand() % 100,
          50 + rand() % 100
        );
        defect.confidence = 0.75 + (rand() % 25) / 100.0;
        defect.classId = rand() % 4;
        result.defects.push_back(defect);
        result.regions.push_back(defect.bbox);
      }
      result.confidence = 0.75 + (rand() % 25) / 100.0;
    } else {
      result.level = SeverityLevel::OK;
      result.severity = 0.0;
      result.confidence = 1.0;
    }
  }

  // 记录检测耗时
  m_detectTimer.stop();
  double detectTimeMs = m_detectTimer.elapsedMs();
  m_detectStats.record(detectTimeMs);
  result.cycleTimeMs = static_cast<int>(detectTimeMs);

  LOG_DEBUG("Detection completed in {:.2f}ms (avg: {:.2f}ms), defects: {}",
            detectTimeMs, m_detectStats.avg(), result.defects.size());

  return result;
}
