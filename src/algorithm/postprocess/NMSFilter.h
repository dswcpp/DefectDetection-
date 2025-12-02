#ifndef NMSFILTER_H
#define NMSFILTER_H

#include "../algorithm_global.h"
#include "../IDefectDetector.h"
#include <vector>

// 非极大值抑制过滤器：去除重叠的检测结果
class ALGORITHM_LIBRARY NMSFilter {
public:
  NMSFilter();
  ~NMSFilter() = default;

  // 设置 IoU 阈值 [0-1]
  void setIoUThreshold(double threshold);
  double iouThreshold() const { return m_iouThreshold; }

  // 设置置信度阈值
  void setConfidenceThreshold(double threshold);
  double confidenceThreshold() const { return m_confThreshold; }

  // 执行 NMS
  std::vector<DefectInfo> filter(const std::vector<DefectInfo>& defects);

  // 按类别分别执行 NMS
  std::vector<DefectInfo> filterByClass(const std::vector<DefectInfo>& defects);

  // 计算两个矩形的 IoU
  static double computeIoU(const cv::Rect& a, const cv::Rect& b);

private:
  double m_iouThreshold = 0.5;
  double m_confThreshold = 0.3;
};

// 缺陷合并器：合并相邻的同类缺陷
class ALGORITHM_LIBRARY DefectMerger {
public:
  DefectMerger();
  ~DefectMerger() = default;

  // 设置合并距离阈值（像素）
  void setDistanceThreshold(double threshold);
  double distanceThreshold() const { return m_distanceThreshold; }

  // 合并相邻缺陷
  std::vector<DefectInfo> merge(const std::vector<DefectInfo>& defects);

  // 按类别分别合并
  std::vector<DefectInfo> mergeByClass(const std::vector<DefectInfo>& defects);

private:
  double m_distanceThreshold = 20.0;

  // 计算两个矩形的距离
  double computeDistance(const cv::Rect& a, const cv::Rect& b);

  // 合并两个缺陷
  DefectInfo mergeDefects(const DefectInfo& a, const DefectInfo& b);
};

#endif // NMSFILTER_H
