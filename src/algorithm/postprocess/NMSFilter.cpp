#include "NMSFilter.h"
#include <algorithm>
#include <map>
#include <cmath>

// ============================================================================
// NMSFilter
// ============================================================================

NMSFilter::NMSFilter() {}

void NMSFilter::setIoUThreshold(double threshold) {
  m_iouThreshold = qBound(0.0, threshold, 1.0);
}

void NMSFilter::setConfidenceThreshold(double threshold) {
  m_confThreshold = qBound(0.0, threshold, 1.0);
}

double NMSFilter::computeIoU(const cv::Rect& a, const cv::Rect& b) {
  int x1 = std::max(a.x, b.x);
  int y1 = std::max(a.y, b.y);
  int x2 = std::min(a.x + a.width, b.x + b.width);
  int y2 = std::min(a.y + a.height, b.y + b.height);

  if (x2 <= x1 || y2 <= y1) {
    return 0.0;
  }

  double intersection = (x2 - x1) * (y2 - y1);
  double areaA = a.width * a.height;
  double areaB = b.width * b.height;
  double unionArea = areaA + areaB - intersection;

  return intersection / unionArea;
}

std::vector<DefectInfo> NMSFilter::filter(const std::vector<DefectInfo>& defects) {
  if (defects.empty()) {
    return {};
  }

  // 按置信度降序排序
  std::vector<DefectInfo> sorted = defects;
  std::sort(sorted.begin(), sorted.end(), [](const DefectInfo& a, const DefectInfo& b) {
    return a.confidence > b.confidence;
  });

  std::vector<bool> suppressed(sorted.size(), false);
  std::vector<DefectInfo> result;

  for (size_t i = 0; i < sorted.size(); ++i) {
    if (suppressed[i]) continue;
    if (sorted[i].confidence < m_confThreshold) continue;

    result.push_back(sorted[i]);

    // 抑制与当前缺陷重叠度高的其他缺陷
    for (size_t j = i + 1; j < sorted.size(); ++j) {
      if (suppressed[j]) continue;

      double iou = computeIoU(sorted[i].bbox, sorted[j].bbox);
      if (iou > m_iouThreshold) {
        suppressed[j] = true;
      }
    }
  }

  return result;
}

std::vector<DefectInfo> NMSFilter::filterByClass(const std::vector<DefectInfo>& defects) {
  // 按类别分组
  std::map<int, std::vector<DefectInfo>> byClass;
  for (const auto& d : defects) {
    byClass[d.classId].push_back(d);
  }

  // 对每个类别分别执行 NMS
  std::vector<DefectInfo> result;
  for (auto& pair : byClass) {
    auto filtered = filter(pair.second);
    result.insert(result.end(), filtered.begin(), filtered.end());
  }

  return result;
}

// ============================================================================
// DefectMerger
// ============================================================================

DefectMerger::DefectMerger() {}

void DefectMerger::setDistanceThreshold(double threshold) {
  m_distanceThreshold = std::max(0.0, threshold);
}

double DefectMerger::computeDistance(const cv::Rect& a, const cv::Rect& b) {
  // 计算两个矩形边界之间的最小距离
  int left = b.x - (a.x + a.width);
  int right = a.x - (b.x + b.width);
  int top = b.y - (a.y + a.height);
  int bottom = a.y - (b.y + b.height);

  int hDist = std::max({left, right, 0});
  int vDist = std::max({top, bottom, 0});

  return std::sqrt(hDist * hDist + vDist * vDist);
}

DefectInfo DefectMerger::mergeDefects(const DefectInfo& a, const DefectInfo& b) {
  DefectInfo merged;
  
  // 合并边界框
  int x1 = std::min(a.bbox.x, b.bbox.x);
  int y1 = std::min(a.bbox.y, b.bbox.y);
  int x2 = std::max(a.bbox.x + a.bbox.width, b.bbox.x + b.bbox.width);
  int y2 = std::max(a.bbox.y + a.bbox.height, b.bbox.y + b.bbox.height);
  merged.bbox = cv::Rect(x1, y1, x2 - x1, y2 - y1);

  // 合并轮廓
  merged.contour = a.contour;
  merged.contour.insert(merged.contour.end(), b.contour.begin(), b.contour.end());

  // 取最大置信度和严重度
  merged.confidence = std::max(a.confidence, b.confidence);
  merged.severity = std::max(a.severity, b.severity);
  
  // 保留主要缺陷的类别
  if (a.confidence >= b.confidence) {
    merged.classId = a.classId;
    merged.className = a.className;
  } else {
    merged.classId = b.classId;
    merged.className = b.className;
  }

  // 合并属性
  merged.attributes = a.attributes;
  for (auto it = b.attributes.begin(); it != b.attributes.end(); ++it) {
    if (!merged.attributes.contains(it.key())) {
      merged.attributes[it.key()] = it.value();
    }
  }

  return merged;
}

std::vector<DefectInfo> DefectMerger::merge(const std::vector<DefectInfo>& defects) {
  if (defects.size() < 2) {
    return defects;
  }

  std::vector<DefectInfo> result = defects;
  bool merged = true;

  while (merged) {
    merged = false;
    
    for (size_t i = 0; i < result.size() && !merged; ++i) {
      for (size_t j = i + 1; j < result.size() && !merged; ++j) {
        double dist = computeDistance(result[i].bbox, result[j].bbox);
        
        if (dist <= m_distanceThreshold) {
          // 合并这两个缺陷
          DefectInfo mergedDefect = mergeDefects(result[i], result[j]);
          result.erase(result.begin() + j);
          result.erase(result.begin() + i);
          result.push_back(mergedDefect);
          merged = true;
        }
      }
    }
  }

  return result;
}

std::vector<DefectInfo> DefectMerger::mergeByClass(const std::vector<DefectInfo>& defects) {
  // 按类别分组
  std::map<int, std::vector<DefectInfo>> byClass;
  for (const auto& d : defects) {
    byClass[d.classId].push_back(d);
  }

  // 对每个类别分别合并
  std::vector<DefectInfo> result;
  for (auto& pair : byClass) {
    auto merged = merge(pair.second);
    result.insert(result.end(), merged.begin(), merged.end());
  }

  return result;
}
