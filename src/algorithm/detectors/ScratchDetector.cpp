#include "ScratchDetector.h"
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
    return makeErrorResult("Empty input image");
  }

  updateParameters();

  // 预处理
  cv::Mat preprocessed = preprocessImage(image);

  std::vector<DefectInfo> allDefects;

  // 多尺度检测
  std::vector<double> scales = {1.0, 0.5, 0.25};
  for (double scale : scales) {
    cv::Mat scaled;
    if (scale < 1.0) {
      cv::resize(preprocessed, scaled, cv::Size(), scale, scale, cv::INTER_AREA);
    } else {
      scaled = preprocessed;
    }

    // 边缘检测（Canny）
    int lowThreshold = std::max(10, 100 - m_sensitivity);
    int highThreshold = lowThreshold * 3;
    
    cv::Mat edges;
    cv::Canny(scaled, edges, lowThreshold, highThreshold);

    // 形态学操作：连接断开的边缘
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 1));
    cv::dilate(edges, edges, kernel);
    cv::erode(edges, edges, kernel);

    // 轮廓检测
    auto contourDefects = findScratches(edges, scaled);
    
    // Hough 线检测（增强长直线划痕检测）
    auto lineDefects = detectLinesHough(edges, scaled);
    
    // 调整坐标回原始尺度
    for (auto& d : contourDefects) {
      if (scale < 1.0) {
        d.bbox.x = static_cast<int>(d.bbox.x / scale);
        d.bbox.y = static_cast<int>(d.bbox.y / scale);
        d.bbox.width = static_cast<int>(d.bbox.width / scale);
        d.bbox.height = static_cast<int>(d.bbox.height / scale);
      }
      allDefects.push_back(d);
    }
    for (auto& d : lineDefects) {
      if (scale < 1.0) {
        d.bbox.x = static_cast<int>(d.bbox.x / scale);
        d.bbox.y = static_cast<int>(d.bbox.y / scale);
        d.bbox.width = static_cast<int>(d.bbox.width / scale);
        d.bbox.height = static_cast<int>(d.bbox.height / scale);
      }
      allDefects.push_back(d);
    }
  }

  // 去重（NMS）
  allDefects = nmsDefects(allDefects, 0.5);

  // 过滤低置信度
  allDefects = filterByConfidence(allDefects);

  double timeMs = timer.elapsed();
  return makeSuccessResult(allDefects, timeMs);
}

std::vector<DefectInfo> ScratchDetector::detectLinesHough(const cv::Mat& edges, const cv::Mat& original) {
  std::vector<DefectInfo> defects;
  
  // 概率 Hough 变换检测线段
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
    defect.attributes["method"] = "hough";
    
    defects.push_back(defect);
  }
  
  return defects;
}

std::vector<DefectInfo> ScratchDetector::nmsDefects(const std::vector<DefectInfo>& defects, double iouThreshold) {
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
      double iou = static_cast<double>(intersection.area()) / 
                   (sorted[i].bbox.area() + sorted[j].bbox.area() - intersection.area());
      if (iou > iouThreshold) {
        suppressed[j] = true;
      }
    }
  }
  
  return result;
}

std::vector<DefectInfo> ScratchDetector::findScratches(const cv::Mat& edges, const cv::Mat& original) {
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
