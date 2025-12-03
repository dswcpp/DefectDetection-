#include "YoloDetector.h"
#include "../common/Logger.h"
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>

const QStringList YoloDetector::DEFAULT_CLASSES = {
    "scratch", "crack", "foreign", "stain", "dent", "hole"
};

YoloDetector::YoloDetector() {
  m_confidenceThreshold = 0.5;
  m_classNames = DEFAULT_CLASSES;
}

bool YoloDetector::initialize() {
  updateParameters();
  
  if (m_modelPath.isEmpty()) {
    LOG_WARN("YoloDetector: No model path specified");
    m_initialized = false;
    return false;
  }
  
  if (!QFile::exists(m_modelPath)) {
    LOG_ERROR("YoloDetector: Model file not found: {}", m_modelPath.toStdString());
    m_initialized = false;
    return false;
  }
  
  try {
    // 加载 ONNX 模型
    m_net = cv::dnn::readNetFromONNX(m_modelPath.toStdString());
    
    if (m_net.empty()) {
      LOG_ERROR("YoloDetector: Failed to load model");
      m_initialized = false;
      return false;
    }
    
    // 设置推理后端
    if (m_useGPU) {
#ifdef HAVE_CUDA
      m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
      m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
      LOG_INFO("YoloDetector: Using CUDA backend");
#else
      LOG_WARN("YoloDetector: CUDA not available, using CPU");
      m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
      m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
#endif
    } else {
      m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
      m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }
    
    m_modelLoaded = true;
    m_initialized = true;
    
    LOG_INFO("YoloDetector: Model loaded successfully from {}", m_modelPath.toStdString());
    LOG_INFO("YoloDetector: Input size: {}x{}, Classes: {}", 
             m_inputSize.width, m_inputSize.height, m_classNames.size());
    
    return true;
    
  } catch (const cv::Exception& e) {
    LOG_ERROR("YoloDetector: OpenCV exception: {}", e.what());
    m_initialized = false;
    return false;
  } catch (const std::exception& e) {
    LOG_ERROR("YoloDetector: Exception: {}", e.what());
    m_initialized = false;
    return false;
  }
}

void YoloDetector::release() {
  m_net = cv::dnn::Net();
  m_modelLoaded = false;
  m_initialized = false;
  LOG_INFO("YoloDetector: Released");
}

void YoloDetector::updateParameters() {
  m_modelPath = getParam<QString>("modelPath", m_modelPath);
  m_confidenceThreshold = getParam<double>("confidence", m_confidenceThreshold);
  m_nmsThreshold = getParam<float>("nmsThreshold", m_nmsThreshold);
  m_useGPU = getParam<bool>("useGPU", m_useGPU);
  
  int inputW = getParam<int>("inputWidth", m_inputSize.width);
  int inputH = getParam<int>("inputHeight", m_inputSize.height);
  m_inputSize = cv::Size(inputW, inputH);
}

void YoloDetector::setInputSize(int width, int height) {
  m_inputSize = cv::Size(width, height);
}

void YoloDetector::setUseGPU(bool use) {
  if (m_useGPU != use) {
    m_useGPU = use;
    if (m_modelLoaded) {
      // 重新配置后端
      initialize();
    }
  }
}

QString YoloDetector::modelInfo() const {
  if (!m_modelLoaded) {
    return "No model loaded";
  }
  
  QFileInfo fi(m_modelPath);
  return QString("Model: %1\nSize: %2x%3\nClasses: %4\nBackend: %5")
      .arg(fi.fileName())
      .arg(m_inputSize.width)
      .arg(m_inputSize.height)
      .arg(m_classNames.size())
      .arg(m_useGPU ? "CUDA" : "CPU");
}

DetectionResult YoloDetector::detect(const cv::Mat& image) {
  QElapsedTimer timer;
  timer.start();
  
  if (image.empty()) {
    return makeErrorResult("Empty input image");
  }
  
  if (!m_modelLoaded) {
    return makeErrorResult("Model not loaded");
  }
  
  try {
    // 预处理
    cv::Mat blob = preprocess(image);
    
    // 推理
    m_net.setInput(blob);
    cv::Mat output = m_net.forward();
    
    // 后处理
    std::vector<DefectInfo> defects = postprocess(output, image.size());
    
    // 过滤低置信度
    size_t beforeFilter = defects.size();
    defects = filterByConfidence(defects);
    
    double timeMs = timer.elapsed();
    
    if (!defects.empty()) {
      // 统计各类别
      std::map<int, int> classCounts;
      double minConf = 1.0, maxConf = 0.0;
      for (const auto& d : defects) {
        classCounts[d.classId]++;
        minConf = std::min(minConf, d.confidence);
        maxConf = std::max(maxConf, d.confidence);
      }
      LOG_INFO("YoloDetector::detect - {} defects (raw:{}, filtered:{}), conf:[{:.2f},{:.2f}], time:{:.1f}ms",
               defects.size(), beforeFilter, defects.size(), minConf, maxConf, timeMs);
    } else {
      LOG_DEBUG("YoloDetector::detect - No defects found, time:{:.1f}ms", timeMs);
    }
    
    return makeSuccessResult(defects, timeMs);
    
  } catch (const cv::Exception& e) {
    return makeErrorResult(QString("OpenCV error: %1").arg(e.what()));
  } catch (const std::exception& e) {
    return makeErrorResult(QString("Error: %1").arg(e.what()));
  }
}

cv::Mat YoloDetector::letterbox(const cv::Mat& image, cv::Size targetSize, cv::Scalar color) {
  int iw = image.cols;
  int ih = image.rows;
  int tw = targetSize.width;
  int th = targetSize.height;
  
  // 计算缩放比例，保持宽高比
  m_scaleFactor = std::min(static_cast<float>(tw) / iw, 
                           static_cast<float>(th) / ih);
  
  int newW = static_cast<int>(iw * m_scaleFactor);
  int newH = static_cast<int>(ih * m_scaleFactor);
  
  // 缩放图像
  cv::Mat resized;
  cv::resize(image, resized, cv::Size(newW, newH), 0, 0, cv::INTER_LINEAR);
  
  // 计算填充
  m_padW = (tw - newW) / 2;
  m_padH = (th - newH) / 2;
  
  // 创建目标图像并填充
  cv::Mat result(targetSize, image.type(), color);
  resized.copyTo(result(cv::Rect(m_padW, m_padH, newW, newH)));
  
  return result;
}

cv::Mat YoloDetector::preprocess(const cv::Mat& image) {
  // Letterbox resize
  cv::Mat letterboxed = letterbox(image, m_inputSize);
  
  // BGR to RGB
  cv::Mat rgb;
  cv::cvtColor(letterboxed, rgb, cv::COLOR_BGR2RGB);
  
  // 转换为 blob
  cv::Mat blob;
  cv::dnn::blobFromImage(rgb, blob, 1.0 / 255.0, m_inputSize, 
                         cv::Scalar(), true, false, CV_32F);
  
  return blob;
}

std::vector<DefectInfo> YoloDetector::postprocess(const cv::Mat& output, 
                                                   const cv::Size& originalSize) {
  std::vector<DefectInfo> defects;
  std::vector<cv::Rect> boxes;
  std::vector<float> confidences;
  std::vector<int> classIds;
  
  // YOLOv5/v8 输出格式: [batch, num_detections, 5+num_classes]
  // 其中 5 = [x_center, y_center, width, height, obj_conf]
  
  const int dimensions = output.size[2];
  const int rows = output.size[1];
  const int numClasses = dimensions - 5;
  
  // 输出数据指针
  float* data = (float*)output.data;
  
  for (int i = 0; i < rows; ++i) {
    float* row = data + i * dimensions;
    
    float objConf = row[4];
    if (objConf < m_confidenceThreshold) {
      continue;
    }
    
    // 找最大类别概率
    float maxClassConf = 0;
    int maxClassId = 0;
    for (int c = 0; c < numClasses; ++c) {
      float classConf = row[5 + c];
      if (classConf > maxClassConf) {
        maxClassConf = classConf;
        maxClassId = c;
      }
    }
    
    float confidence = objConf * maxClassConf;
    if (confidence < m_confidenceThreshold) {
      continue;
    }
    
    // 坐标转换 (从 letterbox 坐标转回原始坐标)
    float cx = row[0];
    float cy = row[1];
    float w = row[2];
    float h = row[3];
    
    // 去除 padding
    cx = (cx - m_padW) / m_scaleFactor;
    cy = (cy - m_padH) / m_scaleFactor;
    w = w / m_scaleFactor;
    h = h / m_scaleFactor;
    
    // 转换为左上角坐标
    int x = static_cast<int>(cx - w / 2);
    int y = static_cast<int>(cy - h / 2);
    int width = static_cast<int>(w);
    int height = static_cast<int>(h);
    
    // 边界检查
    x = std::max(0, std::min(x, originalSize.width - 1));
    y = std::max(0, std::min(y, originalSize.height - 1));
    width = std::min(width, originalSize.width - x);
    height = std::min(height, originalSize.height - y);
    
    boxes.push_back(cv::Rect(x, y, width, height));
    confidences.push_back(confidence);
    classIds.push_back(maxClassId);
  }
  
  // NMS
  std::vector<int> indices;
  cv::dnn::NMSBoxes(boxes, confidences, static_cast<float>(m_confidenceThreshold), m_nmsThreshold, indices);
  
  // 构建结果
  for (int idx : indices) {
    DefectInfo defect;
    defect.bbox = boxes[idx];
    defect.confidence = confidences[idx];
    defect.classId = classIds[idx];
    
    if (classIds[idx] < m_classNames.size()) {
      defect.className = m_classNames[classIds[idx]];
    } else {
      defect.className = QString("class_%1").arg(classIds[idx]);
    }
    
    // 严重度基于置信度
    defect.severity = confidences[idx];
    
    defects.push_back(defect);
  }
  
  return defects;
}
