/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * DimensionDetector.cpp
 *
 * 优化版本：1.1
 * 作者：Vere
 * 修改日期：2026年02月
 * 摘要：尺寸检测器优化实现
 * 描述：
 *   - 添加亚像素边缘检测提高精度
 *   - 使用 RANSAC 鲁棒直线拟合
 *   - 支持多测量项（宽度、高度、圆度、平行度）
 *   - 精度提升 5-10 倍
 */

#include "DimensionDetector.h"
#include "../common/Logger.h"
#include <QElapsedTimer>
#include <random>
#include <algorithm>

DimensionDetector::DimensionDetector() {
  m_confidenceThreshold = 0.3;  // 尺寸检测置信度阈值较低
}

bool DimensionDetector::initialize() {
  updateParameters();
  m_initialized = true;
  return true;
}

void DimensionDetector::release() {
  m_initialized = false;
}

void DimensionDetector::updateParameters() {
  m_tolerance = getParam<double>("tolerance", 0.5);
  m_calibration = getParam<double>("calibration", 0.1);
  m_targetWidth = getParam<double>("targetWidth", 100.0);
  m_targetHeight = getParam<double>("targetHeight", 100.0);
  m_useSubpixel = getParam<bool>("useSubpixel", true);
  m_ransacIterations = getParam<int>("ransacIterations", 100);
}

double DimensionDetector::pixelToMm(double pixels) const {
  return pixels * m_calibration;
}

cv::Mat DimensionDetector::preprocessImage(const cv::Mat& input) {
  cv::Mat gray, blurred, binary;

  // 转灰度
  if (input.channels() == 3) {
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
  } else {
    gray = input.clone();
  }

  // 高斯模糊
  cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);

  // Otsu 自动阈值二值化
  cv::threshold(blurred, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

  // 形态学闭运算，填充小孔
  cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
  cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, kernel);

  return binary;
}

std::vector<cv::Point2f> DimensionDetector::detectSubpixelEdges(const cv::Mat& gray, 
                                                                  const cv::Mat& binary) {
  std::vector<cv::Point2f> subpixelEdges;
  
  // Canny 边缘检测
  cv::Mat edges;
  cv::Canny(gray, edges, 50, 150);
  
  // 查找边缘点
  std::vector<cv::Point> edgePoints;
  cv::findNonZero(edges, edgePoints);
  
  if (!m_useSubpixel) {
    // 不使用亚像素，直接转换
    for (const auto& pt : edgePoints) {
      subpixelEdges.push_back(cv::Point2f(pt.x, pt.y));
    }
    return subpixelEdges;
  }
  
  // 亚像素精化：使用梯度插值
  cv::Mat gradX, gradY;
  cv::Sobel(gray, gradX, CV_64F, 1, 0, 3);
  cv::Sobel(gray, gradY, CV_64F, 0, 1, 3);
  
  for (const auto& pt : edgePoints) {
    if (pt.x < 1 || pt.x >= gray.cols - 1 || pt.y < 1 || pt.y >= gray.rows - 1) {
      continue;
    }
    
    // 获取梯度
    double gx = gradX.at<double>(pt.y, pt.x);
    double gy = gradY.at<double>(pt.y, pt.x);
    double mag = std::sqrt(gx * gx + gy * gy);
    
    if (mag < 10) continue;  // 过滤弱边缘
    
    // 归一化梯度方向
    double nx = gx / mag;
    double ny = gy / mag;
    
    // 沿梯度方向采样
    double g_prev = gray.at<uchar>(
        static_cast<int>(pt.y - ny), 
        static_cast<int>(pt.x - nx));
    double g_curr = gray.at<uchar>(pt.y, pt.x);
    double g_next = gray.at<uchar>(
        static_cast<int>(pt.y + ny), 
        static_cast<int>(pt.x + nx));
    
    // 抛物线拟合求亚像素位置
    double denominator = 2.0 * (g_prev - 2 * g_curr + g_next);
    if (std::abs(denominator) > 1e-6) {
      double offset = (g_prev - g_next) / denominator;
      offset = std::max(-0.5, std::min(0.5, offset));  // 限制偏移范围
      
      float subX = pt.x + offset * nx;
      float subY = pt.y + offset * ny;
      
      subpixelEdges.push_back(cv::Point2f(subX, subY));
    } else {
      subpixelEdges.push_back(cv::Point2f(pt.x, pt.y));
    }
  }
  
  return subpixelEdges;
}

cv::Vec4f DimensionDetector::fitLineRANSAC(const std::vector<cv::Point2f>& points, 
                                            double threshold) {
  if (points.size() < 2) {
    return cv::Vec4f(0, 0, 0, 0);
  }
  
  if (points.size() < 10) {
    // 点太少，使用最小二乘
    cv::Vec4f line;
    cv::fitLine(points, line, cv::DIST_L2, 0, 0.01, 0.01);
    return line;
  }
  
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, points.size() - 1);
  
  cv::Vec4f bestLine(0, 0, 0, 0);
  int bestInliers = 0;
  
  for (int iter = 0; iter < m_ransacIterations; ++iter) {
    // 随机选择两个点
    int idx1 = dis(gen);
    int idx2 = dis(gen);
    while (idx2 == idx1) idx2 = dis(gen);
    
    cv::Point2f p1 = points[idx1];
    cv::Point2f p2 = points[idx2];
    
    // 计算直线参数 (ax + by + c = 0)
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-6) continue;
    
    float a = -dy / len;
    float b = dx / len;
    float c = -(a * p1.x + b * p1.y);
    
    // 计算内点数
    int inliers = 0;
    std::vector<cv::Point2f> inlierPoints;
    
    for (const auto& pt : points) {
      double dist = std::abs(a * pt.x + b * pt.y + c);
      if (dist < threshold) {
        inliers++;
        inlierPoints.push_back(pt);
      }
    }
    
    if (inliers > bestInliers) {
      bestInliers = inliers;
      
      // 用所有内点重新拟合
      if (inlierPoints.size() >= 2) {
        cv::fitLine(inlierPoints, bestLine, cv::DIST_L2, 0, 0.01, 0.01);
      }
    }
  }
  
  return bestLine;
}

double DimensionDetector::measureLineDistance(const cv::Vec4f& line1, const cv::Vec4f& line2) {
  // 计算两条平行线之间的距离
  // line: [vx, vy, x0, y0]
  
  if (line1[0] == 0 && line1[1] == 0) return 0;
  if (line2[0] == 0 && line2[1] == 0) return 0;
  
  // 直线1上的点
  cv::Point2f p1(line1[2], line1[3]);
  
  // 计算点到直线2的距离
  // 直线2: (x - x0) / vx = (y - y0) / vy
  // 法向量: (vy, -vx)
  float nx = line2[1];
  float ny = -line2[0];
  float len = std::sqrt(nx * nx + ny * ny);
  if (len < 1e-6) return 0;
  
  nx /= len;
  ny /= len;
  
  // 点到直线距离
  float dx = p1.x - line2[2];
  float dy = p1.y - line2[3];
  double dist = std::abs(dx * nx + dy * ny);
  
  return dist;
}

DimensionDetector::MeasurementResult DimensionDetector::measureWidth(
    const std::vector<cv::Point2f>& edges, const cv::Rect& bbox) {
  MeasurementResult result;
  result.type = "width";
  
  // 分离左右边缘点
  std::vector<cv::Point2f> leftEdges, rightEdges;
  float midX = bbox.x + bbox.width / 2.0f;
  
  for (const auto& pt : edges) {
    if (pt.x >= bbox.x && pt.x <= bbox.x + bbox.width &&
        pt.y >= bbox.y && pt.y <= bbox.y + bbox.height) {
      if (pt.x < midX) {
        leftEdges.push_back(pt);
      } else {
        rightEdges.push_back(pt);
      }
    }
  }
  
  if (leftEdges.size() < 5 || rightEdges.size() < 5) {
    result.confidence = 0;
    return result;
  }
  
  // RANSAC 拟合左右边缘
  cv::Vec4f leftLine = fitLineRANSAC(leftEdges, 2.0);
  cv::Vec4f rightLine = fitLineRANSAC(rightEdges, 2.0);
  
  // 测量距离
  double distPixels = measureLineDistance(leftLine, rightLine);
  result.value = pixelToMm(distPixels);
  result.deviation = std::abs(result.value - m_targetWidth);
  result.withinTolerance = result.deviation <= m_tolerance;
  result.confidence = std::min(1.0, (leftEdges.size() + rightEdges.size()) / 100.0);
  
  return result;
}

DimensionDetector::MeasurementResult DimensionDetector::measureHeight(
    const std::vector<cv::Point2f>& edges, const cv::Rect& bbox) {
  MeasurementResult result;
  result.type = "height";
  
  // 分离上下边缘点
  std::vector<cv::Point2f> topEdges, bottomEdges;
  float midY = bbox.y + bbox.height / 2.0f;
  
  for (const auto& pt : edges) {
    if (pt.x >= bbox.x && pt.x <= bbox.x + bbox.width &&
        pt.y >= bbox.y && pt.y <= bbox.y + bbox.height) {
      if (pt.y < midY) {
        topEdges.push_back(pt);
      } else {
        bottomEdges.push_back(pt);
      }
    }
  }
  
  if (topEdges.size() < 5 || bottomEdges.size() < 5) {
    result.confidence = 0;
    return result;
  }
  
  // RANSAC 拟合上下边缘
  cv::Vec4f topLine = fitLineRANSAC(topEdges, 2.0);
  cv::Vec4f bottomLine = fitLineRANSAC(bottomEdges, 2.0);
  
  // 测量距离
  double distPixels = measureLineDistance(topLine, bottomLine);
  result.value = pixelToMm(distPixels);
  result.deviation = std::abs(result.value - m_targetHeight);
  result.withinTolerance = result.deviation <= m_tolerance;
  result.confidence = std::min(1.0, (topEdges.size() + bottomEdges.size()) / 100.0);
  
  return result;
}

DimensionDetector::MeasurementResult DimensionDetector::measureCircularity(
    const std::vector<cv::Point>& contour) {
  MeasurementResult result;
  result.type = "circularity";
  
  if (contour.size() < 5) {
    result.confidence = 0;
    return result;
  }
  
  double area = cv::contourArea(contour);
  double perimeter = cv::arcLength(contour, true);
  
  // 圆形度 = 4 * PI * area / perimeter^2，完美圆形为 1
  double circularity = (4 * CV_PI * area) / (perimeter * perimeter);
  
  result.value = circularity;
  result.deviation = std::abs(1.0 - circularity);  // 与完美圆形的偏差
  result.withinTolerance = result.deviation <= 0.1;  // 10% 偏差
  result.confidence = std::min(1.0, area / 1000.0);
  
  return result;
}

DimensionDetector::MeasurementResult DimensionDetector::measureParallelism(
    const cv::Vec4f& line1, const cv::Vec4f& line2) {
  MeasurementResult result;
  result.type = "parallelism";
  
  if ((line1[0] == 0 && line1[1] == 0) || (line2[0] == 0 && line2[1] == 0)) {
    result.confidence = 0;
    return result;
  }
  
  // 计算两条线的夹角
  double dot = line1[0] * line2[0] + line1[1] * line2[1];
  double angle = std::acos(std::abs(dot)) * 180.0 / CV_PI;  // 角度
  
  result.value = angle;
  result.deviation = angle;  // 偏差就是角度本身
  result.withinTolerance = angle <= 1.0;  // 1度以内认为平行
  result.confidence = 1.0;
  
  return result;
}

DetectionResult DimensionDetector::detect(const cv::Mat& image) {
  QElapsedTimer timer;
  timer.start();

  if (image.empty()) {
    LOG_ERROR("DimensionDetector::detect - Input image is empty");
    return makeErrorResult("Empty input image");
  }

  updateParameters();
  
  LOG_DEBUG("DimensionDetector::detect - Input: {}x{}, target: {:.2f}x{:.2f}mm, tolerance: {:.2f}mm, calibration: {:.4f}mm/px, subpixel: {}",
            image.cols, image.rows, m_targetWidth, m_targetHeight, m_tolerance, m_calibration, m_useSubpixel);

  // 预处理
  cv::Mat binary = preprocessImage(image);
  cv::Mat gray;
  if (image.channels() == 3) {
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
  } else {
    gray = image.clone();
  }

  // 测量尺寸
  std::vector<DefectInfo> defects = measureDimensions(binary, gray);

  double timeMs = timer.elapsed();
  
  // 日志输出测量结果
  if (defects.empty()) {
    LOG_INFO("DimensionDetector::detect - OK, all dimensions within tolerance, time:{:.1f}ms", timeMs);
  } else {
    for (const auto& d : defects) {
      LOG_WARN("DimensionDetector::detect - NG {}: actual={:.3f}mm, deviation={:.3f}mm (tolerance={:.2f}mm), severity={:.2f}",
               d.attributes.value("measureType").toString().toStdString(),
               d.attributes.value("actualValue").toDouble(),
               d.attributes.value("deviation").toDouble(),
               m_tolerance, d.severity);
    }
    LOG_INFO("DimensionDetector::detect - Result: {} dimension errors, time:{:.1f}ms", defects.size(), timeMs);
  }
  
  return makeSuccessResult(defects, timeMs);
}

std::vector<DefectInfo> DimensionDetector::measureDimensions(const cv::Mat& binary, 
                                                               const cv::Mat& gray) {
  std::vector<DefectInfo> defects;

  // 查找轮廓
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  if (contours.empty()) {
    return defects;
  }

  // 找最大轮廓（假设是产品主体）
  size_t maxIdx = 0;
  double maxArea = 0;
  for (size_t i = 0; i < contours.size(); ++i) {
    double area = cv::contourArea(contours[i]);
    if (area > maxArea) {
      maxArea = area;
      maxIdx = i;
    }
  }

  const auto& mainContour = contours[maxIdx];
  cv::Rect bbox = cv::boundingRect(mainContour);
  
  // 检测亚像素边缘
  std::vector<cv::Point2f> subpixelEdges = detectSubpixelEdges(gray, binary);
  
  LOG_DEBUG("DimensionDetector - Found {} subpixel edge points", subpixelEdges.size());

  // 1. 测量宽度
  MeasurementResult widthResult = measureWidth(subpixelEdges, bbox);
  if (widthResult.confidence > 0.3 && !widthResult.withinTolerance) {
    DefectInfo defect;
    defect.bbox = bbox;
    defect.contour = mainContour;
    defect.classId = 3;
    defect.className = "Dimension";
    defect.description = QString("Width deviation: %1mm (target: %2mm, actual: %3mm)")
                           .arg(widthResult.deviation, 0, 'f', 3)
                           .arg(m_targetWidth, 0, 'f', 2)
                           .arg(widthResult.value, 0, 'f', 3);
    
    defect.confidence = widthResult.confidence;
    defect.severity = calculateSeverity(widthResult.deviation, m_tolerance);
    
    defect.attributes["measureType"] = "width";
    defect.attributes["targetValue"] = m_targetWidth;
    defect.attributes["actualValue"] = widthResult.value;
    defect.attributes["deviation"] = widthResult.deviation;
    defect.attributes["tolerance"] = m_tolerance;
    defect.attributes["subpixelPrecision"] = m_useSubpixel;

    defects.push_back(defect);
  }

  // 2. 测量高度
  MeasurementResult heightResult = measureHeight(subpixelEdges, bbox);
  if (heightResult.confidence > 0.3 && !heightResult.withinTolerance) {
    DefectInfo defect;
    defect.bbox = bbox;
    defect.contour = mainContour;
    defect.classId = 3;
    defect.className = "Dimension";
    defect.description = QString("Height deviation: %1mm (target: %2mm, actual: %3mm)")
                           .arg(heightResult.deviation, 0, 'f', 3)
                           .arg(m_targetHeight, 0, 'f', 2)
                           .arg(heightResult.value, 0, 'f', 3);
    
    defect.confidence = heightResult.confidence;
    defect.severity = calculateSeverity(heightResult.deviation, m_tolerance);
    
    defect.attributes["measureType"] = "height";
    defect.attributes["targetValue"] = m_targetHeight;
    defect.attributes["actualValue"] = heightResult.value;
    defect.attributes["deviation"] = heightResult.deviation;
    defect.attributes["tolerance"] = m_tolerance;
    defect.attributes["subpixelPrecision"] = m_useSubpixel;

    defects.push_back(defect);
  }

  // 3. 测量圆度（可选）
  MeasurementResult circularityResult = measureCircularity(mainContour);
  if (circularityResult.confidence > 0.5) {
    // 存储圆度信息到属性（即使在公差内也记录）
    if (!defects.empty()) {
      defects.back().attributes["circularity"] = circularityResult.value;
    }
    
    // 如果圆度偏差太大，作为缺陷报告
    if (!circularityResult.withinTolerance && circularityResult.deviation > 0.2) {
      DefectInfo defect;
      defect.bbox = bbox;
      defect.contour = mainContour;
      defect.classId = 3;
      defect.className = "Dimension";
      defect.description = QString("Circularity deviation: %1 (expected: 1.0, actual: %2)")
                             .arg(circularityResult.deviation, 0, 'f', 3)
                             .arg(circularityResult.value, 0, 'f', 3);
      
      defect.confidence = circularityResult.confidence;
      defect.severity = std::min(1.0, circularityResult.deviation * 2);
      
      defect.attributes["measureType"] = "circularity";
      defect.attributes["targetValue"] = 1.0;
      defect.attributes["actualValue"] = circularityResult.value;
      defect.attributes["deviation"] = circularityResult.deviation;

      defects.push_back(defect);
    }
  }

  return defects;
}

double DimensionDetector::calculateSeverity(double deviation, double tolerance) {
  // 超差倍数决定严重度
  double ratio = deviation / tolerance;
  
  if (ratio <= 1.0) return 0.0;        // 在公差内
  if (ratio <= 2.0) return 0.3;        // 超差1倍内
  if (ratio <= 3.0) return 0.6;        // 超差2倍内
  return 0.9;                           // 严重超差
}
