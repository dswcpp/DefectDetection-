/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * CrackDetector.cpp
 *
 * 优化版本：1.1
 * 作者：Vere
 * 修改日期：2026年02月
 * 摘要：裂纹检测器优化实现
 * 描述：
 *   - 添加 Gabor 滤波器增强线性特征
 *   - 添加骨架化分析准确测量裂纹长度
 *   - 添加分支点检测识别复杂裂纹网络
 *   - 统一使用 NMSFilter 进行去重
 */

#include "CrackDetector.h"
#include "../postprocess/NMSFilter.h"
#include "../common/Logger.h"
#include <QElapsedTimer>
#include <cmath>

CrackDetector::CrackDetector() {
  m_confidenceThreshold = 0.5;
}

bool CrackDetector::initialize() {
  updateParameters();
  m_initialized = true;
  return true;
}

void CrackDetector::release() {
  m_initialized = false;
}

void CrackDetector::updateParameters() {
  m_threshold = getParam<int>("threshold", 80);
  m_minArea = getParam<int>("minArea", 20);
  m_morphKernelSize = getParam<int>("morphKernelSize", 3);
  m_binaryThreshold = getParam<int>("binaryThreshold", 128);
  m_useGabor = getParam<bool>("useGabor", true);
}

cv::Mat CrackDetector::applyGaborFilter(const cv::Mat& gray) {
  // 多方向 Gabor 滤波器，增强线性特征
  std::vector<double> orientations = {0, 45, 90, 135};  // 4个方向
  int kernelSize = 21;
  double sigma = 4.0;
  double lambda = 10.0;
  double gamma = 0.5;
  double psi = 0;
  
  cv::Mat result = cv::Mat::zeros(gray.size(), CV_32F);
  
  for (double theta : orientations) {
    double thetaRad = theta * CV_PI / 180.0;
    
    // 创建 Gabor 核
    cv::Mat gaborKernel = cv::getGaborKernel(
        cv::Size(kernelSize, kernelSize),
        sigma, thetaRad, lambda, gamma, psi, CV_32F
    );
    
    // 应用滤波
    cv::Mat filtered;
    cv::filter2D(gray, filtered, CV_32F, gaborKernel);
    
    // 取绝对值并累加最大响应
    cv::Mat absFiltered = cv::abs(filtered);
    result = cv::max(result, absFiltered);
  }
  
  // 归一化到 0-255
  cv::normalize(result, result, 0, 255, cv::NORM_MINMAX);
  cv::Mat output;
  result.convertTo(output, CV_8U);
  
  return output;
}

cv::Mat CrackDetector::skeletonize(const cv::Mat& binary) {
  // Zhang-Suen 骨架化算法实现
  cv::Mat img = binary.clone();
  img /= 255;  // 归一化到 0-1
  
  cv::Mat skeleton = cv::Mat::zeros(img.size(), CV_8UC1);
  cv::Mat temp;
  cv::Mat eroded;
  
  // 形态学骨架化
  cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
  
  bool done = false;
  do {
    cv::erode(img, eroded, element);
    cv::dilate(eroded, temp, element);
    cv::subtract(img, temp, temp);
    cv::bitwise_or(skeleton, temp, skeleton);
    eroded.copyTo(img);
    
    done = (cv::countNonZero(img) == 0);
  } while (!done);
  
  skeleton *= 255;  // 恢复到 0-255
  return skeleton;
}

int CrackDetector::countBranchPoints(const cv::Mat& skeleton, const cv::Rect& roi) {
  // 提取 ROI 区域
  cv::Mat region = skeleton(roi);
  
  int branchPoints = 0;
  
  // 遍历骨架点，计算8邻域连接数
  for (int y = 1; y < region.rows - 1; ++y) {
    for (int x = 1; x < region.cols - 1; ++x) {
      if (region.at<uchar>(y, x) == 0) continue;
      
      // 计算8邻域中的骨架点数量
      int neighbors = 0;
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
          if (dx == 0 && dy == 0) continue;
          if (region.at<uchar>(y + dy, x + dx) > 0) {
            neighbors++;
          }
        }
      }
      
      // 3个以上邻居表示分支点
      if (neighbors >= 3) {
        branchPoints++;
      }
    }
  }
  
  return branchPoints;
}

double CrackDetector::measureSkeletonLength(const cv::Mat& skeleton, const cv::Rect& roi) {
  // 提取 ROI 区域
  cv::Mat region = skeleton(roi);
  
  // 统计骨架像素数量
  int pixelCount = cv::countNonZero(region);
  
  // 考虑对角线连接（对角线距离为 sqrt(2)）
  double length = 0;
  
  for (int y = 1; y < region.rows - 1; ++y) {
    for (int x = 1; x < region.cols - 1; ++x) {
      if (region.at<uchar>(y, x) == 0) continue;
      
      // 检查8邻域连接方向
      bool hasDiagonal = false;
      bool hasOrthogonal = false;
      
      // 对角线方向
      if (region.at<uchar>(y-1, x-1) > 0 || region.at<uchar>(y-1, x+1) > 0 ||
          region.at<uchar>(y+1, x-1) > 0 || region.at<uchar>(y+1, x+1) > 0) {
        hasDiagonal = true;
      }
      // 正交方向
      if (region.at<uchar>(y-1, x) > 0 || region.at<uchar>(y+1, x) > 0 ||
          region.at<uchar>(y, x-1) > 0 || region.at<uchar>(y, x+1) > 0) {
        hasOrthogonal = true;
      }
      
      // 估算贡献长度
      if (hasDiagonal && !hasOrthogonal) {
        length += std::sqrt(2.0) / 2.0;
      } else if (!hasDiagonal && hasOrthogonal) {
        length += 0.5;
      } else {
        length += 0.75;  // 混合情况
      }
    }
  }
  
  // 简化：直接使用像素数作为长度估计
  return std::max(length, static_cast<double>(pixelCount) * 0.8);
}

cv::Mat CrackDetector::preprocessImage(const cv::Mat& input) {
  cv::Mat gray, enhanced, blurred, binary;

  // 转灰度
  if (input.channels() == 3) {
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
  } else {
    gray = input.clone();
  }

  // Gabor 滤波增强线性特征
  if (m_useGabor) {
    enhanced = applyGaborFilter(gray);
  } else {
    // CLAHE 增强对比度
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(gray, enhanced);
  }

  // 高斯模糊
  cv::GaussianBlur(enhanced, blurred, cv::Size(5, 5), 0);

  // 自适应二值化
  cv::adaptiveThreshold(blurred, binary, 255, 
                        cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                        cv::THRESH_BINARY_INV, 11, 2);

  // 形态学操作：闭运算填充小孔
  int kernelSize = m_morphKernelSize | 1;
  cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, 
                                              cv::Size(kernelSize, kernelSize));
  cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, kernel);

  return binary;
}

DetectionResult CrackDetector::detect(const cv::Mat& image) {
  QElapsedTimer timer;
  timer.start();

  if (image.empty()) {
    LOG_ERROR("CrackDetector::detect - Input image is empty");
    return makeErrorResult("Empty input image");
  }

  updateParameters();
  
  LOG_DEBUG("CrackDetector::detect - Input: {}x{}, params: threshold={}, minArea={}, morphKernel={}, useGabor={}",
            image.cols, image.rows, m_threshold, m_minArea, m_morphKernelSize, m_useGabor);

  // 预处理
  cv::Mat binary = preprocessImage(image);
  
  // 骨架化
  cv::Mat skeleton = skeletonize(binary);

  // 查找裂纹（使用轮廓方法）
  std::vector<DefectInfo> contourDefects = findCracks(binary, image);
  
  // 骨架分析（补充方法）
  std::vector<DefectInfo> skeletonDefects = analyzeSkeleton(skeleton, binary, image);
  
  // 合并结果
  std::vector<DefectInfo> allDefects;
  allDefects.insert(allDefects.end(), contourDefects.begin(), contourDefects.end());
  allDefects.insert(allDefects.end(), skeletonDefects.begin(), skeletonDefects.end());
  
  // 使用 NMSFilter 去重
  size_t beforeNMS = allDefects.size();
  NMSFilter nmsFilter;
  nmsFilter.setIoUThreshold(0.5);
  nmsFilter.setConfidenceThreshold(0.0);
  allDefects = nmsFilter.filter(allDefects);

  // 过滤低置信度
  size_t beforeFilter = allDefects.size();
  allDefects = filterByConfidence(allDefects);

  double timeMs = timer.elapsed();
  
  // 统计信息
  double totalArea = 0, maxComplexity = 0, totalLength = 0;
  int totalBranches = 0;
  for (const auto& d : allDefects) {
    if (d.attributes.contains("area")) totalArea += d.attributes.value("area").toDouble();
    if (d.attributes.contains("complexity")) maxComplexity = std::max(maxComplexity, d.attributes.value("complexity").toDouble());
    if (d.attributes.contains("skeletonLength")) totalLength += d.attributes.value("skeletonLength").toDouble();
    if (d.attributes.contains("branchPoints")) totalBranches += d.attributes.value("branchPoints").toInt();
  }
  
  LOG_INFO("CrackDetector::detect - Result: {} cracks (contour:{}, skeleton:{}), NMS:{}->{}, filter:{}->{}, totalArea={:.0f}px, totalLength={:.0f}px, branches={}, time:{:.1f}ms",
           allDefects.size(), contourDefects.size(), skeletonDefects.size(),
           beforeNMS, beforeFilter, beforeFilter, allDefects.size(),
           totalArea, totalLength, totalBranches, timeMs);
  
  return makeSuccessResult(allDefects, timeMs);
}

std::vector<DefectInfo> CrackDetector::analyzeSkeleton(const cv::Mat& skeleton, 
                                                        const cv::Mat& binary,
                                                        const cv::Mat& /*original*/) {
  std::vector<DefectInfo> defects;
  
  // 在骨架上查找连通域
  cv::Mat labels, stats, centroids;
  int numLabels = cv::connectedComponentsWithStats(skeleton, labels, stats, centroids);
  
  for (int i = 1; i < numLabels; ++i) {  // 跳过背景
    int x = stats.at<int>(i, cv::CC_STAT_LEFT);
    int y = stats.at<int>(i, cv::CC_STAT_TOP);
    int w = stats.at<int>(i, cv::CC_STAT_WIDTH);
    int h = stats.at<int>(i, cv::CC_STAT_HEIGHT);
    int pixelCount = stats.at<int>(i, cv::CC_STAT_AREA);
    
    if (pixelCount < 10) continue;  // 过滤小碎片
    
    cv::Rect bbox(x, y, w, h);
    
    // 确保 bbox 在图像范围内
    bbox &= cv::Rect(0, 0, skeleton.cols, skeleton.rows);
    if (bbox.empty()) continue;
    
    // 测量骨架长度
    double skeletonLength = measureSkeletonLength(skeleton, bbox);
    if (skeletonLength < m_minArea / 2) continue;
    
    // 计算分支点数量
    int branchPoints = countBranchPoints(skeleton, bbox);
    
    // 获取对应的二值区域面积
    cv::Mat binaryROI = binary(bbox);
    double area = cv::countNonZero(binaryROI);
    
    // 计算复杂度
    double complexity = (skeletonLength > 0) ? (skeletonLength * skeletonLength) / (area + 1) : 0;
    
    // 只保留复杂度足够高的（排除圆形斑点）
    if (complexity < 1.5 && branchPoints == 0) continue;
    
    DefectInfo defect;
    defect.bbox = bbox;
    defect.classId = 1;
    defect.className = "Crack";
    
    // 置信度基于长度、复杂度和分支
    double lengthScore = std::min(1.0, skeletonLength / 100.0);
    double complexityScore = std::min(1.0, complexity / 5.0);
    double branchScore = std::min(1.0, branchPoints / 5.0);
    defect.confidence = lengthScore * 0.4 + complexityScore * 0.4 + branchScore * 0.2;
    
    // 严重度
    defect.severity = calculateSeverity(area, skeletonLength, branchPoints);
    
    // 附加属性
    defect.attributes["area"] = area;
    defect.attributes["skeletonLength"] = skeletonLength;
    defect.attributes["branchPoints"] = branchPoints;
    defect.attributes["complexity"] = complexity;
    defect.attributes["method"] = "skeleton";

    defects.push_back(defect);
  }
  
  return defects;
}

std::vector<DefectInfo> CrackDetector::findCracks(const cv::Mat& binary, const cv::Mat& /*original*/) {
  std::vector<DefectInfo> defects;

  // 查找轮廓
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(binary, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  for (size_t i = 0; i < contours.size(); ++i) {
    const auto& contour = contours[i];
    
    if (!isValidCrack(contour)) {
      continue;
    }

    double area = cv::contourArea(contour);
    if (area < m_minArea) {
      continue;
    }

    // 计算轮廓的周长
    double perimeter = cv::arcLength(contour, true);

    // 计算复杂度（周长²/面积）
    double complexity = (perimeter * perimeter) / (4 * CV_PI * area);
    
    // 裂纹通常不是圆形，复杂度应该较高
    if (complexity < 2.0) {
      continue;
    }

    // 拟合最小外接矩形
    cv::RotatedRect rotRect = cv::minAreaRect(contour);
    double length = std::max(rotRect.size.width, rotRect.size.height);
    double width = std::min(rotRect.size.width, rotRect.size.height);

    // 创建缺陷信息
    DefectInfo defect;
    defect.bbox = cv::boundingRect(contour);
    defect.contour = contour;
    defect.classId = 1;
    defect.className = "Crack";
    
    // 置信度基于复杂度和面积
    double complexityScore = std::min(1.0, (complexity - 2.0) / 10.0);
    double areaScore = std::min(1.0, area / 500.0);
    defect.confidence = complexityScore * 0.6 + areaScore * 0.4;
    
    // 严重度
    defect.severity = calculateSeverity(area, length, 0);
    
    // 附加属性
    defect.attributes["area"] = area;
    defect.attributes["perimeter"] = perimeter;
    defect.attributes["complexity"] = complexity;
    defect.attributes["length"] = length;
    defect.attributes["width"] = width;
    defect.attributes["method"] = "contour";

    defects.push_back(defect);
  }

  return defects;
}

bool CrackDetector::isValidCrack(const std::vector<cv::Point>& contour) {
  if (contour.size() < 5) {
    return false;
  }
  return true;
}

double CrackDetector::calculateSeverity(double area, double length, int branchCount) {
  // 面积、长度和分支数越大，严重度越高
  double areaFactor = std::min(1.0, area / 1000.0);
  double lengthFactor = std::min(1.0, length / 100.0);
  double branchFactor = std::min(1.0, branchCount / 5.0);
  
  return areaFactor * 0.3 + lengthFactor * 0.5 + branchFactor * 0.2;
}
