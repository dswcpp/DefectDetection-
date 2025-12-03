/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * StatisticsView.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：统计视图接口定义
 * 描述：统计分析界面，显示良率趋势图、缺陷类型分布图、
 *       时间段统计等图表
 *
 * 当前版本：1.0
 */

#ifndef STATISTICSVIEW_H
#define STATISTICSVIEW_H

#include <QWidget>

class StatisticsView : public QWidget {
  Q_OBJECT
public:
  explicit StatisticsView(QWidget *parent = nullptr);

signals:
};

#endif // STATISTICSVIEW_H
