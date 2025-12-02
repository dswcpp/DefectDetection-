#include "ForeignDetector.h"
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
    return makeErrorResult("Empty input image");
  }

  updateParameters();

  // 预处理
  cv::Mat preprocessed = preprocessImage(image);

  // 计算局部对比度：使用形态学顶帽和底帽变换
  cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(15, 15));
  
  // 顶帽：提取比背景亮的区域（明亮异物）
  cv::Mat tophat;
  cv::morphologyEx(preprocessed, tophat, cv::MORPH_TOPHAT, kernel);
  
  // 底帽：提取比背景暗的区域（暗色异物）
  cv::Mat blackhat;
  cv::morphologyEx(preprocessed, blackhat, cv::MORPH_BLACKHAT, kernel);

  // 合并亮暗异物
  cv::Mat combined;
  cv::add(tophat, blackhat, combined);

  // 二值化
  int threshold = static_cast<int>(255 * m_contrast);
  cv::Mat binary;
  cv::threshold(combined, binary, threshold, 255, cv::THRESH_BINARY);

  // 形态学去噪
  cv::Mat smallKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
  cv::morphologyEx(binary, binary, cv::MORPH_OPEN, smallKernel);

  // 查找异物
  std::vector<DefectInfo> defects = findForeignObjects(binary, image);

  // 过滤低置信度
  defects = filterByConfidence(defects);

  double timeMs = timer.elapsed();
  return makeSuccessResult(defects, timeMs);
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
