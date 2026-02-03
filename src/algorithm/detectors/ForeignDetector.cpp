/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ForeignDetector.cpp
 *
 * 优化版本：1.1
 * 作者：Vere
 * 修改日期：2026年02月
 * 摘要：异物检测器优化实现
 * 描述：
 *   - 添加 LBP 纹理分析检测不明显异物
 *   - 添加形状特征分析（圆形度、凸包率等）
 *   - 统一使用 NMSFilter 进行去重
 *   - 优化颜色异常检测算法
 */

#include "ForeignDetector.h"
#include "../postprocess/NMSFilter.h"
#include "../common/Logger.h"
#include <QElapsedTimer>
#include <cmath>

ForeignDetector::ForeignDetector() {
  m_confidenceThreshold = 0.5;
}

bool ForeignDetector::initialize() {
  updateParameters();
  m_initialized = true;
  return true;
}

void ForeignDetector::release() {
  m_initialized = false;
}

void ForeignDetector::updateParameters() {
  m_minArea = getParam<int>("minArea", 5);
  m_contrast = getParam<double>("contrast", 0.3);
  m_colorThreshold = getParam<int>("colorThreshold", 50);
}

cv::Mat ForeignDetector::preprocessImage(const cv::Mat& input) {
  cv::Mat gray, blurred;

  // 转灰度
  if (input.channels() == 3) {
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
  } else {
    gray = input.clone();
  }

  // 中值滤波（去除椒盐噪声，保留边缘）
  cv::medianBlur(gray, blurred, 5);

  return blurred;
}

DetectionResult ForeignDetector::detect(const cv::Mat& image) {
  QElapsedTimer timer;
  timer.start();

  if (image.empty()) {
    LOG_ERROR("ForeignDetector::detect - Input image is empty");
    return makeErrorResult("Empty input image");
  }

  updateParameters();
  
  LOG_DEBUG("ForeignDetector::detect - Input: {}x{}, channels={}, params: minArea={}, contrast={:.2f}",
            image.cols, image.rows, image.channels(), m_minArea, m_contrast);

  std::vector<DefectInfo> allDefects;

  // 1. 灰度异物检测（Top-hat/Black-hat）
  cv::Mat preprocessed = preprocessImage(image);
  cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(15, 15));
  
  cv::Mat tophat, blackhat;
  cv::morphologyEx(preprocessed, tophat, cv::MORPH_TOPHAT, kernel);
  cv::morphologyEx(preprocessed, blackhat, cv::MORPH_BLACKHAT, kernel);

  cv::Mat combined;
  cv::add(tophat, blackhat, combined);

  int threshold = static_cast<int>(255 * m_contrast);
  cv::Mat binary;
  cv::threshold(combined, binary, threshold, 255, cv::THRESH_BINARY);

  cv::Mat smallKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
  cv::morphologyEx(binary, binary, cv::MORPH_OPEN, smallKernel);

  auto grayDefects = findForeignObjects(binary, image);
  size_t grayCount = grayDefects.size();
  allDefects.insert(allDefects.end(), grayDefects.begin(), grayDefects.end());

  // 2. 颜色异物检测（仅对彩色图像）
  size_t colorCount = 0;
  if (image.channels() == 3) {
    auto colorDefects = detectColorAnomalies(image);
    colorCount = colorDefects.size();
    allDefects.insert(allDefects.end(), colorDefects.begin(), colorDefects.end());
  }

  // 3. LBP 纹理异物检测
  auto textureDefects = detectTextureAnomalies(preprocessed);
  size_t textureCount = textureDefects.size();
  allDefects.insert(allDefects.end(), textureDefects.begin(), textureDefects.end());

  // 使用统一的 NMSFilter 去重
  size_t beforeNMS = allDefects.size();
  NMSFilter nmsFilter;
  nmsFilter.setIoUThreshold(0.5);
  nmsFilter.setConfidenceThreshold(0.0);
  allDefects = nmsFilter.filter(allDefects);

  // 过滤低置信度
  size_t beforeFilter = allDefects.size();
  allDefects = filterByConfidence(allDefects);

  double timeMs = timer.elapsed();
  
  // 统计
  double maxContrast = 0;
  double totalArea = 0;
  for (const auto& d : allDefects) {
    if (d.attributes.contains("contrast")) maxContrast = std::max(maxContrast, d.attributes.value("contrast").toDouble());
    if (d.attributes.contains("area")) totalArea += d.attributes.value("area").toDouble();
  }
  
  LOG_INFO("ForeignDetector::detect - Result: {} foreign (gray:{}, color:{}, texture:{}), NMS:{}->{}, filter:{}->{}, totalArea={:.0f}px, maxContrast={:.2f}, time:{:.1f}ms",
           allDefects.size(), grayCount, colorCount, textureCount, beforeNMS, beforeFilter, beforeFilter, allDefects.size(),
           totalArea, maxContrast, timeMs);
  
  return makeSuccessResult(allDefects, timeMs);
}

std::vector<DefectInfo> ForeignDetector::detectTextureAnomalies(const cv::Mat& gray) {
  std::vector<DefectInfo> defects;
  
  // 计算 LBP (Local Binary Pattern) 纹理特征
  int radius = 1;
  int neighbors = 8;
  
  cv::Mat lbp = cv::Mat::zeros(gray.size(), CV_8UC1);
  
  for (int y = radius; y < gray.rows - radius; ++y) {
    for (int x = radius; x < gray.cols - radius; ++x) {
      uchar center = gray.at<uchar>(y, x);
      uchar code = 0;
      
      // 8 邻域 LBP
      code |= (gray.at<uchar>(y-1, x-1) >= center) << 7;
      code |= (gray.at<uchar>(y-1, x) >= center) << 6;
      code |= (gray.at<uchar>(y-1, x+1) >= center) << 5;
      code |= (gray.at<uchar>(y, x+1) >= center) << 4;
      code |= (gray.at<uchar>(y+1, x+1) >= center) << 3;
      code |= (gray.at<uchar>(y+1, x) >= center) << 2;
      code |= (gray.at<uchar>(y+1, x-1) >= center) << 1;
      code |= (gray.at<uchar>(y, x-1) >= center) << 0;
      
      lbp.at<uchar>(y, x) = code;
    }
  }
  
  // 计算局部 LBP 直方图并检测异常
  int blockSize = 32;
  cv::Mat lbpMean, lbpStd;
  
  // 计算全局 LBP 直方图统计
  cv::Scalar globalMean, globalStd;
  cv::meanStdDev(lbp, globalMean, globalStd);
  
  // 滑动窗口检测局部纹理异常
  for (int y = 0; y < gray.rows - blockSize; y += blockSize / 2) {
    for (int x = 0; x < gray.cols - blockSize; x += blockSize / 2) {
      cv::Rect roi(x, y, blockSize, blockSize);
      cv::Mat block = lbp(roi);
      
      cv::Scalar localMean, localStd;
      cv::meanStdDev(block, localMean, localStd);
      
      // 检查纹理偏差
      double meanDiff = std::abs(localMean[0] - globalMean[0]);
      double stdDiff = std::abs(localStd[0] - globalStd[0]);
      
      // 纹理异常判断
      double anomalyScore = meanDiff / (globalStd[0] + 1.0) + stdDiff / (globalStd[0] + 1.0);
      
      if (anomalyScore > 2.0) {  // 阈值
        // 在这个区域进行更精细的检测
        cv::Mat grayBlock = gray(roi);
        cv::Mat binary;
        cv::threshold(grayBlock, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        for (const auto& contour : contours) {
          double area = cv::contourArea(contour);
          if (area < m_minArea || area > blockSize * blockSize * 0.8) continue;
          
          DefectInfo defect;
          cv::Rect bbox = cv::boundingRect(contour);
          defect.bbox = cv::Rect(bbox.x + x, bbox.y + y, bbox.width, bbox.height);
          
          // 调整轮廓坐标
          defect.contour = contour;
          for (auto& pt : defect.contour) {
            pt.x += x;
            pt.y += y;
          }
          
          defect.classId = 2;
          defect.className = "Foreign";
          defect.confidence = std::min(1.0, anomalyScore / 5.0);
          defect.severity = std::min(1.0, area / 200.0);
          defect.attributes["area"] = area;
          defect.attributes["method"] = "LBP";
          defect.attributes["textureAnomaly"] = anomalyScore;
          
          // 分析形状特征
          analyzeShapeFeatures(defect, defect.contour);
          
          defects.push_back(defect);
        }
      }
    }
  }
  
  return defects;
}

void ForeignDetector::analyzeShapeFeatures(DefectInfo& defect, const std::vector<cv::Point>& contour) {
  if (contour.size() < 5) return;
  
  double area = cv::contourArea(contour);
  double perimeter = cv::arcLength(contour, true);
  
  // 圆形度: 4 * PI * area / perimeter^2，圆形为1
  double circularity = (perimeter > 0) ? (4.0 * CV_PI * area) / (perimeter * perimeter) : 0.0;
  
  // 凸包
  std::vector<cv::Point> hull;
  cv::convexHull(contour, hull);
  double hullArea = cv::contourArea(hull);
  
  // 凸包率（实心度）：area / hullArea
  double solidity = (hullArea > 0) ? area / hullArea : 0.0;
  
  // 外接矩形
  cv::RotatedRect rotRect = cv::minAreaRect(contour);
  double rectArea = rotRect.size.width * rotRect.size.height;
  
  // 矩形度：area / rectArea
  double rectangularity = (rectArea > 0) ? area / rectArea : 0.0;
  
  // 长宽比
  double aspectRatio = (rotRect.size.height > 0) ? 
      std::max(rotRect.size.width, rotRect.size.height) / 
      std::min(rotRect.size.width, rotRect.size.height) : 1.0;
  
  // 存储形状特征
  defect.attributes["circularity"] = circularity;
  defect.attributes["solidity"] = solidity;
  defect.attributes["rectangularity"] = rectangularity;
  defect.attributes["aspectRatio"] = aspectRatio;
  defect.attributes["perimeter"] = perimeter;
  
  // 根据形状特征调整置信度
  // 异物通常形状不规则（低圆形度、低矩形度）
  double shapeScore = (1.0 - circularity) * 0.3 + (1.0 - rectangularity) * 0.3 + solidity * 0.4;
  defect.confidence = defect.confidence * 0.7 + shapeScore * 0.3;
}

std::vector<DefectInfo> ForeignDetector::detectColorAnomalies(const cv::Mat& image) {
  std::vector<DefectInfo> defects;
  
  // 转换到 Lab 颜色空间
  cv::Mat lab;
  cv::cvtColor(image, lab, cv::COLOR_BGR2Lab);
  
  std::vector<cv::Mat> channels;
  cv::split(lab, channels);
  
  // 计算 a, b 通道的均值和标准差
  cv::Scalar meanA, stdA, meanB, stdB;
  cv::meanStdDev(channels[1], meanA, stdA);
  cv::meanStdDev(channels[2], meanB, stdB);
  
  // 检测颜色异常区域（偏离均值超过阈值）
  cv::Mat anomalyA, anomalyB;
  cv::absdiff(channels[1], meanA[0], anomalyA);
  cv::absdiff(channels[2], meanB[0], anomalyB);
  
  cv::Mat colorAnomaly;
  cv::max(anomalyA, anomalyB, colorAnomaly);
  
  // 阈值化
  cv::Mat binary;
  double colorThresh = std::max(stdA[0], stdB[0]) * 2.5;
  cv::threshold(colorAnomaly, binary, colorThresh, 255, cv::THRESH_BINARY);
  
  // 形态学处理
  cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
  cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);
  cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, kernel);
  
  // 查找轮廓
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  
  for (const auto& contour : contours) {
    double area = cv::contourArea(contour);
    if (area < m_minArea) continue;
    
    DefectInfo defect;
    defect.bbox = cv::boundingRect(contour);
    defect.contour = contour;
    defect.classId = 2;
    defect.className = "Foreign";
    defect.confidence = std::min(1.0, area / 100.0);
    defect.severity = std::min(1.0, area / 200.0);
    defect.attributes["area"] = area;
    defect.attributes["method"] = "color";
    
    // 分析形状特征
    analyzeShapeFeatures(defect, contour);
    
    defects.push_back(defect);
  }
  
  return defects;
}

std::vector<DefectInfo> ForeignDetector::findForeignObjects(const cv::Mat& binary, const cv::Mat& original) {
  std::vector<DefectInfo> defects;

  // 查找轮廓
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  // 计算图像平均亮度（用于对比度计算）
  cv::Mat gray;
  if (original.channels() == 3) {
    cv::cvtColor(original, gray, cv::COLOR_BGR2GRAY);
  } else {
    gray = original;
  }
  double meanBrightness = cv::mean(gray)[0];

  for (const auto& contour : contours) {
    double area = cv::contourArea(contour);
    
    if (area < m_minArea) {
      continue;
    }

    cv::Rect bbox = cv::boundingRect(contour);

    // 计算区域内的平均亮度
    cv::Mat roi = gray(bbox);
    cv::Mat mask = cv::Mat::zeros(bbox.size(), CV_8UC1);
    std::vector<std::vector<cv::Point>> contourShifted = {contour};
    for (auto& pt : contourShifted[0]) {
      pt.x -= bbox.x;
      pt.y -= bbox.y;
    }
    cv::drawContours(mask, contourShifted, 0, cv::Scalar(255), -1);
    double roiMean = cv::mean(roi, mask)[0];

    // 计算与背景的对比度
    double contrast = std::abs(roiMean - meanBrightness) / 255.0;

    // 创建缺陷信息
    DefectInfo defect;
    defect.bbox = bbox;
    defect.contour = contour;
    defect.classId = 2;
    defect.className = "Foreign";
    
    // 置信度基于对比度和面积
    double contrastScore = std::min(1.0, contrast / 0.5);
    double areaScore = std::min(1.0, area / 100.0);
    defect.confidence = contrastScore * 0.7 + areaScore * 0.3;
    
    // 严重度
    defect.severity = calculateSeverity(area, contrast);
    
    // 附加属性
    defect.attributes["area"] = area;
    defect.attributes["contrast"] = contrast;
    defect.attributes["meanBrightness"] = roiMean;
    defect.attributes["method"] = "morphology";
    
    // 分析形状特征
    analyzeShapeFeatures(defect, contour);

    defects.push_back(defect);
  }

  return defects;
}

double ForeignDetector::calculateSeverity(double area, double contrast) {
  // 面积和对比度越大，严重度越高
  double areaFactor = std::min(1.0, area / 200.0);
  double contrastFactor = std::min(1.0, contrast / 0.5);
  
  return areaFactor * 0.4 + contrastFactor * 0.6;
}
