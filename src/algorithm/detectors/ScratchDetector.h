#ifndef SCRATCHDETECTOR_H
#define SCRATCHDETECTOR_H

#include "../BaseDetector.h"

// 划痕检测器：使用边缘检测+形态学方法
class ALGORITHM_LIBRARY ScratchDetector : public BaseDetector {
public:
  ScratchDetector();
  ~ScratchDetector() override = default;

  // IDefectDetector 接口
  QString name() const override { return QObject::tr("划痕检测"); }
  QString type() const override { return "scratch"; }
  
  bool initialize() override;
  void release() override;
  DetectionResult detect(const cv::Mat& image) override;

private:
  // 参数
  int m_sensitivity = 75;       // 灵敏度 [0-100]
  int m_minLength = 10;         // 最小长度（像素）
  int m_maxWidth = 5;           // 最大宽度（像素）
  int m_contrastThreshold = 30; // 对比度阈值

  // 内部方法
  void updateParameters();
  cv::Mat preprocessImage(const cv::Mat& input);
  std::vector<DefectInfo> findScratches(const cv::Mat& edges, const cv::Mat& original);
  std::vector<DefectInfo> detectLinesHough(const cv::Mat& edges, const cv::Mat& original);
  std::vector<DefectInfo> nmsDefects(const std::vector<DefectInfo>& defects, double iouThreshold);
  bool isValidScratch(const std::vector<cv::Point>& contour);
  double calculateSeverity(double length, double avgWidth);
};

#endif // SCRATCHDETECTOR_H
