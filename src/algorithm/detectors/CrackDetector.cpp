#include "CrackDetector.h"
#include "../DetectorFactory.h"
#include <QElapsedTimer>

// 自动注册到工厂
REGISTER_DETECTOR("crack", CrackDetector)

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
}

cv::Mat CrackDetector::preprocessImage(const cv::Mat& input) {
  cv::Mat gray, blurred, binary;

  // 转灰度
  if (input.channels() == 3) {
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
  } else {
    gray = input.clone();
  }

  // 高斯模糊
  cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);

  // 自适应二值化（更适合检测裂纹）
  cv::adaptiveThreshold(blurred, binary, 255, 
                        cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                        cv::THRESH_BINARY_INV, 11, 2);

  // 形态学操作：闭运算填充小孔
  int kernelSize = m_morphKernelSize | 1;  // 确保奇数
  cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, 
                                              cv::Size(kernelSize, kernelSize));
  cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, kernel);

  // 细化骨架（可选，用于提取裂纹中心线）
  // cv::ximgproc::thinning(binary, binary);

  return binary;
}

DetectionResult CrackDetector::detect(const cv::Mat& image) {
  QElapsedTimer timer;
  timer.start();

  if (image.empty()) {
    return makeErrorResult("Empty input image");
  }

  updateParameters();

  // 预处理
  cv::Mat binary = preprocessImage(image);

  // 查找裂纹
  std::vector<DefectInfo> defects = findCracks(binary, image);

  // 过滤低置信度
  defects = filterByConfidence(defects);

  double timeMs = timer.elapsed();
  return makeSuccessResult(defects, timeMs);
}

std::vector<DefectInfo> CrackDetector::findCracks(const cv::Mat& binary, const cv::Mat& original) {
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

    // 计算轮廓的周长（弧长）
    double perimeter = cv::arcLength(contour, true);

    // 计算复杂度（周长²/面积），裂纹通常复杂度较高
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
    defect.severity = calculateSeverity(area, length);
    
    // 附加属性
    defect.attributes["area"] = area;
    defect.attributes["perimeter"] = perimeter;
    defect.attributes["complexity"] = complexity;
    defect.attributes["length"] = length;
    defect.attributes["width"] = width;

    defects.push_back(defect);
  }

  return defects;
}

bool CrackDetector::isValidCrack(const std::vector<cv::Point>& contour) {
  // 最小点数
  if (contour.size() < 5) {
    return false;
  }

  return true;
}

double CrackDetector::calculateSeverity(double area, double length) {
  // 面积和长度越大，严重度越高
  double areaFactor = std::min(1.0, area / 1000.0);
  double lengthFactor = std::min(1.0, length / 100.0);
  
  return areaFactor * 0.5 + lengthFactor * 0.5;
}
