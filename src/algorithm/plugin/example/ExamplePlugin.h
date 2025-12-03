/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ExamplePlugin.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：示例插件实现
 * 描述：检测器插件示例代码，演示如何实现IDetectorPlugin接口，
 *       可作为开发新插件的模板
 *
 * 当前版本：1.0
 */

#ifndef EXAMPLEPLUGIN_H
#define EXAMPLEPLUGIN_H

#include "../IDetectorPlugin.h"
#include "../../BaseDetector.h"
#include <QObject>

// 示例检测器：简单的亮度异常检测
class ExampleDetector : public BaseDetector {
public:
  ExampleDetector() {
    m_confidenceThreshold = 0.5;
  }
  
  QString name() const override { return QObject::tr("示例检测器"); }
  QString type() const override { return "example"; }
  
  bool initialize() override {
    m_threshold = getParam<int>("threshold", 200);
    m_initialized = true;
    return true;
  }
  
  void release() override {
    m_initialized = false;
  }
  
  DetectionResult detect(const cv::Mat& image) override {
    if (image.empty()) {
      return makeErrorResult("Empty image");
    }
    
    QElapsedTimer timer;
    timer.start();
    
    cv::Mat gray;
    if (image.channels() == 3) {
      cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
      gray = image;
    }
    
    // 简单阈值检测
    cv::Mat binary;
    cv::threshold(gray, binary, m_threshold, 255, cv::THRESH_BINARY);
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    std::vector<DefectInfo> defects;
    for (const auto& contour : contours) {
      double area = cv::contourArea(contour);
      if (area < 100) continue;
      
      DefectInfo defect;
      defect.bbox = cv::boundingRect(contour);
      defect.contour = contour;
      defect.classId = 0;
      defect.className = "bright_spot";
      defect.confidence = std::min(1.0, area / 1000.0);
      defect.severity = defect.confidence;
      defects.push_back(defect);
    }
    
    return makeSuccessResult(defects, timer.elapsed());
  }

private:
  int m_threshold = 200;
};

// 示例插件
class ExamplePlugin : public QObject, public IDetectorPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID IDetectorPlugin_iid FILE "example_plugin.json")
  Q_INTERFACES(IDetectorPlugin)
  
public:
  QString pluginName() const override { return "Example Detector Plugin"; }
  QString pluginDescription() const override { 
    return "A simple example detector plugin for demonstration"; 
  }
  QString pluginAuthor() const override { return "DefectDetection Team"; }
  QVersionNumber pluginVersion() const override { return QVersionNumber(1, 0, 0); }
  
  QString detectorType() const override { return "example"; }
  QString detectorName() const override { return "Example Detector"; }
  
  IDefectDetector* createDetector() override {
    return new ExampleDetector();
  }
};

#endif // EXAMPLEPLUGIN_H
