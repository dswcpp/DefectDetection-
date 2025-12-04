/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ImageViewControls.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：图像视图控制控件接口定义
 * 描述：图像视图的控制面板，包含缩放按钮、适应窗口、1:1显示等快捷操作
 *
 * 当前版本：1.0
 */

#ifndef IMAGEVIEWCONTROLS_H
#define IMAGEVIEWCONTROLS_H

#include <QWidget>

class QButtonGroup;
class QRadioButton;
class QCheckBox;
class QPushButton;
class QLabel;

class ImageViewControls : public QWidget {
  Q_OBJECT
public:
  explicit ImageViewControls(QWidget* parent = nullptr);

signals:
  // 显示模式改变信号
  void displayModeChanged(int mode);  // 0=原图, 1=标注图

  // ROI显示状态改变
  void showROIChanged(bool show);

  // 缩放控制
  void zoomInRequested();
  void zoomOutRequested();
  void zoomFitRequested();
  void zoomActualRequested();

public slots:
  void setZoomLevel(double zoom);
  void setDisplayMode(int mode);  // 0=原图, 1=标注图
  void setShowROI(bool show);

private:
  void setupUI();

  QButtonGroup* m_displayModeGroup;
  QRadioButton* m_originalButton;
  QRadioButton* m_annotatedButton;
  QCheckBox* m_showROICheck;
  QPushButton* m_zoomInButton;
  QPushButton* m_zoomOutButton;
  QPushButton* m_zoomFitButton;
  QPushButton* m_zoom100Button;
  QLabel* m_zoomLabel;
};

#endif // IMAGEVIEWCONTROLS_H