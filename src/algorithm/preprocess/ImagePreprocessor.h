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
// 只包含必要的 OpenCV 头文件
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>  // for fastNlMeansDenoising
#include <QRect>
#include <vector>

// 图像质量分析结果
struct ALGORITHM_LIBRARY ImageQuality {
  double brightness = 0.0;      // 平均亮度 [0-255]
  double contrast = 0.0;        // 对比度（标准差）
  double sharpness = 0.0;       // 清晰度（拉普拉斯方差）
  double noise = 0.0;           // 噪声估计
  double dynamicRange = 0.0;    // 动态范围 [0-1]
  bool isUnderexposed = false;  // 是否欠曝
  bool isOverexposed = false;   // 是否过曝
  bool isLowContrast = false;   // 是否低对比度
  bool isBlurry = false;        // 是否模糊
};

// 自适应参数建议
struct ALGORITHM_LIBRARY AdaptiveParams {
  int denoiseStrength = 10;
  double contrastFactor = 1.0;
  int brightnessDelta = 0;
  double gamma = 1.0;
  bool needsCLAHE = false;
  bool needsRetinex = false;
};

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
  void setAutoAdapt(bool enable) { m_autoAdapt = enable; }
  bool isAutoAdapt() const { return m_autoAdapt; }

  // 执行预处理
  cv::Mat process(const cv::Mat& input);
  
  // 自适应预处理（自动分析图像并优化参数）
  cv::Mat processAdaptive(const cv::Mat& input);

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
  
  // Retinex 增强（新增）
  cv::Mat singleScaleRetinex(const cv::Mat& input, double sigma = 80.0);
  cv::Mat multiScaleRetinex(const cv::Mat& input, 
                            const std::vector<double>& sigmas = {15, 80, 250});
  cv::Mat msrcr(const cv::Mat& input);  // Multi-Scale Retinex with Color Restoration
  
  // 图像质量分析（新增）
  ImageQuality analyzeQuality(const cv::Mat& input);
  
  // 获取自适应参数建议（新增）
  AdaptiveParams suggestParams(const ImageQuality& quality);

private:
  cv::Rect m_roi;
  bool m_hasROI = false;
  bool m_autoAdapt = false;

  int m_denoiseStrength = 10;
  double m_contrastFactor = 1.0;
  int m_brightnessDelta = 0;
  double m_gamma = 1.0;
  
  // 内部辅助方法
  double estimateNoise(const cv::Mat& gray);
  double estimateSharpness(const cv::Mat& gray);
};

#endif // IMAGEPREPROCESSOR_H
