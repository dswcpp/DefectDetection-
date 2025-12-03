/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * CrackDetector.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：裂纹检测器接口定义
 * 描述：基于形态学和轮廓分析的裂纹检测算法，支持分支裂纹检测、
 *       裂纹长度和宽度测量
 *
 * 当前版本：1.0
 */

#ifndef CRACKDETECTOR_H
#define CRACKDETECTOR_H

#include "../BaseDetector.h"

// 裂纹检测器：使用二值化+形态学+连通域分析
class ALGORITHM_LIBRARY CrackDetector : public BaseDetector {
public:
  CrackDetector();
  ~CrackDetector() override = default;

  // IDefectDetector 接口
  QString name() const override { return QObject::tr("裂纹检测"); }
  QString type() const override { return "crack"; }
  
  bool initialize() override;
  void release() override;
  DetectionResult detect(const cv::Mat& image) override;

private:
  // 参数
  int m_threshold = 80;          // 检测阈值 [0-100]
  int m_minArea = 20;            // 最小面积（像素²）
  int m_morphKernelSize = 3;     // 形态学核大小
  int m_binaryThreshold = 128;   // 二值化阈值

  // 内部方法
  void updateParameters();
  cv::Mat preprocessImage(const cv::Mat& input);
  std::vector<DefectInfo> findCracks(const cv::Mat& binary, const cv::Mat& original);
  bool isValidCrack(const std::vector<cv::Point>& contour);
  double calculateSeverity(double area, double length);
};

#endif // CRACKDETECTOR_H
