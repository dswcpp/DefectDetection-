/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * BaseDetector.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：检测器基类实现定义
 * 描述：检测器公共实现基类，提供参数管理、置信度过滤、结果构造等
 *       通用功能，具体检测器继承此类
 *
 * 当前版本：1.0
 */

#ifndef BASEDETECTOR_H
#define BASEDETECTOR_H

#include "IDefectDetector.h"

// 检测器基类，提供通用实现
class ALGORITHM_LIBRARY BaseDetector : public IDefectDetector {
public:
  BaseDetector() = default;
  ~BaseDetector() override = default;

  // IDefectDetector 接口实现
  bool isInitialized() const override { return m_initialized; }
  
  void setEnabled(bool enabled) override { m_enabled = enabled; }
  bool isEnabled() const override { return m_enabled; }

  void setConfidenceThreshold(double threshold) override { 
    m_confidenceThreshold = qBound(0.0, threshold, 1.0); 
  }
  double confidenceThreshold() const override { return m_confidenceThreshold; }

  void setParameters(const QVariantMap& params) override { m_params = params; }
  QVariantMap parameters() const override { return m_params; }

protected:
  // 辅助方法：创建成功结果
  DetectionResult makeSuccessResult(const std::vector<DefectInfo>& defects, double timeMs) {
    DetectionResult result;
    result.success = true;
    result.defects = defects;
    result.processingTimeMs = timeMs;
    return result;
  }

  // 辅助方法：创建失败结果
  DetectionResult makeErrorResult(const QString& error) {
    DetectionResult result;
    result.success = false;
    result.errorMessage = error;
    return result;
  }

  // 辅助方法：过滤低置信度缺陷
  std::vector<DefectInfo> filterByConfidence(const std::vector<DefectInfo>& defects) {
    std::vector<DefectInfo> filtered;
    for (const auto& d : defects) {
      if (d.confidence >= m_confidenceThreshold) {
        filtered.push_back(d);
      }
    }
    return filtered;
  }

  // 辅助方法：获取参数值
  template<typename T>
  T getParam(const QString& key, const T& defaultValue) const {
    return m_params.value(key, defaultValue).template value<T>();
  }

  bool m_initialized = false;
  bool m_enabled = true;
  double m_confidenceThreshold = 0.5;
  QVariantMap m_params;
};

#endif // BASEDETECTOR_H
