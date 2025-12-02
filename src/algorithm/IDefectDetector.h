#ifndef IDEFECTDETECTOR_H
#define IDEFECTDETECTOR_H

#include "algorithm_global.h"
#include <QString>
#include <QVariantMap>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>

// 缺陷信息结构
struct DefectInfo {
  cv::Rect bbox;              // 边界框
  double confidence = 0.0;    // 置信度 [0, 1]
  double severity = 0.0;      // 严重度 [0, 1]
  int classId = 0;            // 类别ID
  QString className;          // 类别名称
  QString description;        // 描述信息
  std::vector<cv::Point> contour;  // 轮廓点（可选）
  
  // 额外属性（如长度、面积等）
  QVariantMap attributes;
};

// 检测结果结构
struct DetectionResult {
  bool success = false;           // 检测是否成功
  QString errorMessage;           // 错误信息
  std::vector<DefectInfo> defects; // 检测到的缺陷列表
  double processingTimeMs = 0.0;  // 处理耗时
  cv::Mat debugImage;             // 调试图像（可选）
};

// 检测器接口
class ALGORITHM_LIBRARY IDefectDetector {
public:
  virtual ~IDefectDetector() = default;

  // 检测器名称
  virtual QString name() const = 0;

  // 检测器类型标识
  virtual QString type() const = 0;

  // 初始化检测器
  virtual bool initialize() = 0;

  // 释放资源
  virtual void release() = 0;

  // 是否已初始化
  virtual bool isInitialized() const = 0;

  // 执行检测
  virtual DetectionResult detect(const cv::Mat& image) = 0;

  // 参数管理
  virtual void setParameters(const QVariantMap& params) = 0;
  virtual QVariantMap parameters() const = 0;

  // 启用/禁用
  virtual void setEnabled(bool enabled) = 0;
  virtual bool isEnabled() const = 0;

  // 设置置信度阈值
  virtual void setConfidenceThreshold(double threshold) = 0;
  virtual double confidenceThreshold() const = 0;
};

// 检测器智能指针类型
using DetectorPtr = std::shared_ptr<IDefectDetector>;

#endif // IDEFECTDETECTOR_H
