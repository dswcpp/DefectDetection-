/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ScratchDetector.cpp
 *
 * 优化版本：1.1
 * 作者：Vere
 * 修改日期：2026年02月
 * 摘要：划痕检测器优化实现
 * 描述：
 *   - 使用 LSD 线段检测替代 HoughLinesP（更精确）
 *   - 统一使用 NMSFilter 进行去重
 *   - 添加灰度剖面分析提高宽度测量精度
 *   - 优化多尺度检测策略
 */

#include "ScratchDetector.h"
#include "../postprocess/NMSFilter.h"
#include "../common/Logger.h"
#include <QElapsedTimer>

ScratchDetector::ScratchDetector() {
  m_confidenceThreshold = 0.5;
}

bool ScratchDetector::initialize() {
  updateParameters();
  m_initialized = true;
  return true;
}

void ScratchDetector::release() {
  m_initialized = false;
}

void ScratchDetector::updateParameters() {
  m_sensitivity = getParam<int>("sensitivity", 75);
  m_minLength = getParam<int>("minLength", 10);
  m_maxWidth = getParam<int>("maxWidth", 5);
  m_contrastThreshold = getParam<int>("contrastThreshold", 30);
}

cv::Mat ScratchDetector::preprocessImage(const cv::Mat& input) {
  cv::Mat gray, blurred;

  // 转灰度
  if (input.channels() == 3) {
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
  } else {
    gray = input.clone();
  }

  // 高斯模糊去噪
  int kernelSize = 3;
  cv::GaussianBlur(gray, blurred, cv::Size(kernelSize, kernelSize), 0);

  return blurred;
}

DetectionResult ScratchDetector::detect(const cv::Mat& image) {
  QElapsedTimer timer;
  timer.start();

  if (image.empty()) {
    LOG_ERROR("ScratchDetector::detect - Input image is empty");
    return makeErrorResult("Empty input image");
  }

  updateParameters();
  
  LOG_DEBUG("ScratchDetector::detect - Input: {}x{}, channels={}, params: sensitivity={}, minLength={}, maxWidth={}", 
            image.cols, image.rows, image.channels(), m_sensitivity, m_minLength, m_maxWidth);

  // 预处理
  cv::Mat preprocessed = preprocessImage(image);

  std::vector<DefectInfo> allDefects;
  size_t lsdCount = 0, contourCount = 0;

  // 多尺度检测
  std::vector<double> scales = {1.0, 0.5};
  for (double scale : scales) {
    cv::Mat scaled;
    if (scale < 1.0) {
      cv::resize(preprocessed, scaled, cv::Size(), scale, scale, cv::INTER_AREA);
    } else {
      scaled = preprocessed;
    }

    // 1. LSD 线段检测（主要方法，更精确）
    auto lsdDefects = detectLinesLSD(scaled, image);
    lsdCount += lsdDefects.size();
    
    // 2. Canny + 轮廓检测（补充方法，检测弯曲划痕）
    int lowThreshold = std::max(10, 100 - m_sensitivity);
    int highThreshold = lowThreshold * 3;
    cv::Mat edges;
    cv::Canny(scaled, edges, lowThreshold, highThreshold);

    // 形态学操作：连接断开的边缘
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 1));
    cv::dilate(edges, edges, kernel);
    cv::erode(edges, edges, kernel);

    auto contourDefects = findScratches(edges, scaled);
    contourCount += contourDefects.size();

    // 调整坐标回原始尺度
    auto adjustCoords = [scale](std::vector<DefectInfo>& defects) {
      if (scale < 1.0) {
        for (auto& d : defects) {
          d.bbox.x = static_cast<int>(d.bbox.x / scale);
          d.bbox.y = static_cast<int>(d.bbox.y / scale);
          d.bbox.width = static_cast<int>(d.bbox.width / scale);
          d.bbox.height = static_cast<int>(d.bbox.height / scale);
          // 调整轮廓点
          for (auto& pt : d.contour) {
            pt.x = static_cast<int>(pt.x / scale);
            pt.y = static_cast<int>(pt.y / scale);
          }
        }
      }
    };
    
    adjustCoords(lsdDefects);
    adjustCoords(contourDefects);
    
    allDefects.insert(allDefects.end(), lsdDefects.begin(), lsdDefects.end());
    allDefects.insert(allDefects.end(), contourDefects.begin(), contourDefects.end());
  }

  // 对每个缺陷进行灰度剖面分析（精确测量宽度）
  for (auto& defect : allDefects) {
    analyzeGrayProfile(defect, preprocessed);
  }

  // 使用统一的 NMSFilter 去重
  size_t beforeNMS = allDefects.size();
  NMSFilter nmsFilter;
  nmsFilter.setIoUThreshold(0.5);
  nmsFilter.setConfidenceThreshold(0.0);  // 先不过滤置信度
  allDefects = nmsFilter.filter(allDefects);

  // 过滤低置信度
  size_t beforeFilter = allDefects.size();
  allDefects = filterByConfidence(allDefects);

  double timeMs = timer.elapsed();
  
  // 计算置信度统计
  double minConf = 1.0, maxConf = 0.0, avgConf = 0.0;
  for (const auto& d : allDefects) {
    minConf = std::min(minConf, d.confidence);
    maxConf = std::max(maxConf, d.confidence);
    avgConf += d.confidence;
  }
  if (!allDefects.empty()) avgConf /= allDefects.size();
  
  LOG_INFO("ScratchDetector::detect - Result: {} defects (LSD:{}, contour:{}), NMS:{}->{}, filter:{}->{}, conf:[{:.2f},{:.2f}], time:{:.1f}ms",
           allDefects.size(), lsdCount, contourCount, 
           beforeNMS, beforeFilter, beforeFilter, allDefects.size(),
           allDefects.empty() ? 0.0 : minConf, maxConf, timeMs);
  
  return makeSuccessResult(allDefects, timeMs);
}

std::vector<DefectInfo> ScratchDetector::detectLinesLSD(const cv::Mat& gray, const cv::Mat& /*original*/) {
  std::vector<DefectInfo> defects;
  
  // 创建 LSD 检测器
  cv::Ptr<cv::LineSegmentDetector> lsd = cv::createLineSegmentDetector(
      cv::LSD_REFINE_STD,  // 精细化模式
      0.8,                  // scale
      0.6,                  // sigma_scale
      2.0,                  // quant
      22.5,                 // ang_th
      0,                    // log_eps
      0.7,                  // density_th
      1024                  // n_bins
  );
  
  // 检测线段
  std::vector<cv::Vec4f> lines;
  std::vector<double> widths, precs, nfas;
  lsd->detect(gray, lines, widths, precs, nfas);
  
  for (size_t i = 0; i < lines.size(); ++i) {
    const auto& line = lines[i];
    float x1 = line[0], y1 = line[1], x2 = line[2], y2 = line[3];
    
    double length = std::sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
    if (length < m_minLength) continue;
    
    // LSD 提供线宽估计
    double lineWidth = (i < widths.size()) ? widths[i] : 1.0;
    if (lineWidth > m_maxWidth) continue;
    
    // 计算方向角度
    double angle = std::atan2(y2 - y1, x2 - x1) * 180.0 / CV_PI;
    if (angle < 0) angle += 180.0;
    
    DefectInfo defect;
    defect.bbox = cv::Rect(
      static_cast<int>(std::min(x1, x2)),
      static_cast<int>(std::min(y1, y2)),
      static_cast<int>(std::abs(x2 - x1)) + 1,
      static_cast<int>(std::abs(y2 - y1)) + 1
    );
    
    // 确保 bbox 有效
    defect.bbox.width = std::max(defect.bbox.width, 1);
    defect.bbox.height = std::max(defect.bbox.height, 1);
    
    // 存储线段端点作为轮廓
    defect.contour = {
      cv::Point(static_cast<int>(x1), static_cast<int>(y1)),
      cv::Point(static_cast<int>(x2), static_cast<int>(y2))
    };
    
    defect.classId = 0;
    defect.className = "Scratch";
    
    // 置信度基于 LSD 的 NFA 和长度
    double nfa = (i < nfas.size()) ? nfas[i] : 0.0;
    double lengthScore = std::min(1.0, length / 100.0);
    double nfaScore = std::min(1.0, std::max(0.0, -nfa / 10.0));  // NFA 越小越好
    defect.confidence = lengthScore * 0.6 + nfaScore * 0.4;
    
    defect.severity = calculateSeverity(length, lineWidth);
    
    defect.attributes["length"] = length;
    defect.attributes["width"] = lineWidth;
    defect.attributes["angle"] = angle;
    defect.attributes["method"] = "LSD";
    if (i < nfas.size()) defect.attributes["nfa"] = nfas[i];
    
    defects.push_back(defect);
  }
  
  return defects;
}

std::vector<DefectInfo> ScratchDetector::detectLinesHough(const cv::Mat& edges, const cv::Mat& /*original*/) {
  std::vector<DefectInfo> defects;
  
  // 概率 Hough 变换检测线段（作为备用方法）
  std::vector<cv::Vec4i> lines;
  cv::HoughLinesP(edges, lines, 1, CV_PI / 180, 50, m_minLength, 10);
  
  for (const auto& line : lines) {
    int x1 = line[0], y1 = line[1], x2 = line[2], y2 = line[3];
    
    double length = std::sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
    if (length < m_minLength) continue;
    
    DefectInfo defect;
    defect.bbox = cv::Rect(
      std::min(x1, x2), std::min(y1, y2),
      std::abs(x2 - x1) + 1, std::abs(y2 - y1) + 1
    );
    defect.classId = 0;
    defect.className = "Scratch";
    defect.confidence = std::min(1.0, length / 150.0);
    defect.severity = std::min(1.0, length / 200.0);
    defect.attributes["length"] = length;
    defect.attributes["method"] = "Hough";
    
    defects.push_back(defect);
  }
  
  return defects;
}

void ScratchDetector::analyzeGrayProfile(DefectInfo& defect, const cv::Mat& gray) {
  if (defect.contour.size() < 2) return;
  
  // 获取线段的两个端点
  cv::Point p1 = defect.contour[0];
  cv::Point p2 = defect.contour[1];
  
  // 计算垂直于线段的方向
  double dx = p2.x - p1.x;
  double dy = p2.y - p1.y;
  double len = std::sqrt(dx*dx + dy*dy);
  if (len < 1) return;
  
  // 归一化垂直方向
  double perpX = -dy / len;
  double perpY = dx / len;
  
  // 沿线段采样多个位置
  int numSamples = std::min(10, static_cast<int>(len / 5) + 1);
  std::vector<double> widthMeasurements;
  
  for (int i = 0; i < numSamples; ++i) {
    double t = (numSamples > 1) ? static_cast<double>(i) / (numSamples - 1) : 0.5;
    int cx = static_cast<int>(p1.x + t * dx);
    int cy = static_cast<int>(p1.y + t * dy);
    
    // 沿垂直方向采样灰度值
    std::vector<uchar> profile;
    int halfWidth = 10;  // 采样半宽度
    
    for (int j = -halfWidth; j <= halfWidth; ++j) {
      int px = cx + static_cast<int>(j * perpX);
      int py = cy + static_cast<int>(j * perpY);
      
      if (px >= 0 && px < gray.cols && py >= 0 && py < gray.rows) {
        profile.push_back(gray.at<uchar>(py, px));
      }
    }
    
    if (profile.size() < 5) continue;
    
    // 计算背景灰度（两端平均）
    double leftBg = 0, rightBg = 0;
    int bgSamples = std::min(3, static_cast<int>(profile.size() / 4));
    for (int j = 0; j < bgSamples; ++j) {
      leftBg += profile[j];
      rightBg += profile[profile.size() - 1 - j];
    }
    leftBg /= bgSamples;
    rightBg /= bgSamples;
    double bgValue = (leftBg + rightBg) / 2.0;
    
    // 寻找划痕宽度（灰度显著偏离背景的区域）
    double threshold = bgValue * 0.8;  // 20% 偏差
    int startIdx = -1, endIdx = -1;
    
    for (size_t j = 0; j < profile.size(); ++j) {
      bool isDark = profile[j] < threshold;
      if (isDark && startIdx < 0) startIdx = j;
      if (isDark) endIdx = j;
    }
    
    if (startIdx >= 0 && endIdx > startIdx) {
      widthMeasurements.push_back(endIdx - startIdx);
    }
  }
  
  // 计算平均宽度
  if (!widthMeasurements.empty()) {
    double avgWidth = 0;
    for (double w : widthMeasurements) avgWidth += w;
    avgWidth /= widthMeasurements.size();
    
    defect.attributes["measuredWidth"] = avgWidth;
    
    // 更新严重度
    double length = defect.attributes.value("length", 0.0).toDouble();
    defect.severity = calculateSeverity(length, avgWidth);
  }
}

std::vector<DefectInfo> ScratchDetector::findScratches(const cv::Mat& edges, const cv::Mat& /*original*/) {
  std::vector<DefectInfo> defects;

  // 查找轮廓
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  for (const auto& contour : contours) {
    if (!isValidScratch(contour)) {
      continue;
    }

    // 拟合最小外接矩形
    cv::RotatedRect rotRect = cv::minAreaRect(contour);
    
    // 获取长宽（长边为长度，短边为宽度）
    double length = std::max(rotRect.size.width, rotRect.size.height);
    double width = std::min(rotRect.size.width, rotRect.size.height);

    // 检查是否符合划痕特征（细长）
    if (length < m_minLength || width > m_maxWidth) {
      continue;
    }

    // 计算长宽比
    double aspectRatio = length / std::max(1.0, width);
    if (aspectRatio < 3.0) {
      // 不够细长，不是划痕
      continue;
    }

    // 创建缺陷信息
    DefectInfo defect;
    defect.bbox = cv::boundingRect(contour);
    defect.contour = contour;
    defect.classId = 0;
    defect.className = "Scratch";
    
    // 置信度基于长宽比和长度
    defect.confidence = std::min(1.0, aspectRatio / 10.0) * 0.5 + 
                        std::min(1.0, length / 100.0) * 0.5;
    
    // 严重度
    defect.severity = calculateSeverity(length, width);
    
    // 附加属性
    defect.attributes["length"] = length;
    defect.attributes["width"] = width;
    defect.attributes["aspectRatio"] = aspectRatio;
    defect.attributes["angle"] = rotRect.angle;
    defect.attributes["method"] = "contour";

    defects.push_back(defect);
  }

  return defects;
}

bool ScratchDetector::isValidScratch(const std::vector<cv::Point>& contour) {
  // 最小点数
  if (contour.size() < 5) {
    return false;
  }

  // 最小面积
  double area = cv::contourArea(contour);
  if (area < 10) {
    return false;
  }

  return true;
}

double ScratchDetector::calculateSeverity(double length, double avgWidth) {
  // 根据长度和宽度计算严重度
  // 越长、越宽的划痕越严重
  double lengthFactor = std::min(1.0, length / 200.0);
  double widthFactor = std::min(1.0, avgWidth / 10.0);
  
  return lengthFactor * 0.7 + widthFactor * 0.3;
}
