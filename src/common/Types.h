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

struct DetectResult {
  QString defectType;                    // 缺陷类型（如 "scratch", "crack"）
  std::vector<cv::Rect> regions;         // 检测到的缺陷区域列表
  std::vector<DefectRegion> defects;     // 带置信度的缺陷列表（DNN用）
  double severity;                       // 严重度评分
  SeverityLevel level;                   // 严重度等级（OK/NG/Warning等）
  bool isOK;                             // 是否合格
  qint64 timestamp;                      // 检测时间戳
  QString errorMsg;                      // 错误信息（如有）
};

Q_DECLARE_METATYPE(DetectResult);



#endif // TYPES_H
