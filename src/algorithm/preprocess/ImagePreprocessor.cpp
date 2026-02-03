/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ImagePreprocessor.cpp
 *
 * 优化版本：1.1
 * 作者：Vere
 * 修改日期：2026年02月
 * 摘要：图像预处理器优化实现
 * 描述：
 *   - 添加图像质量分析功能
 *   - 添加自适应参数建议
 *   - 添加 Retinex 增强（SSR、MSR、MSRCR）
 *   - 优化去噪和对比度增强策略
 */

#include "ImagePreprocessor.h"
#include <cmath>
#include <algorithm>

ImagePreprocessor::ImagePreprocessor() {
}

void ImagePreprocessor::setROI(const cv::Rect& roi) {
  m_roi = roi;
  m_hasROI = true;
}

void ImagePreprocessor::clearROI() {
  m_roi = cv::Rect();
  m_hasROI = false;
}

void ImagePreprocessor::setDenoiseStrength(int strength) {
  m_denoiseStrength = qBound(0, strength, 100);
}

void ImagePreprocessor::setContrastEnhance(double factor) {
  m_contrastFactor = qBound(0.5, factor, 2.0);
}

void ImagePreprocessor::setBrightnessAdjust(int delta) {
  m_brightnessDelta = qBound(-100, delta, 100);
}

void ImagePreprocessor::setGammaCorrection(double gamma) {
  m_gamma = qBound(0.1, gamma, 3.0);
}

double ImagePreprocessor::estimateNoise(const cv::Mat& gray) {
  // 使用拉普拉斯算子估计噪声
  // 基于 Immerkaer 方法
  cv::Mat laplacian;
  cv::Laplacian(gray, laplacian, CV_64F);
  
  cv::Scalar mean, stddev;
  cv::meanStdDev(laplacian, mean, stddev);
  
  // 噪声估计 = sigma * sqrt(pi/2) / 6
  double sigma = stddev[0] * std::sqrt(CV_PI / 2.0) / 6.0;
  return sigma;
}

double ImagePreprocessor::estimateSharpness(const cv::Mat& gray) {
  // 使用拉普拉斯方差估计清晰度
  cv::Mat laplacian;
  cv::Laplacian(gray, laplacian, CV_64F);
  
  cv::Scalar mean, stddev;
  cv::meanStdDev(laplacian, mean, stddev);
  
  // 方差越大越清晰
  return stddev[0] * stddev[0];
}

ImageQuality ImagePreprocessor::analyzeQuality(const cv::Mat& input) {
  ImageQuality quality;
  
  cv::Mat gray;
  if (input.channels() == 3) {
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
  } else {
    gray = input;
  }
  
  // 1. 亮度分析
  cv::Scalar mean, stddev;
  cv::meanStdDev(gray, mean, stddev);
  quality.brightness = mean[0];
  quality.contrast = stddev[0];
  
  // 2. 动态范围
  double minVal, maxVal;
  cv::minMaxLoc(gray, &minVal, &maxVal);
  quality.dynamicRange = (maxVal - minVal) / 255.0;
  
  // 3. 清晰度
  quality.sharpness = estimateSharpness(gray);
  
  // 4. 噪声估计
  quality.noise = estimateNoise(gray);
  
  // 5. 判断问题
  quality.isUnderexposed = quality.brightness < 60;
  quality.isOverexposed = quality.brightness > 200;
  quality.isLowContrast = quality.contrast < 30 || quality.dynamicRange < 0.3;
  quality.isBlurry = quality.sharpness < 100;
  
  return quality;
}

AdaptiveParams ImagePreprocessor::suggestParams(const ImageQuality& quality) {
  AdaptiveParams params;
  
  // 1. 去噪强度（基于噪声估计）
  if (quality.noise < 5) {
    params.denoiseStrength = 0;
  } else if (quality.noise < 10) {
    params.denoiseStrength = 20;
  } else if (quality.noise < 20) {
    params.denoiseStrength = 50;
  } else {
    params.denoiseStrength = 80;
  }
  
  // 2. 亮度调整
  if (quality.isUnderexposed) {
    // 欠曝，需要提亮
    params.brightnessDelta = static_cast<int>((128 - quality.brightness) * 0.5);
    params.gamma = 0.7;  // 提亮暗部
  } else if (quality.isOverexposed) {
    // 过曝，需要压暗
    params.brightnessDelta = static_cast<int>((128 - quality.brightness) * 0.3);
    params.gamma = 1.3;  // 压暗亮部
  } else {
    params.brightnessDelta = 0;
    params.gamma = 1.0;
  }
  
  // 3. 对比度增强
  if (quality.isLowContrast) {
    params.contrastFactor = std::min(1.8, 60.0 / (quality.contrast + 1));
    params.needsCLAHE = true;
  } else if (quality.contrast > 80) {
    params.contrastFactor = 0.8;  // 降低过高对比度
  } else {
    params.contrastFactor = 1.0;
  }
  
  // 4. Retinex 建议（用于光照不均或动态范围压缩的情况）
  if (quality.dynamicRange < 0.4 && quality.isLowContrast) {
    params.needsRetinex = true;
  }
  
  return params;
}

cv::Mat ImagePreprocessor::processAdaptive(const cv::Mat& input) {
  if (input.empty()) {
    return input;
  }
  
  // 分析图像质量
  ImageQuality quality = analyzeQuality(input);
  
  // 获取建议参数
  AdaptiveParams params = suggestParams(quality);
  
  cv::Mat result = input.clone();
  
  // 1. 应用 ROI
  if (m_hasROI) {
    result = applyROI(result);
  }
  
  // 2. Retinex 增强（如果需要）
  if (params.needsRetinex) {
    result = multiScaleRetinex(result);
  }
  
  // 3. 去噪
  if (params.denoiseStrength > 0) {
    m_denoiseStrength = params.denoiseStrength;
    result = denoise(result);
  }
  
  // 4. 亮度调整
  if (params.brightnessDelta != 0) {
    m_brightnessDelta = params.brightnessDelta;
    result = adjustBrightness(result);
  }
  
  // 5. 对比度增强
  if (params.needsCLAHE) {
    result = clahe(result, 2.0);
  } else if (std::abs(params.contrastFactor - 1.0) > 0.01) {
    m_contrastFactor = params.contrastFactor;
    result = enhanceContrast(result);
  }
  
  // 6. Gamma 校正
  if (std::abs(params.gamma - 1.0) > 0.01) {
    m_gamma = params.gamma;
    result = correctGamma(result);
  }
  
  return result;
}

cv::Mat ImagePreprocessor::process(const cv::Mat& input) {
  if (input.empty()) {
    return input;
  }
  
  // 如果启用自适应模式，使用自适应处理
  if (m_autoAdapt) {
    return processAdaptive(input);
  }

  cv::Mat result = input.clone();

  // 1. 应用 ROI
  if (m_hasROI) {
    result = applyROI(result);
  }

  // 2. 去噪
  if (m_denoiseStrength > 0) {
    result = denoise(result);
  }

  // 3. 亮度调整
  if (m_brightnessDelta != 0) {
    result = adjustBrightness(result);
  }

  // 4. 对比度增强
  if (std::abs(m_contrastFactor - 1.0) > 0.01) {
    result = enhanceContrast(result);
  }

  // 5. Gamma 校正
  if (std::abs(m_gamma - 1.0) > 0.01) {
    result = correctGamma(result);
  }

  return result;
}

cv::Mat ImagePreprocessor::applyROI(const cv::Mat& input) {
  if (!m_hasROI || m_roi.empty()) {
    return input;
  }

  // 确保 ROI 在图像范围内
  cv::Rect validROI = m_roi & cv::Rect(0, 0, input.cols, input.rows);
  if (validROI.empty()) {
    return input;
  }

  return input(validROI).clone();
}

cv::Mat ImagePreprocessor::denoise(const cv::Mat& input) {
  cv::Mat result;
  
  // 根据强度选择去噪方法
  if (m_denoiseStrength <= 30) {
    // 轻度：高斯模糊
    int ksize = 3;
    cv::GaussianBlur(input, result, cv::Size(ksize, ksize), 0);
  } else if (m_denoiseStrength <= 60) {
    // 中度：双边滤波（保边去噪）
    int d = 5 + (m_denoiseStrength - 30) / 10;
    double sigmaColor = 30 + (m_denoiseStrength - 30);
    double sigmaSpace = 30 + (m_denoiseStrength - 30);
    cv::bilateralFilter(input, result, d, sigmaColor, sigmaSpace);
  } else {
    // 强力：非局部均值去噪
    int h = 5 + (m_denoiseStrength - 60) / 10;
    if (input.channels() == 1) {
      cv::fastNlMeansDenoising(input, result, h);
    } else {
      cv::fastNlMeansDenoisingColored(input, result, h, h);
    }
  }

  return result;
}

cv::Mat ImagePreprocessor::adjustBrightness(const cv::Mat& input) {
  cv::Mat result;
  input.convertTo(result, -1, 1.0, m_brightnessDelta);
  return result;
}

cv::Mat ImagePreprocessor::enhanceContrast(const cv::Mat& input) {
  cv::Mat result;
  // 以128为中心进行对比度调整
  double beta = 128 * (1.0 - m_contrastFactor);
  input.convertTo(result, -1, m_contrastFactor, beta);
  return result;
}

cv::Mat ImagePreprocessor::correctGamma(const cv::Mat& input) {
  cv::Mat lookUpTable(1, 256, CV_8U);
  uchar* p = lookUpTable.ptr();
  double invGamma = 1.0 / m_gamma;
  
  for (int i = 0; i < 256; ++i) {
    p[i] = cv::saturate_cast<uchar>(std::pow(i / 255.0, invGamma) * 255.0);
  }

  cv::Mat result;
  cv::LUT(input, lookUpTable, result);
  return result;
}

cv::Mat ImagePreprocessor::normalize(const cv::Mat& input) {
  cv::Mat result;
  cv::normalize(input, result, 0, 255, cv::NORM_MINMAX);
  return result;
}

cv::Mat ImagePreprocessor::sharpen(const cv::Mat& input) {
  cv::Mat kernel = (cv::Mat_<float>(3, 3) <<
    0, -1, 0,
    -1, 5, -1,
    0, -1, 0);
  
  cv::Mat result;
  cv::filter2D(input, result, -1, kernel);
  return result;
}

cv::Mat ImagePreprocessor::equalizeHistogram(const cv::Mat& input) {
  cv::Mat result;
  
  if (input.channels() == 1) {
    cv::equalizeHist(input, result);
  } else {
    // 转换到 YCrCb，只均衡化 Y 通道
    cv::Mat ycrcb;
    cv::cvtColor(input, ycrcb, cv::COLOR_BGR2YCrCb);
    
    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);
    cv::equalizeHist(channels[0], channels[0]);
    cv::merge(channels, ycrcb);
    
    cv::cvtColor(ycrcb, result, cv::COLOR_YCrCb2BGR);
  }

  return result;
}

cv::Mat ImagePreprocessor::clahe(const cv::Mat& input, double clipLimit) {
  cv::Mat result;
  cv::Ptr<cv::CLAHE> clahePtr = cv::createCLAHE(clipLimit, cv::Size(8, 8));
  
  if (input.channels() == 1) {
    clahePtr->apply(input, result);
  } else {
    // 转换到 LAB，只处理 L 通道
    cv::Mat lab;
    cv::cvtColor(input, lab, cv::COLOR_BGR2Lab);
    
    std::vector<cv::Mat> channels;
    cv::split(lab, channels);
    clahePtr->apply(channels[0], channels[0]);
    cv::merge(channels, lab);
    
    cv::cvtColor(lab, result, cv::COLOR_Lab2BGR);
  }

  return result;
}

cv::Mat ImagePreprocessor::singleScaleRetinex(const cv::Mat& input, double sigma) {
  cv::Mat floatImg;
  input.convertTo(floatImg, CV_64F);
  
  // 避免 log(0)
  floatImg += 1.0;
  
  // 高斯模糊
  cv::Mat blurred;
  int ksize = static_cast<int>(sigma * 6) | 1;  // 确保是奇数
  cv::GaussianBlur(floatImg, blurred, cv::Size(ksize, ksize), sigma);
  
  // SSR = log(input) - log(blur)
  cv::Mat logInput, logBlur;
  cv::log(floatImg, logInput);
  cv::log(blurred + 1.0, logBlur);
  
  cv::Mat ssr = logInput - logBlur;
  
  // 归一化到 0-255
  cv::normalize(ssr, ssr, 0, 255, cv::NORM_MINMAX);
  
  cv::Mat result;
  ssr.convertTo(result, CV_8U);
  
  return result;
}

cv::Mat ImagePreprocessor::multiScaleRetinex(const cv::Mat& input, 
                                              const std::vector<double>& sigmas) {
  if (input.channels() == 1) {
    // 灰度图像
    cv::Mat msr = cv::Mat::zeros(input.size(), CV_64F);
    
    for (double sigma : sigmas) {
      cv::Mat ssr = singleScaleRetinex(input, sigma);
      cv::Mat ssrFloat;
      ssr.convertTo(ssrFloat, CV_64F);
      msr += ssrFloat / sigmas.size();
    }
    
    cv::Mat result;
    msr.convertTo(result, CV_8U);
    return result;
  } else {
    // 彩色图像，分通道处理
    std::vector<cv::Mat> channels;
    cv::split(input, channels);
    
    for (auto& channel : channels) {
      cv::Mat msr = cv::Mat::zeros(channel.size(), CV_64F);
      
      for (double sigma : sigmas) {
        cv::Mat ssr = singleScaleRetinex(channel, sigma);
        cv::Mat ssrFloat;
        ssr.convertTo(ssrFloat, CV_64F);
        msr += ssrFloat / sigmas.size();
      }
      
      msr.convertTo(channel, CV_8U);
    }
    
    cv::Mat result;
    cv::merge(channels, result);
    return result;
  }
}

cv::Mat ImagePreprocessor::msrcr(const cv::Mat& input) {
  if (input.channels() != 3) {
    // 非彩色图像，使用普通 MSR
    return multiScaleRetinex(input);
  }
  
  // MSRCR: Multi-Scale Retinex with Color Restoration
  std::vector<double> sigmas = {15, 80, 250};
  
  // 分离通道
  std::vector<cv::Mat> channels;
  cv::split(input, channels);
  
  // 计算每个通道的 MSR
  std::vector<cv::Mat> msrChannels;
  for (const auto& channel : channels) {
    cv::Mat msr = cv::Mat::zeros(channel.size(), CV_64F);
    
    for (double sigma : sigmas) {
      cv::Mat ssr = singleScaleRetinex(channel, sigma);
      cv::Mat ssrFloat;
      ssr.convertTo(ssrFloat, CV_64F);
      msr += ssrFloat / sigmas.size();
    }
    
    msrChannels.push_back(msr);
  }
  
  // 颜色恢复
  // CRF = beta * log(alpha * Ic / sum(Ic)) 
  // 其中 Ic 是每个通道，sum(Ic) 是所有通道之和
  double alpha = 125.0;
  double beta = 46.0;
  double gain = 1.0;
  double offset = 0.0;
  
  cv::Mat sum = cv::Mat::zeros(input.size(), CV_64F);
  for (const auto& channel : channels) {
    cv::Mat floatChannel;
    channel.convertTo(floatChannel, CV_64F);
    sum += floatChannel;
  }
  sum += 1.0;  // 避免除零
  
  std::vector<cv::Mat> resultChannels;
  for (size_t i = 0; i < channels.size(); ++i) {
    cv::Mat floatChannel;
    channels[i].convertTo(floatChannel, CV_64F);
    floatChannel += 1.0;
    
    // 颜色恢复因子
    cv::Mat crf;
    cv::log(alpha * floatChannel / sum, crf);
    crf *= beta;
    
    // 应用颜色恢复
    cv::Mat restored = gain * (msrChannels[i].mul(crf)) + offset;
    
    // 归一化
    cv::normalize(restored, restored, 0, 255, cv::NORM_MINMAX);
    
    cv::Mat channel8u;
    restored.convertTo(channel8u, CV_8U);
    resultChannels.push_back(channel8u);
  }
  
  cv::Mat result;
  cv::merge(resultChannels, result);
  
  return result;
}
