#ifndef DETECTORMANAGER_H
#define DETECTORMANAGER_H

#include "algorithm_global.h"
#include "IDefectDetector.h"
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QMutex>
#include <vector>
#include <map>

// 检测管理器：管理多个检测器的执行
class ALGORITHM_LIBRARY DetectorManager : public QObject {
  Q_OBJECT
public:
  explicit DetectorManager(QObject* parent = nullptr);
  ~DetectorManager() override;

  // 初始化所有检测器
  bool initialize();

  // 释放所有检测器
  void release();

  // 添加/移除检测器
  void addDetector(const QString& name, DetectorPtr detector);
  void removeDetector(const QString& name);
  DetectorPtr getDetector(const QString& name) const;
  QStringList detectorNames() const;

  // 启用/禁用检测器
  void setDetectorEnabled(const QString& name, bool enabled);
  bool isDetectorEnabled(const QString& name) const;

  // 设置检测器参数
  void setDetectorParameters(const QString& name, const QVariantMap& params);
  QVariantMap getDetectorParameters(const QString& name) const;

  // 从配置加载参数
  void loadFromConfig();

  // 保存参数到配置
  void saveToConfig();

  // 执行所有启用的检测器
  struct CombinedResult {
    bool success = true;
    QString errorMessage;
    std::vector<DefectInfo> allDefects;
    double totalTimeMs = 0.0;
    std::map<QString, DetectionResult> detectorResults;
  };
  
  // 串行执行所有检测器
  CombinedResult detectAll(const cv::Mat& image);
  
  // 并行执行所有检测器（多线程）
  CombinedResult detectAllParallel(const cv::Mat& image);

  // 执行单个检测器
  DetectionResult detectWith(const QString& name, const cv::Mat& image);
  
  // 设置是否使用并行检测
  void setParallelEnabled(bool enabled) { m_parallelEnabled = enabled; }
  bool isParallelEnabled() const { return m_parallelEnabled; }

signals:
  void detectorAdded(const QString& name);
  void detectorRemoved(const QString& name);
  void detectionStarted();
  void detectionFinished(const CombinedResult& result);
  void detectorResult(const QString& name, const DetectionResult& result);

private:
  void registerBuiltinDetectors();

  std::map<QString, DetectorPtr> m_detectors;
  bool m_initialized = false;
  bool m_parallelEnabled = true;  // 默认启用并行检测
  mutable QMutex m_resultMutex;   // 保护并行结果合并
};

#endif // DETECTORMANAGER_H
