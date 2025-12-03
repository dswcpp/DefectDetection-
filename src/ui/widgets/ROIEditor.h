/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ROIEditor.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：ROI编辑器控件接口定义
 * 描述：感兴趣区域编辑器，支持矩形/多边形ROI绘制、调整、保存
 *
 * 当前版本：1.0
 */

#ifndef ROIEDITOR_H
#define ROIEDITOR_H

#include <QWidget>

class ROIEditor : public QWidget {
  Q_OBJECT
public:
  explicit ROIEditor(QWidget *parent = nullptr);

signals:
};

#endif // ROIEDITOR_H
