/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ImageView.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：图像视图控件接口定义
 * 描述：基于QGraphicsView的图像显示控件，支持缩放、平移、
 *       缺陷标注显示、ROI编辑、手动标注绘制等功能
 *
 * 当前版本：1.0
 */

#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QGraphicsView>
#include <QImage>
#include <QVector>
#include <QMenu>
#include <QDateTime>
#include "ui_global.h"
#include "opencv2/opencv.hpp"

class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsRectItem;
class QGraphicsTextItem;
class QGraphicsEllipseItem;
class QGraphicsPolygonItem;

// 缺陷严重等级
enum class DefectSeverity {
  Critical = 0,  // 严重
  Major = 1,     // 重大
  Minor = 2,     // 轻微
  Info = 3       // 信息
};

// 缺陷类型
enum class DefectType {
  Scratch,       // 划痕
  Crack,         // 裂纹
  Bubble,        // 气泡
  ForeignObject, // 异物
  Deformation,   // 变形
  ColorDefect,   // 色差
  Other          // 其他
};

// 标注形状类型
enum class AnnotationShape {
  Rectangle,     // 矩形
  Circle,        // 圆形
  Polygon        // 多边形
};

// 缺陷标注信息结构体
struct DefectAnnotation {
  int id;                        // 缺陷ID
  AnnotationShape shape;         // 标注形状
  cv::Rect boundingRect;         // 边界框
  std::vector<cv::Point> points; // 多边形点（如果是多边形）
  DefectType type;               // 缺陷类型
  DefectSeverity severity;       // 严重等级
  QString description;           // 描述
  double confidence;             // 置信度（如果是自动检测）
  bool isManual;                 // 是否手动标注
  QColor color;                  // 显示颜色
  QDateTime timestamp;           // 标注时间
};

// 检测框信息结构体（保留用于兼容）
struct DetectionBox {
  cv::Rect rect;
  QString label;
  QColor color;
  double confidence;
};

class ImageView : public QGraphicsView {
  Q_OBJECT
public:
  explicit ImageView(QWidget* parent = nullptr);
  ~ImageView();

  // 图像操作
  void setImage(const cv::Mat& image);
  void setImage(const QImage& image);
  void clear();

  // 缺陷标注 - 增强版
  void addDefectAnnotation(const DefectAnnotation& annotation);
  void updateDefectAnnotation(int id, const DefectAnnotation& annotation);
  void removeDefectAnnotation(int id);
  void clearAnnotations();
  QVector<DefectAnnotation> getDefectAnnotations() const;
  DefectAnnotation* getDefectAnnotationAt(const QPoint& pos);

  // 手动标注模式
  void setAnnotationMode(bool enable);
  bool isAnnotationMode() const { return m_annotationMode; }
  void setCurrentAnnotationShape(AnnotationShape shape);
  void setCurrentDefectType(DefectType type);
  void setCurrentDefectSeverity(DefectSeverity severity);

  // 检测框绘制（保留兼容）
  void drawDetectionBoxes(const QVector<DetectionBox>& boxes);
  void addDetectionBox(const DetectionBox& box);
  void clearDetectionBoxes();

  // 旧接口兼容
  void drawDefectRegions(const std::vector<cv::Rect>& regions,
                         const QColor& color = Qt::red);

  // ROI 编辑
  void setROI(const cv::Rect& roi);
  cv::Rect getROI() const;
  void enableROIEdit(bool enable);

  // 显示模式
  enum class DisplayMode { Original, Annotated, SideBySide };
  void setDisplayMode(DisplayMode mode);
  DisplayMode displayMode() const { return m_displayMode; }

  // ROI显示控制
  void setShowROI(bool show);
  bool isShowROI() const { return m_showROI; }

  // 显示控制
  void setShowSeverityLabels(bool show) { m_showSeverityLabels = show; updateDisplay(); }
  void setShowConfidence(bool show) { m_showConfidence = show; updateDisplay(); }
  void setHighlightSeverity(DefectSeverity severity, bool highlight);
  void setShowAnnotations(bool show);

  // 缩放
  void zoomIn();
  void zoomOut();
  void zoomFit();
  void zoomActual();

  // 导出功能
  QImage exportAnnotatedImage() const;
  bool saveAnnotations(const QString& filePath) const;
  bool loadAnnotations(const QString& filePath);

signals:
  void roiChanged(const QRect& roi);
  void mousePositionChanged(const QPoint& pos, int grayValue);
  void zoomChanged(double factor);

  // 新信号
  void defectAnnotationAdded(const DefectAnnotation& annotation);
  void defectAnnotationUpdated(int id, const DefectAnnotation& annotation);
  void defectAnnotationRemoved(int id);
  void defectAnnotationSelected(int id);
  void annotationModeChanged(bool enabled);

protected:
  void wheelEvent(QWheelEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

private:
  QGraphicsScene* m_scene = nullptr;
  QGraphicsPixmapItem* m_imageItem = nullptr;
  QGraphicsRectItem* m_roiItem = nullptr;

  // 缺陷标注相关
  QVector<DefectAnnotation> m_defectAnnotations;
  QMap<int, QGraphicsItem*> m_annotationGraphicsItems;
  QMap<int, QGraphicsTextItem*> m_annotationLabels;
  int m_nextAnnotationId = 1;

  // 手动标注状态
  bool m_annotationMode = false;
  AnnotationShape m_currentShape = AnnotationShape::Rectangle;
  DefectType m_currentDefectType = DefectType::Other;
  DefectSeverity m_currentSeverity = DefectSeverity::Minor;
  bool m_isDrawing = false;
  QPointF m_drawingStart;
  QPointF m_drawingEnd;
  QVector<QPointF> m_polygonPoints;
  QGraphicsItem* m_tempDrawingItem = nullptr;

  // 选中的标注
  int m_selectedAnnotationId = -1;
  bool m_isMovingAnnotation = false;
  QPointF m_moveOffset;

  // 旧的成员变量（保留兼容）
  QVector<QGraphicsRectItem*> m_defectItems;
  QVector<QGraphicsTextItem*> m_labelItems;
  QVector<DetectionBox> m_detectionBoxes;

  // 其他状态
  bool m_roiEditEnabled = false;
  bool m_isDragging = false;
  QPointF m_dragStart;
  double m_zoomFactor = 1.0;
  DisplayMode m_displayMode = DisplayMode::Original;
  QImage m_currentImage;

  // 显示选项
  bool m_showSeverityLabels = true;
  bool m_showConfidence = true;
  bool m_showROI = false;
  bool m_showAnnotations = true;
  QSet<DefectSeverity> m_highlightedSeverities;

  // 右键菜单
  QMenu* m_contextMenu = nullptr;

  // 私有方法
  QImage cvMatToQImage(const cv::Mat& mat);
  void drawSingleDetectionBox(const DetectionBox& box);
  void updateDisplay();
  void drawAnnotation(const DefectAnnotation& annotation);
  void removeAnnotationGraphics(int id);
  void selectAnnotation(int id);
  void createContextMenu();
  void updateAnnotationGraphics(int id);
  void finishPolygonDrawing();
  void cancelDrawing();

  // 辅助方法
  QColor severityToColor(DefectSeverity severity) const;
  QString severityToString(DefectSeverity severity) const;
  QString defectTypeToString(DefectType type) const;
  DefectAnnotation createAnnotationFromRect(const QRectF& rect);
  bool isPointInAnnotation(const QPointF& point, const DefectAnnotation& annotation) const;

private slots:
  void onEditAnnotation();
  void onDeleteAnnotation();
  void onChangeAnnotationSeverity();
  void onChangeAnnotationType();
};

#endif // IMAGEVIEW_H
