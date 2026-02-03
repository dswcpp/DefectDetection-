/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * DimensionDetector.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：尺寸检测器接口定义
 * 描述：尺寸测量检测器，基于边缘检测和亚像素拟合，
 *       支持长度、宽度、角度测量和公差判断
 *
 * 当前版本：1.0
 */

#ifndef DIMENSIONDETECTOR_H
#define DIMENSIONDETECTOR_H

#include "../BaseDetector.h"

// 尺寸检测器：检测产品尺寸是否在公差范围内
class ALGORITHM_LIBRARY DimensionDetector : public BaseDetector {
public:
  DimensionDetector();
  ~DimensionDetector() override = default;

  // IDefectDetector 接口
  QString name() const override { return QObject::tr("尺寸测量"); }
  QString type() const override { return "dimension"; }
  
  bool initialize() override;
  void release() override;
  DetectionResult detect(const cv::Mat& image) override;

  // 测量结果结构
  struct MeasurementResult {
    double value = 0.0;           // 测量值（mm）
    double deviation = 0.0;       // 偏差（mm）
    double confidence = 0.0;      // 测量置信度
    bool withinTolerance = true;  // 是否在公差内
    QString type;                 // 测量类型
  };

private:
  // 参数
  double m_tolerance = 0.5;       // 公差（mm）
  double m_calibration = 0.1;     // 标定系数（mm/pixel）
  double m_targetWidth = 100.0;   // 目标宽度（mm）
  double m_targetHeight = 100.0;  // 目标高度（mm）
  bool m_useSubpixel = true;      // 是否使用亚像素精度
  int m_ransacIterations = 100;   // RANSAC 迭代次数

  // 内部方法
  void updateParameters();
  cv::Mat preprocessImage(const cv::Mat& input);
  std::vector<cv::Point2f> detectSubpixelEdges(const cv::Mat& gray, const cv::Mat& binary);
  cv::Vec4f fitLineRANSAC(const std::vector<cv::Point2f>& points, double threshold);
  double measureLineDistance(const cv::Vec4f& line1, const cv::Vec4f& line2);
  MeasurementResult measureWidth(const std::vector<cv::Point2f>& edges, const cv::Rect& bbox);
  MeasurementResult measureHeight(const std::vector<cv::Point2f>& edges, const cv::Rect& bbox);
  MeasurementResult measureCircularity(const std::vector<cv::Point>& contour);
  MeasurementResult measureParallelism(const cv::Vec4f& line1, const cv::Vec4f& line2);
  std::vector<DefectInfo> measureDimensions(const cv::Mat& binary, const cv::Mat& original);
  double pixelToMm(double pixels) const;
  double calculateSeverity(double deviation, double tolerance);
};

#endif // DIMENSIONDETECTOR_H
