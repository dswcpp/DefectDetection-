/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ImagePreviewDialog.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：图片预览对话框接口定义
 * 描述：增强版图片预览对话框，集成ImageView和AnnotationPanel，
 *       支持缺陷标注显示、ROI编辑、手动标注添加/编辑/删除等功能
 *
 * 当前版本：1.0
 */

#ifndef IMAGEPREVIEWDIALOG_H
#define IMAGEPREVIEWDIALOG_H

#include <QDialog>
#include <QImage>
#include <vector>
#include "ui_global.h"
#include "opencv2/opencv.hpp"

class ImageView;
class AnnotationPanel;
class QToolBar;
class QAction;
class QLabel;
class QSlider;
class QComboBox;
class QPushButton;
class DatabaseManager;
struct DefectAnnotation;

// 增强版图片预览对话框
// 支持缺陷标注显示、ROI编辑、手动标注添加/编辑/删除
class UI_LIBRARY ImagePreviewDialog : public QDialog {
  Q_OBJECT
public:
  explicit ImagePreviewDialog(QWidget *parent = nullptr);
  ~ImagePreviewDialog();

  // 设置图片
  void setImage(const QImage& image);
  void setImage(const QString& imagePath);
  void setImage(const cv::Mat& image);

  // 设置缺陷区域（自动检测结果）
  void setDefectRegions(const std::vector<cv::Rect>& regions, 
                        const QColor& color = Qt::red);
  
  // 设置检测结果（带标签和置信度）
  void setDetectionResults(const std::vector<cv::Rect>& boxes,
                           const QStringList& labels,
                           const std::vector<double>& confidences);

  // ROI设置
  void setROI(const cv::Rect& roi);
  cv::Rect getROI() const;
  void enableROIEdit(bool enable);

  // 获取标注
  QVector<DefectAnnotation> getAnnotations() const;
  
  // 设置是否显示工具栏
  void setToolbarVisible(bool visible);
  void setAnnotationPanelVisible(bool visible);

  // 设置数据库管理器和检测记录ID（用于保存标注到数据库）
  void setDatabaseManager(DatabaseManager* dbManager);
  void setInspectionId(qint64 inspectionId);

signals:
  void roiChanged(const cv::Rect& roi);
  void annotationsChanged();
  void imageSaved(const QString& path);
  void annotationsSavedToDatabase();

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

private slots:
  void onZoomIn();
  void onZoomOut();
  void onZoomFit();
  void onZoomActual();
  void onZoomSliderChanged(int value);
  void onToggleAnnotationMode();
  void onToggleROIMode();
  void onSaveImage();
  void onCopyImage();
  void onZoomChanged(double factor);
  void onSaveToDatabase();

private:
  void setupUI();
  void createToolBar();
  void createStatusBar();
  void updateZoomLabel();
  void updateStatusInfo(const QPoint& pos, int grayValue);

  // 主要组件
  ImageView* m_imageView = nullptr;
  AnnotationPanel* m_annotationPanel = nullptr;
  
  // 工具栏
  QToolBar* m_toolbar = nullptr;
  QAction* m_zoomInAction = nullptr;
  QAction* m_zoomOutAction = nullptr;
  QAction* m_zoomFitAction = nullptr;
  QAction* m_zoomActualAction = nullptr;
  QAction* m_annotateAction = nullptr;
  QAction* m_roiAction = nullptr;
  QAction* m_saveAction = nullptr;
  QAction* m_copyAction = nullptr;
  
  // 缩放控件
  QSlider* m_zoomSlider = nullptr;
  QLabel* m_zoomLabel = nullptr;
  
  // 状态栏
  QLabel* m_positionLabel = nullptr;
  QLabel* m_grayValueLabel = nullptr;
  QLabel* m_imageSizeLabel = nullptr;
  
  // 状态
  double m_currentZoom = 1.0;
  bool m_firstShow = true;
  QString m_currentImagePath;

  // 数据库相关
  DatabaseManager* m_dbManager = nullptr;
  qint64 m_inspectionId = 0;
  QAction* m_saveToDbAction = nullptr;
};

#endif // IMAGEPREVIEWDIALOG_H
