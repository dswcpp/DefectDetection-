#include "ImageView.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QScrollBar>
#include <QWheelEvent>
#include <QFont>
#include <algorithm>

ImageView::ImageView(QWidget* parent) : QGraphicsView{parent}
{
  m_scene = new QGraphicsScene(this);
  setScene(m_scene);

  m_imageItem = new QGraphicsPixmapItem();
  m_scene->addItem(m_imageItem);

  m_roiItem = new QGraphicsRectItem();
  m_roiItem->setPen(QPen(QColor("#ff9800"), 2, Qt::DashLine));
  m_roiItem->setVisible(false);
  m_scene->addItem(m_roiItem);

  // 设置深色背景以匹配原型设计
  setBackgroundBrush(QColor("#0f172a"));
  setRenderHint(QPainter::Antialiasing, true);
  setDragMode(QGraphicsView::ScrollHandDrag);
  setMouseTracking(true);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  setResizeAnchor(QGraphicsView::AnchorViewCenter);

  // 设置样式
  setObjectName(QStringLiteral("ImageView"));
}

void ImageView::setImage(const cv::Mat& image)
{
  if (image.empty()) {
    clear();
    return;
  }
  const QImage qimg = cvMatToQImage(image);
  setImage(qimg);
}

void ImageView::setImage(const QImage& image)
{
  m_currentImage = image;
  m_imageItem->setPixmap(QPixmap::fromImage(image));
  m_scene->setSceneRect(QRectF(QPointF(0, 0), QSizeF(image.size())));
  m_roiItem->setVisible(false);
  zoomFit();
}

void ImageView::clear()
{
  m_currentImage = QImage();
  m_imageItem->setPixmap(QPixmap());
  clearAnnotations();
  m_roiItem->setVisible(false);
}

void ImageView::drawDefectRegions(const std::vector<cv::Rect>& regions, const QColor& color)
{
  clearAnnotations();
  for (const auto& rect : regions) {
    auto* item = new QGraphicsRectItem(QRectF(rect.x, rect.y, rect.width, rect.height));
    item->setPen(QPen(color, 2));
    item->setBrush(QColor(color.red(), color.green(), color.blue(), 40));
    m_scene->addItem(item);
    m_defectItems.push_back(item);
  }
}

void ImageView::clearAnnotations()
{
  for (auto* item : m_defectItems) {
    m_scene->removeItem(item);
    delete item;
  }
  m_defectItems.clear();

  // 同时清除标签
  for (auto* label : m_labelItems) {
    m_scene->removeItem(label);
    delete label;
  }
  m_labelItems.clear();
}

void ImageView::drawDetectionBoxes(const QVector<DetectionBox>& boxes)
{
  clearDetectionBoxes();
  m_detectionBoxes = boxes;

  for (const auto& box : boxes) {
    drawSingleDetectionBox(box);
  }
}

void ImageView::addDetectionBox(const DetectionBox& box)
{
  m_detectionBoxes.append(box);
  drawSingleDetectionBox(box);
}

void ImageView::clearDetectionBoxes()
{
  clearAnnotations();
  m_detectionBoxes.clear();
}

void ImageView::drawSingleDetectionBox(const DetectionBox& box)
{
  // 创建检测框
  auto* rectItem = new QGraphicsRectItem(QRectF(box.rect.x, box.rect.y,
                                                box.rect.width, box.rect.height));

  // 设置框的样式 - 根据检测结果使用不同颜色
  QPen pen(box.color, 2.5, Qt::SolidLine);
  pen.setCapStyle(Qt::RoundCap);
  rectItem->setPen(pen);

  // 半透明填充
  QColor fillColor = box.color;
  fillColor.setAlpha(30);
  rectItem->setBrush(fillColor);

  m_scene->addItem(rectItem);
  m_defectItems.push_back(rectItem);

  // 创建标签（如BOX1, BOX2等）
  if (!box.label.isEmpty()) {
    auto* textItem = new QGraphicsTextItem();

    // 设置标签文本
    QString labelText = box.label;
    if (box.confidence > 0) {
      labelText += QString(" %1%").arg(static_cast<int>(box.confidence * 100));
    }
    textItem->setPlainText(labelText);

    // 设置标签样式
    QFont font("Arial", 10, QFont::Bold);
    textItem->setFont(font);
    textItem->setDefaultTextColor(Qt::white);

    // 创建标签背景
    QRectF textRect = textItem->boundingRect();
    auto* bgRect = new QGraphicsRectItem(textRect);
    bgRect->setBrush(box.color);
    bgRect->setPen(Qt::NoPen);

    // 设置位置 - 标签位于框的左上角
    bgRect->setPos(box.rect.x, box.rect.y - textRect.height() - 2);
    textItem->setPos(box.rect.x + 2, box.rect.y - textRect.height() - 2);

    m_scene->addItem(bgRect);
    m_scene->addItem(textItem);
    m_defectItems.push_back(bgRect);
    m_labelItems.push_back(textItem);
  }
}

void ImageView::setROI(const cv::Rect& roi)
{
  if (roi.width <= 0 || roi.height <= 0) {
    m_roiItem->setVisible(false);
    return;
  }

  QRectF rect(roi.x, roi.y, roi.width, roi.height);
  rect = rect.intersected(m_scene->sceneRect());
  m_roiItem->setRect(rect);
  m_roiItem->setVisible(true);
  emit roiChanged(rect.toRect());
}

cv::Rect ImageView::getROI() const
{
  if (!m_roiItem->isVisible())
    return cv::Rect();

  const QRectF rect = m_roiItem->rect();
  return cv::Rect(rect.x(), rect.y(), rect.width(), rect.height());
}

void ImageView::enableROIEdit(bool enable)
{
  m_roiEditEnabled = enable;
  if (!enable) {
    m_roiItem->setVisible(false);
    m_isDragging = false;
  }
}

void ImageView::setDisplayMode(DisplayMode mode)
{
  m_displayMode = mode;
}

void ImageView::zoomIn()
{
  m_zoomFactor = std::min(m_zoomFactor * 1.25, 10.0);
  setTransform(QTransform::fromScale(m_zoomFactor, m_zoomFactor));
  emit zoomChanged(m_zoomFactor);
}

void ImageView::zoomOut()
{
  m_zoomFactor = std::max(m_zoomFactor * 0.8, 0.1);
  setTransform(QTransform::fromScale(m_zoomFactor, m_zoomFactor));
  emit zoomChanged(m_zoomFactor);
}

void ImageView::zoomFit()
{
  if (!m_imageItem->pixmap().isNull()) {
    fitInView(m_imageItem, Qt::KeepAspectRatio);
    m_zoomFactor = transform().m11();
    emit zoomChanged(m_zoomFactor);
  }
}

void ImageView::zoomActual()
{
  resetTransform();
  m_zoomFactor = 1.0;
  emit zoomChanged(m_zoomFactor);
}

void ImageView::wheelEvent(QWheelEvent* event)
{
  if (event->modifiers() & Qt::ControlModifier) {
    const double factor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    m_zoomFactor = std::clamp(m_zoomFactor * factor, 0.1, 10.0);
    setTransform(QTransform::fromScale(m_zoomFactor, m_zoomFactor));
    emit zoomChanged(m_zoomFactor);
  } else {
    QGraphicsView::wheelEvent(event);
  }
}

void ImageView::mouseMoveEvent(QMouseEvent* event)
{
  const QPointF scenePos = mapToScene(event->pos());
  if (m_roiEditEnabled && m_isDragging) {
    QRectF rect(m_dragStart, scenePos);
    rect = rect.normalized();
    rect = rect.intersected(m_scene->sceneRect());
    m_roiItem->setRect(rect);
    m_roiItem->setVisible(true);
  }

  if (!m_currentImage.isNull()) {
    const QPoint imagePos = QPoint(static_cast<int>(scenePos.x()),
                                   static_cast<int>(scenePos.y()));
    if (QRect(QPoint(0, 0), m_currentImage.size()).contains(imagePos)) {
      const auto pixel = m_currentImage.pixel(imagePos);
      const int gray = qGray(pixel);
      emit mousePositionChanged(imagePos, gray);
    }
  }
  QGraphicsView::mouseMoveEvent(event);
}

void ImageView::mousePressEvent(QMouseEvent* event)
{
  if (m_roiEditEnabled && event->button() == Qt::LeftButton) {
    m_isDragging = true;
    m_dragStart = mapToScene(event->pos());
    m_roiItem->setRect(QRectF(m_dragStart, QSizeF(0, 0)));
    m_roiItem->setVisible(true);
    event->accept();
    return;
  }
  QGraphicsView::mousePressEvent(event);
}

void ImageView::mouseReleaseEvent(QMouseEvent* event)
{
  if (m_roiEditEnabled && m_isDragging && event->button() == Qt::LeftButton) {
    m_isDragging = false;
    emit roiChanged(m_roiItem->rect().toRect());
    event->accept();
    return;
  }
  QGraphicsView::mouseReleaseEvent(event);
}

void ImageView::resizeEvent(QResizeEvent* event)
{
  QGraphicsView::resizeEvent(event);
  if (!m_isDragging) {
    zoomFit();
  }
}

QImage ImageView::cvMatToQImage(const cv::Mat& mat)
{
  switch (mat.type()) {
  case CV_8UC1:
    return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8).copy();
  case CV_8UC3: {
    cv::Mat rgb;
    cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
  }
  default:
    return QImage();
  }
}
