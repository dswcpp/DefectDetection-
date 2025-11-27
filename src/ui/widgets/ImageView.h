#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QGraphicsView>
#include <QImage>
#include <QVector>
#include "ui_global.h"
#include "opencv2/opencv.hpp"

class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsRectItem;

class ImageView : public QGraphicsView {
  Q_OBJECT
public:
  explicit ImageView(QWidget* parent = nullptr);

  // 图像操作
  void setImage(const cv::Mat& image);
  void setImage(const QImage& image);
  void clear();

  // 缺陷标注
  void drawDefectRegions(const std::vector<cv::Rect>& regions,
                         const QColor& color = Qt::red);
  void clearAnnotations();

  // ROI 编辑
  void setROI(const cv::Rect& roi);
  cv::Rect getROI() const;
  void enableROIEdit(bool enable);

  // 显示模式
  enum class DisplayMode { Original, Annotated, SideBySide };
  void setDisplayMode(DisplayMode mode);

  // 缩放
  void zoomIn();
  void zoomOut();
  void zoomFit();
  void zoomActual();

signals:
  void roiChanged(const QRect& roi);
  void mousePositionChanged(const QPoint& pos, int grayValue);
  void zoomChanged(double factor);

protected:
  void wheelEvent(QWheelEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

private:
  QGraphicsScene* m_scene = nullptr;
  QGraphicsPixmapItem* m_imageItem = nullptr;
  QGraphicsRectItem* m_roiItem = nullptr;
  QVector<QGraphicsRectItem*> m_defectItems;

  bool m_roiEditEnabled = false;
  bool m_isDragging = false;
  QPointF m_dragStart;
  double m_zoomFactor = 1.0;
  DisplayMode m_displayMode = DisplayMode::Original;
  QImage m_currentImage;

  QImage cvMatToQImage(const cv::Mat& mat);
};

#endif // IMAGEVIEW_H
