/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * DetectView.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：检测视图接口定义
 * 描述：检测主界面视图，整合图像显示、结果卡片、参数面板等组件
 *
 * 当前版本：1.0
 */

#ifndef DETECTVIEW_H
#define DETECTVIEW_H

#include <QWidget>

class DetectView : public QWidget {
  Q_OBJECT
public:
  explicit DetectView(QWidget *parent = nullptr);

signals:
};

#endif // DETECTVIEW_H
