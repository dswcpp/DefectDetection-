#include "ForeignDetector.h"
#include "../common/Logger.h"
#include <QElapsedTimer>

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

  // 1. 灰度异物检测
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

  // NMS 去重
  size_t beforeNMS = allDefects.size();
  allDefects = nmsDefects(allDefects, 0.5);

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
  
  LOG_INFO("ForeignDetector::detect - Result: {} foreign (gray:{}, color:{}), NMS:{}->{}, filter:{}->{}, totalArea={:.0f}px, maxContrast={:.2f}, time:{:.1f}ms",
           allDefects.size(), grayCount, colorCount, beforeNMS, beforeFilter, beforeFilter, allDefects.size(),
           totalArea, maxContrast, timeMs);
  
  return makeSuccessResult(allDefects, timeMs);
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
    
    defects.push_back(defect);
  }
  
  return defects;
}

std::vector<DefectInfo> ForeignDetector::nmsDefects(const std::vector<DefectInfo>& defects, double iouThreshold) {
  if (defects.empty()) return defects;
  
  std::vector<DefectInfo> sorted = defects;
  std::sort(sorted.begin(), sorted.end(), [](const DefectInfo& a, const DefectInfo& b) {
    return a.confidence > b.confidence;
  });
  
  std::vector<bool> suppressed(sorted.size(), false);
  std::vector<DefectInfo> result;
  
  for (size_t i = 0; i < sorted.size(); ++i) {
    if (suppressed[i]) continue;
    result.push_back(sorted[i]);
    
    for (size_t j = i + 1; j < sorted.size(); ++j) {
      if (suppressed[j]) continue;
      
      cv::Rect intersection = sorted[i].bbox & sorted[j].bbox;
      if (intersection.area() == 0) continue;
      
      double iou = static_cast<double>(intersection.area()) / 
                   (sorted[i].bbox.area() + sorted[j].bbox.area() - intersection.area());
      if (iou > iouThreshold) {
        suppressed[j] = true;
      }
    }
  }
  
  return result;
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
