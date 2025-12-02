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

DetectPipeline::DetectPipeline(QObject* parent) : QObject(parent) {
  qDebug() << "[DEBUG] DetectPipeline: Constructor start";
  qRegisterMetaType<DetectResult>("DetectResult");
  qRegisterMetaType<cv::Mat>("cv::Mat");

  m_captureTimer = new QTimer(this);
  connect(m_captureTimer, &QTimer::timeout, this, &DetectPipeline::onCaptureTimeout);

  qDebug() << "[DEBUG] DetectPipeline: Calling initDetectors...";
  // 初始化检测组件
  initDetectors();
  qDebug() << "[DEBUG] DetectPipeline: Constructor done";
}

DetectPipeline::~DetectPipeline() {
  stop();
  if (m_detectorManager) {
    m_detectorManager->release();
  }
}

bool DetectPipeline::initDetectors() {
  qDebug() << "[DEBUG] DetectPipeline::initDetectors: Creating DetectorManager...";
  // 初始化检测器管理器
  m_detectorManager = std::make_unique<DetectorManager>();
  qDebug() << "[DEBUG] DetectPipeline::initDetectors: DetectorManager created, initializing...";
  if (!m_detectorManager->initialize()) {
    LOG_WARN("Failed to initialize DetectorManager, using simulated detection");
    m_useRealDetection = false;
  }
  qDebug() << "[DEBUG] DetectPipeline::initDetectors: DetectorManager initialized";

  qDebug() << "[DEBUG] DetectPipeline::initDetectors: Creating ImagePreprocessor...";
  // 初始化预处理器
  m_preprocessor = std::make_unique<ImagePreprocessor>();
  m_preprocessor->setDenoiseStrength(20);

  qDebug() << "[DEBUG] DetectPipeline::initDetectors: Creating NMSFilter...";
  // 初始化 NMS 过滤器
  m_nmsFilter = std::make_unique<NMSFilter>();
  m_nmsFilter->setIoUThreshold(0.5);
  m_nmsFilter->setConfidenceThreshold(0.3);

  qDebug() << "[DEBUG] DetectPipeline::initDetectors: Creating DefectScorer...";
  // 初始化评分器
  m_scorer = std::make_unique<DefectScorer>();

  qDebug() << "[DEBUG] DetectPipeline::initDetectors: All components initialized";
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

  cv::Mat frame;
  if (m_camera->grab(frame) && !frame.empty()) {
    m_currentImagePath = m_camera->currentImagePath();
    emit frameReady(frame);
    DetectResult result = runDetection(frame);
    emit resultReady(result);
  } else {
    LOG_WARN("Failed to grab frame, stopping...");
    stop();
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
    
    // 1. 预处理
    cv::Mat processed = m_preprocessor ? m_preprocessor->process(frame) : frame;

    // 2. 执行检测
    auto detectResult = m_detectorManager->detectAll(processed);

    // 3. NMS 去重
    std::vector<DefectInfo> filteredDefects = detectResult.allDefects;
    if (m_nmsFilter && !filteredDefects.empty()) {
      filteredDefects = m_nmsFilter->filterByClass(filteredDefects);
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
