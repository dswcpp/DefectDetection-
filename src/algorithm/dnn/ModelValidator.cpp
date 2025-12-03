// ModelValidator.cpp
#include "ModelValidator.h"
#include "../common/Logger.h"
#include <QFileInfo>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <chrono>
#include <algorithm>

ModelValidationReport ModelValidator::validateONNX(const QString& onnxPath,
                                                  const Expectation& exp,
                                                  bool tryCUDA) {
  ModelValidationReport rep;
  
  LOG_INFO("ModelValidator: Validating ONNX model: {}, inputSize={}x{}, cuda={}", 
           onnxPath.toStdString(), exp.inputSize.width, exp.inputSize.height, tryCUDA);

  // 0) 文件检查
  QFileInfo fi(onnxPath);
  if (!fi.exists() || !fi.isFile()) {
    LOG_ERROR("ModelValidator: Model file not found: {}", onnxPath.toStdString());
    rep.errors << QString("模型文件不存在：%1").arg(onnxPath);
    return rep;
  }

  if (fi.size() < 10 * 1024) {
    rep.errors << "模型文件过小，疑似损坏";
    return rep;
  }

  // 1) 读取模型
  cv::dnn::Net net;
  try {
    net = cv::dnn::readNetFromONNX(onnxPath.toStdString());
  } catch (const cv::Exception& e) {
    rep.errors << QString("ONNX 解析失败：%1").arg(e.what());
    return rep;
  }

  // 2) Backend/Target
  if (tryCUDA) {
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    rep.backend = "CUDA";
    rep.target = "CUDA";
  } else {
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    rep.backend = "OPENCV";
    rep.target = "CPU";
  }

  // 3) 输出层检查
  std::vector<cv::String> outNames = net.getUnconnectedOutLayersNames();
  if (outNames.empty()) {
    rep.errors << "未检测到输出层（UnconnectedOutLayersNames 为空）";
    return rep;
  }

  // 4) 构造虚拟输入并前向
  const int W = exp.inputSize.width;
  const int H = exp.inputSize.height;
  if (W <= 0 || H <= 0) {
    rep.errors << "期望输入尺寸非法";
    return rep;
  }

  cv::Mat dummy(H, W, CV_8UC3, cv::Scalar(127, 127, 127));
  cv::Mat blob;
  try {
    cv::dnn::blobFromImage(dummy, blob, 1.0/255.0, cv::Size(W, H),
                           cv::Scalar(), true /*swapRB*/, false /*crop*/);
  } catch (const cv::Exception& e) {
    rep.errors << QString("构造 blob 失败：%1").arg(e.what());
    return rep;
  }

  if (exp.requireNCHW) {
    if (blob.dims != 4 || blob.size[1] != exp.inputChannels) {
      rep.errors << QString("输入张量不匹配，需 NCHW 且 C=%1，实际 C=%2")
          .arg(exp.inputChannels)
          .arg(blob.dims >= 2 ? blob.size[1] : -1);
      return rep;
    }
  }

  try {
    net.setInput(blob);

    // warmup
    {
      auto t0 = std::chrono::steady_clock::now();
      net.forward(outNames.front());
      auto t1 = std::chrono::steady_clock::now();
      rep.warmupMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }

    // 单次推理
    cv::Mat out;
    {
      auto t0 = std::chrono::steady_clock::now();
      out = net.forward(outNames.front());
      auto t1 = std::chrono::steady_clock::now();
      rep.singleRunMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }

    // 输出维度检查
    if (out.dims < 2) {
      rep.errors << QString("输出张量维度过低：%1").arg(out.dims);
      return rep;
    }

    if (out.dims < exp.minOutputDims) {
      rep.warnings << QString("输出维度为 %1，低于期望下限 %2")
          .arg(out.dims).arg(exp.minOutputDims);
    }

    // 解析潜在类别数（YOLO 典型：最后维 = 5+C）
    int attrs = out.size[out.dims - 1];
    if (exp.expectedClasses > 0) {
      int maybeC = attrs - 5;
      if (maybeC <= 0) {
        rep.errors << "无法从输出推断类别数（attrs-5 <= 0）";
        return rep;
      }

      rep.numClasses = maybeC;
      if (rep.numClasses != exp.expectedClasses) {
        rep.errors << QString("类别数不匹配：期望 %1，实际 %2")
            .arg(exp.expectedClasses).arg(rep.numClasses);
        return rep;
      }
    }

  } catch (const cv::Exception& e) {
    rep.errors << QString("前向推理失败：%1").arg(e.what());
    return rep;
  }

  // 5) 性能阈值
  if (rep.warmupMs > exp.maxWarmupMs) {
    rep.warnings << QString("warmup 耗时偏高：%1 ms > %2 ms")
        .arg(rep.warmupMs, 0, 'f', 2).arg(exp.maxWarmupMs, 0, 'f', 2);
  }

  if (rep.singleRunMs > exp.maxSingleRunMs) {
    rep.warnings << QString("单次推理耗时偏高：%1 ms > %2 ms")
        .arg(rep.singleRunMs, 0, 'f', 2).arg(exp.maxSingleRunMs, 0, 'f', 2);
  }

  rep.ok = rep.errors.isEmpty();
  
  if (rep.ok) {
    LOG_INFO("ModelValidator: Validation passed - backend={}, warmup={:.1f}ms, inference={:.1f}ms, classes={}",
             rep.backend.toStdString(), rep.warmupMs, rep.singleRunMs, rep.numClasses);
  } else {
    LOG_ERROR("ModelValidator: Validation failed - {} errors, {} warnings", 
              rep.errors.size(), rep.warnings.size());
  }
  
  return rep;
}
