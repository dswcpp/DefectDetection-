#ifndef YOLODETECTOR_H
#define YOLODETECTOR_H

#include "../BaseDetector.h"
#include <opencv2/dnn.hpp>
#include <QStringList>

// YOLO 深度学习检测器
// 支持 YOLOv5/v8 ONNX 模型
class ALGORITHM_LIBRARY YoloDetector : public BaseDetector {
public:
  YoloDetector();
  ~YoloDetector() override = default;

  // IDefectDetector 接口
  QString name() const override { return QObject::tr("YOLO检测器"); }
  QString type() const override { return "yolo"; }
  
  bool initialize() override;
  void release() override;
  DetectionResult detect(const cv::Mat& image) override;

  // 模型配置
  void setModelPath(const QString& path) { m_modelPath = path; }
  QString modelPath() const { return m_modelPath; }
  
  void setClassNames(const QStringList& names) { m_classNames = names; }
  QStringList classNames() const { return m_classNames; }
  
  void setInputSize(int width, int height);
  cv::Size inputSize() const { return m_inputSize; }
  
  // 推理配置 (confidenceThreshold 继承自 BaseDetector)
  void setNmsThreshold(float thresh) { m_nmsThreshold = thresh; }
  float nmsThreshold() const { return m_nmsThreshold; }
  
  void setUseGPU(bool use);
  bool isUsingGPU() const { return m_useGPU; }
  
  // 模型信息
  bool isModelLoaded() const { return m_modelLoaded; }
  QString modelInfo() const;

private:
  void updateParameters();
  cv::Mat preprocess(const cv::Mat& image);
  std::vector<DefectInfo> postprocess(const cv::Mat& output, 
                                       const cv::Size& originalSize);
  cv::Mat letterbox(const cv::Mat& image, cv::Size targetSize, 
                    cv::Scalar color = cv::Scalar(114, 114, 114));

  // 模型
  cv::dnn::Net m_net;
  QString m_modelPath;
  bool m_modelLoaded = false;
  bool m_useGPU = false;
  
  // 输入参数
  cv::Size m_inputSize{640, 640};
  float m_scaleFactor = 1.0f;
  int m_padW = 0;
  int m_padH = 0;
  
  // 推理参数 (m_confidenceThreshold 继承自 BaseDetector)
  float m_nmsThreshold = 0.45f;
  
  // 类别名称
  QStringList m_classNames;
  
  // 默认缺陷类别
  static const QStringList DEFAULT_CLASSES;
};

#endif // YOLODETECTOR_H
