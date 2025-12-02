#include "DetectorManager.h"
#include "DetectorFactory.h"
#include "config/ConfigManager.h"
#include "Logger.h"
#include <QElapsedTimer>

DetectorManager::DetectorManager(QObject* parent) : QObject(parent) {
}

DetectorManager::~DetectorManager() {
  release();
}

bool DetectorManager::initialize() {
  if (m_initialized) {
    return true;
  }

  LOG_INFO("DetectorManager: Initializing...");

  // 注册内置检测器
  registerBuiltinDetectors();

  // 从配置加载参数
  loadFromConfig();

  // 初始化所有检测器
  for (auto& pair : m_detectors) {
    if (!pair.second->initialize()) {
      LOG_WARN("DetectorManager: Failed to initialize detector: {}", pair.first.toStdString());
    }
  }

  m_initialized = true;
  LOG_INFO("DetectorManager: Initialized with {} detectors", m_detectors.size());
  return true;
}

void DetectorManager::release() {
  for (auto& pair : m_detectors) {
    pair.second->release();
  }
  m_detectors.clear();
  m_initialized = false;
  LOG_INFO("DetectorManager: Released");
}

void DetectorManager::registerBuiltinDetectors() {
  auto& factory = DetectorFactory::instance();

  // 创建并添加内置检测器
  QStringList builtinTypes = {
    DetectorFactory::TYPE_SCRATCH,
    DetectorFactory::TYPE_CRACK,
    DetectorFactory::TYPE_FOREIGN,
    DetectorFactory::TYPE_DIMENSION
  };

  for (const auto& type : builtinTypes) {
    if (factory.isRegistered(type)) {
      auto detector = factory.create(type);
      if (detector) {
        addDetector(type, detector);
      }
    }
  }
}

void DetectorManager::addDetector(const QString& name, DetectorPtr detector) {
  if (!detector) {
    return;
  }
  m_detectors[name] = detector;
  emit detectorAdded(name);
  LOG_DEBUG("DetectorManager: Added detector: {}", name.toStdString());
}

void DetectorManager::removeDetector(const QString& name) {
  auto it = m_detectors.find(name);
  if (it != m_detectors.end()) {
    it->second->release();
    m_detectors.erase(it);
    emit detectorRemoved(name);
    LOG_DEBUG("DetectorManager: Removed detector: {}", name.toStdString());
  }
}

DetectorPtr DetectorManager::getDetector(const QString& name) const {
  auto it = m_detectors.find(name);
  if (it != m_detectors.end()) {
    return it->second;
  }
  return nullptr;
}

QStringList DetectorManager::detectorNames() const {
  QStringList names;
  for (const auto& pair : m_detectors) {
    names.append(pair.first);
  }
  return names;
}

void DetectorManager::setDetectorEnabled(const QString& name, bool enabled) {
  auto detector = getDetector(name);
  if (detector) {
    detector->setEnabled(enabled);
  }
}

bool DetectorManager::isDetectorEnabled(const QString& name) const {
  auto detector = getDetector(name);
  return detector ? detector->isEnabled() : false;
}

void DetectorManager::setDetectorParameters(const QString& name, const QVariantMap& params) {
  auto detector = getDetector(name);
  if (detector) {
    detector->setParameters(params);
  }
}

QVariantMap DetectorManager::getDetectorParameters(const QString& name) const {
  auto detector = getDetector(name);
  return detector ? detector->parameters() : QVariantMap();
}

void DetectorManager::loadFromConfig() {
  auto detectorsConfig = gConfig.detectorsConfig();

  // 划痕检测器
  if (auto d = getDetector(DetectorFactory::TYPE_SCRATCH)) {
    auto& cfg = detectorsConfig.scratch;
    d->setEnabled(cfg.enabled);
    QVariantMap params;
    params["sensitivity"] = cfg.sensitivity;
    params["minLength"] = cfg.minLength;
    params["maxWidth"] = cfg.maxWidth;
    params["contrastThreshold"] = cfg.contrastThreshold;
    d->setParameters(params);
  }

  // 裂纹检测器
  if (auto d = getDetector(DetectorFactory::TYPE_CRACK)) {
    auto& cfg = detectorsConfig.crack;
    d->setEnabled(cfg.enabled);
    QVariantMap params;
    params["threshold"] = cfg.threshold;
    params["minArea"] = cfg.minArea;
    params["morphKernelSize"] = cfg.morphKernelSize;
    params["binaryThreshold"] = cfg.binaryThreshold;
    d->setParameters(params);
  }

  // 异物检测器
  if (auto d = getDetector(DetectorFactory::TYPE_FOREIGN)) {
    auto& cfg = detectorsConfig.foreign;
    d->setEnabled(cfg.enabled);
    QVariantMap params;
    params["minArea"] = cfg.minArea;
    params["contrast"] = cfg.contrast;
    params["colorThreshold"] = cfg.colorThreshold;
    d->setParameters(params);
  }

  // 尺寸检测器
  if (auto d = getDetector(DetectorFactory::TYPE_DIMENSION)) {
    auto& cfg = detectorsConfig.dimension;
    d->setEnabled(cfg.enabled);
    QVariantMap params;
    params["tolerance"] = cfg.tolerance;
    params["calibration"] = cfg.calibration;
    params["targetWidth"] = cfg.targetWidth;
    params["targetHeight"] = cfg.targetHeight;
    d->setParameters(params);
  }

  LOG_DEBUG("DetectorManager: Loaded parameters from config");
}

void DetectorManager::saveToConfig() {
  DetectorsConfig config;

  // 划痕检测器
  if (auto d = getDetector(DetectorFactory::TYPE_SCRATCH)) {
    auto params = d->parameters();
    config.scratch.enabled = d->isEnabled();
    config.scratch.sensitivity = params.value("sensitivity", 75).toInt();
    config.scratch.minLength = params.value("minLength", 10).toInt();
    config.scratch.maxWidth = params.value("maxWidth", 5).toInt();
    config.scratch.contrastThreshold = params.value("contrastThreshold", 30).toInt();
  }

  // 裂纹检测器
  if (auto d = getDetector(DetectorFactory::TYPE_CRACK)) {
    auto params = d->parameters();
    config.crack.enabled = d->isEnabled();
    config.crack.threshold = params.value("threshold", 80).toInt();
    config.crack.minArea = params.value("minArea", 20).toInt();
    config.crack.morphKernelSize = params.value("morphKernelSize", 3).toInt();
    config.crack.binaryThreshold = params.value("binaryThreshold", 128).toInt();
  }

  // 异物检测器
  if (auto d = getDetector(DetectorFactory::TYPE_FOREIGN)) {
    auto params = d->parameters();
    config.foreign.enabled = d->isEnabled();
    config.foreign.minArea = params.value("minArea", 5).toInt();
    config.foreign.contrast = params.value("contrast", 0.3).toDouble();
    config.foreign.colorThreshold = params.value("colorThreshold", 50).toInt();
  }

  // 尺寸检测器
  if (auto d = getDetector(DetectorFactory::TYPE_DIMENSION)) {
    auto params = d->parameters();
    config.dimension.enabled = d->isEnabled();
    config.dimension.tolerance = params.value("tolerance", 0.5).toDouble();
    config.dimension.calibration = params.value("calibration", 0.1).toDouble();
    config.dimension.targetWidth = params.value("targetWidth", 100.0).toDouble();
    config.dimension.targetHeight = params.value("targetHeight", 100.0).toDouble();
  }

  gConfig.setDetectorsConfig(config, true);
  LOG_DEBUG("DetectorManager: Saved parameters to config");
}

DetectorManager::CombinedResult DetectorManager::detectAll(const cv::Mat& image) {
  CombinedResult result;
  QElapsedTimer timer;
  timer.start();

  emit detectionStarted();

  if (image.empty()) {
    result.success = false;
    result.errorMessage = "Empty input image";
    emit detectionFinished(result);
    return result;
  }

  // 执行所有启用的检测器
  for (auto& pair : m_detectors) {
    if (!pair.second->isEnabled()) {
      continue;
    }

    DetectionResult detResult = pair.second->detect(image);
    result.detectorResults[pair.first] = detResult;

    if (detResult.success) {
      // 合并检测结果
      for (auto& defect : detResult.defects) {
        result.allDefects.push_back(defect);
      }
    } else {
      LOG_WARN("DetectorManager: Detector {} failed: {}", 
               pair.first.toStdString(), detResult.errorMessage.toStdString());
    }

    emit detectorResult(pair.first, detResult);
  }

  result.totalTimeMs = timer.elapsed();
  emit detectionFinished(result);

  LOG_DEBUG("DetectorManager: Detection completed in {:.2f}ms, found {} defects",
            result.totalTimeMs, result.allDefects.size());

  return result;
}

DetectionResult DetectorManager::detectWith(const QString& name, const cv::Mat& image) {
  auto detector = getDetector(name);
  if (!detector) {
    DetectionResult result;
    result.success = false;
    result.errorMessage = QString("Detector not found: %1").arg(name);
    return result;
  }

  return detector->detect(image);
}
