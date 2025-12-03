/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ImagePreprocessor.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：图像预处理模块接口定义
 * 描述：图像预处理器，提供去噪、对比度增强、亮度调整、
 *       Gamma校正、ROI裁剪等功能
 *
 * 当前版本：1.0
 */

#ifndef IMAGEPREPROCESSOR_H
#define IMAGEPREPROCESSOR_H

#include "../algorithm_global.h"
#include <opencv2/opencv.hpp>
#include <QRect>
#include <vector>

// 图像预处理器
class ALGORITHM_LIBRARY ImagePreprocessor {
public:
  ImagePreprocessor();
  ~ImagePreprocessor() = default;

  // ROI 设置
  void setROI(const cv::Rect& roi);
  void clearROI();
  bool hasROI() const { return m_hasROI; }
  cv::Rect roi() const { return m_roi; }

  // 预处理参数
  void setDenoiseStrength(int strength);  // 去噪强度 [0-100]
  void setContrastEnhance(double factor); // 对比度增强 [0.5-2.0]
  void setBrightnessAdjust(int delta);    // 亮度调整 [-100, 100]
  void setGammaCorrection(double gamma);  // Gamma 校正 [0.1-3.0]

  // 执行预处理
  cv::Mat process(const cv::Mat& input);

  // 单独处理步骤（可选择性调用）
  cv::Mat applyROI(const cv::Mat& input);
  cv::Mat denoise(const cv::Mat& input);
  cv::Mat adjustBrightness(const cv::Mat& input);
  cv::Mat enhanceContrast(const cv::Mat& input);
  cv::Mat correctGamma(const cv::Mat& input);
  cv::Mat normalize(const cv::Mat& input);

  // 图像增强
  cv::Mat sharpen(const cv::Mat& input);
  cv::Mat equalizeHistogram(const cv::Mat& input);
  cv::Mat clahe(const cv::Mat& input, double clipLimit = 2.0);

private:
  cv::Rect m_roi;
  bool m_hasROI = false;

  int m_denoiseStrength = 10;
  double m_contrastFactor = 1.0;
  int m_brightnessDelta = 0;
  double m_gamma = 1.0;
};

#endif // IMAGEPREPROCESSOR_H
