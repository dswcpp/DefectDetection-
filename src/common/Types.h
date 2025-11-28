#ifndef TYPES_H
#define TYPES_H
#include <QMetaType>
#include <QString>
#include <vector>
#include "opencv2/opencv.hpp"
// src/common/Types.h

struct DefectRegion {
  cv::Rect bbox;           // 缺陷边界框
  double confidence;       // 置信度
  int classId;             // 缺陷类别ID
};

enum class SeverityLevel {
  OK,
  Minor,
  Major,
  Critical
};

enum class AlarmLevel {
  Info,       // 信息
  Warning,    // 警告
  Error,      // 错误
  Critical    // 严重
};

struct DetectResult {
  QString defectType;                    // 缺陷类型
  std::vector<cv::Rect> regions;         // 检测到的缺陷框列表
  std::vector<DefectRegion> defects;     // 带置信度的缺陷列表
  double severity = 0.0;                 // 严重度分数
  SeverityLevel level = SeverityLevel::OK; // 严重度等级
  double confidence = 0.0;               // 置信度 0-1
  bool isOK = true;                      // 是否合格
  int cycleTimeMs = 0;                   // 单次检测耗时
  qint64 timestamp = 0;                  // 检测时间戳
  QString errorMsg;                      // 错误信息
};

Q_DECLARE_METATYPE(DetectResult);

#endif // TYPES_H
