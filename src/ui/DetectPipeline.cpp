#include "DetectPipeline.h"
#include "hal/camera/CameraFactory.h"
#include "hal/camera/ICamera.h"
#include "common/Logger.h"

#include <QMetaType>
#include <QTimer>
#include <QDateTime>

DetectPipeline::DetectPipeline(QObject* parent) : QObject(parent) {
  qRegisterMetaType<DetectResult>("DetectResult");
  qRegisterMetaType<cv::Mat>("cv::Mat");

  m_captureTimer = new QTimer(this);
  connect(m_captureTimer, &QTimer::timeout, this, &DetectPipeline::onCaptureTimeout);
}

DetectPipeline::~DetectPipeline() {
  stop();
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
    LOG_ERROR("Failed to open camera with dir: {}", m_imageDir);
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
  // TODO: 接入真实的检测算法，目前用模拟数据
  DetectResult result;
  result.timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
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

  return result;
}
