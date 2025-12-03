/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ResultAggregator.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：结果聚合器接口定义
 * 描述：多工位/多相机检测结果聚合器，合并各路检测结果，
 *       生成综合判定
 *
 * 当前版本：1.0
 */

#ifndef RESULTAGGREGATOR_H
#define RESULTAGGREGATOR_H

#include "Types.h"
#include <QObject>
#include <vector>

class ResultAggregator : public QObject {
  Q_OBJECT
public:
  explicit ResultAggregator(QObject *parent = nullptr);

  // 聚合多工位结果
  DetectResult aggregate(const std::vector<DetectResult> &results);

  // 清空内部缓存
  void reset();

signals:
  void aggregated(const DetectResult &result);
};

#endif // RESULTAGGREGATOR_H
