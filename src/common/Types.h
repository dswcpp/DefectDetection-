/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * Types.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：通用类型定义
 * 描述：定义项目中使用的通用数据结构：DetectResult检测结果、DefectRegion缺陷区域、
 *       SeverityLevel严重等级、PerfStats性能统计等
 *
 * 当前版本：1.0
 */

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

// ==================== 用户权限相关 ====================

// 权限枚举
enum class Permission {
  ViewHistory = 0,      // 查看历史记录
  DeleteHistory,        // 删除历史记录
  ViewStatistics,       // 查看统计
  ExportData,           // 导出数据
  RunDetection,         // 运行检测
  SystemSettings,       // 系统设置
  ManageUsers,          // 用户管理
  COUNT                 // 权限总数
};

// 用户角色
enum class UserRole {
  Admin,      // 管理员 - 所有权限
  Operator,   // 操作员 - 操作权限
  Viewer      // 观察员 - 只读权限
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
