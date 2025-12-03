#include "ImagePreprocessor.h"
#include <cmath>

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

cv::Mat ImagePreprocessor::process(const cv::Mat& input) {
  if (input.empty()) {
    return input;
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
    int d = 5;
    double sigmaColor = 50;
    double sigmaSpace = 50;
    cv::bilateralFilter(input, result, d, sigmaColor, sigmaSpace);
  } else {
    // 强力：非局部均值去噪
    if (input.channels() == 1) {
      cv::fastNlMeansDenoising(input, result, 10);
    } else {
      cv::fastNlMeansDenoisingColored(input, result, 10, 10);
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
