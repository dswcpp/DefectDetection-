#ifndef FOREIGNDETECTOR_H
#define FOREIGNDETECTOR_H

#include "../BaseDetector.h"

// 异物检测器：使用颜色/对比度分析
class ALGORITHM_LIBRARY ForeignDetector : public BaseDetector {
public:
  ForeignDetector();
  ~ForeignDetector() override = default;

  // IDefectDetector 接口
  QString name() const override { return QObject::tr("异物检测"); }
  QString type() const override { return "foreign"; }
  
  bool initialize() override;
  void release() override;
  DetectionResult detect(const cv::Mat& image) override;

private:
  // 参数
  int m_minArea = 5;             // 最小面积（像素²）
  double m_contrast = 0.3;       // 对比度阈值 [0-1]
  int m_colorThreshold = 50;     // 颜色差异阈值

  // 内部方法
  void updateParameters();
  cv::Mat preprocessImage(const cv::Mat& input);
  std::vector<DefectInfo> findForeignObjects(const cv::Mat& diff, const cv::Mat& original);
  std::vector<DefectInfo> detectColorAnomalies(const cv::Mat& image);
  std::vector<DefectInfo> nmsDefects(const std::vector<DefectInfo>& defects, double iouThreshold);
  double calculateSeverity(double area, double contrast);
};

#endif // FOREIGNDETECTOR_H
