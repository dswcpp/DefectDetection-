#include "DimensionDetector.h"
#include "../DetectorFactory.h"
#include <QElapsedTimer>

// 自动注册到工厂
REGISTER_DETECTOR("dimension", DimensionDetector)

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

DetectionResult DimensionDetector::detect(const cv::Mat& image) {
  QElapsedTimer timer;
  timer.start();

  if (image.empty()) {
    return makeErrorResult("Empty input image");
  }

  updateParameters();

  // 预处理
  cv::Mat binary = preprocessImage(image);

  // 测量尺寸
  std::vector<DefectInfo> defects = measureDimensions(binary, image);

  double timeMs = timer.elapsed();
  return makeSuccessResult(defects, timeMs);
}

std::vector<DefectInfo> DimensionDetector::measureDimensions(const cv::Mat& binary, const cv::Mat& original) {
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

  // 最小外接矩形
  cv::RotatedRect rotRect = cv::minAreaRect(mainContour);
  
  // 获取宽高（像素）
  double widthPx = std::max(rotRect.size.width, rotRect.size.height);
  double heightPx = std::min(rotRect.size.width, rotRect.size.height);

  // 转换为毫米
  double widthMm = pixelToMm(widthPx);
  double heightMm = pixelToMm(heightPx);

  // 计算偏差
  double widthDeviation = std::abs(widthMm - m_targetWidth);
  double heightDeviation = std::abs(heightMm - m_targetHeight);

  // 检查宽度是否超差
  if (widthDeviation > m_tolerance) {
    DefectInfo defect;
    defect.bbox = cv::boundingRect(mainContour);
    defect.contour = mainContour;
    defect.classId = 3;
    defect.className = "Dimension";
    defect.description = QString("Width deviation: %1mm (target: %2mm, actual: %3mm)")
                           .arg(widthDeviation, 0, 'f', 2)
                           .arg(m_targetWidth, 0, 'f', 2)
                           .arg(widthMm, 0, 'f', 2);
    
    defect.confidence = std::min(1.0, widthDeviation / (m_tolerance * 3));
    defect.severity = calculateSeverity(widthDeviation, m_tolerance);
    
    defect.attributes["measureType"] = "width";
    defect.attributes["targetValue"] = m_targetWidth;
    defect.attributes["actualValue"] = widthMm;
    defect.attributes["deviation"] = widthDeviation;
    defect.attributes["tolerance"] = m_tolerance;

    defects.push_back(defect);
  }

  // 检查高度是否超差
  if (heightDeviation > m_tolerance) {
    DefectInfo defect;
    defect.bbox = cv::boundingRect(mainContour);
    defect.contour = mainContour;
    defect.classId = 3;
    defect.className = "Dimension";
    defect.description = QString("Height deviation: %1mm (target: %2mm, actual: %3mm)")
                           .arg(heightDeviation, 0, 'f', 2)
                           .arg(m_targetHeight, 0, 'f', 2)
                           .arg(heightMm, 0, 'f', 2);
    
    defect.confidence = std::min(1.0, heightDeviation / (m_tolerance * 3));
    defect.severity = calculateSeverity(heightDeviation, m_tolerance);
    
    defect.attributes["measureType"] = "height";
    defect.attributes["targetValue"] = m_targetHeight;
    defect.attributes["actualValue"] = heightMm;
    defect.attributes["deviation"] = heightDeviation;
    defect.attributes["tolerance"] = m_tolerance;

    defects.push_back(defect);
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
