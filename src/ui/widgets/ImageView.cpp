#include "ImageView.h"
#include "common/Logger.h"
#include <opencv2/imgproc.hpp>  // for cvtColor
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QPainter>
#include <QPen>
#include <QScrollBar>
#include <QWheelEvent>
#include <QFont>
#include <QCursor>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDateTime>
#include <algorithm>
#include <cmath>

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

  // 创建右键菜单
  createContextMenu();
}

ImageView::~ImageView()
{
  clearAnnotations();
}

void ImageView::setImage(const cv::Mat& image)
{
  if (image.empty()) {
    LOG_DEBUG("ImageView::setImage - Empty cv::Mat, clearing view");
    clear();
    return;
  }
  LOG_DEBUG("ImageView::setImage - cv::Mat {}x{}, channels={}", image.cols, image.rows, image.channels());
  const QImage qimg = cvMatToQImage(image);
  setImage(qimg);
}

void ImageView::setImage(const QImage& image)
{
  m_currentImage = image;
  m_imageItem->setPixmap(QPixmap::fromImage(image));
  m_scene->setSceneRect(QRectF(QPointF(0, 0), QSizeF(image.size())));
  m_roiItem->setVisible(false);

  // 自动调整视图以适应图片
  fitInView(m_imageItem, Qt::KeepAspectRatio);
  m_zoomFactor = transform().m11();

  // 确保缩放因子在合理范围内（0.1到50倍）
  m_zoomFactor = std::clamp(m_zoomFactor, 0.1, 50.0);

  LOG_DEBUG("ImageView::setImage - QImage {}x{}, zoom={:.2f}", image.width(), image.height(), m_zoomFactor);
  emit zoomChanged(m_zoomFactor);
}

void ImageView::clear()
{
  m_currentImage = QImage();
  m_imageItem->setPixmap(QPixmap());
  clearAnnotations();
  m_roiItem->setVisible(false);
}

// 缺陷标注 - 增强版实现
void ImageView::addDefectAnnotation(const DefectAnnotation& annotation)
{
  DefectAnnotation newAnnotation = annotation;
  newAnnotation.id = m_nextAnnotationId++;
  newAnnotation.timestamp = QDateTime::currentDateTime();

  m_defectAnnotations.append(newAnnotation);
  drawAnnotation(newAnnotation);

  emit defectAnnotationAdded(newAnnotation);
}

void ImageView::updateDefectAnnotation(int id, const DefectAnnotation& annotation)
{
  for (int i = 0; i < m_defectAnnotations.size(); ++i) {
    if (m_defectAnnotations[i].id == id) {
      m_defectAnnotations[i] = annotation;
      m_defectAnnotations[i].id = id; // 保持原ID
      updateAnnotationGraphics(id);
      emit defectAnnotationUpdated(id, annotation);
      break;
    }
  }
}

void ImageView::removeDefectAnnotation(int id)
{
  for (int i = 0; i < m_defectAnnotations.size(); ++i) {
    if (m_defectAnnotations[i].id == id) {
      m_defectAnnotations.removeAt(i);
      removeAnnotationGraphics(id);
      emit defectAnnotationRemoved(id);
      break;
    }
  }
}

void ImageView::clearAnnotations()
{
  // 清除新的标注系统
  for (auto& item : m_annotationGraphicsItems) {
    m_scene->removeItem(item);
    delete item;
  }
  m_annotationGraphicsItems.clear();

  for (auto& label : m_annotationLabels) {
    m_scene->removeItem(label);
    delete label;
  }
  m_annotationLabels.clear();

  m_defectAnnotations.clear();

  // 清除旧的兼容标注
  for (auto* item : m_defectItems) {
    m_scene->removeItem(item);
    delete item;
  }
  m_defectItems.clear();

  for (auto* label : m_labelItems) {
    m_scene->removeItem(label);
    delete label;
  }
  m_labelItems.clear();
}

QVector<DefectAnnotation> ImageView::getDefectAnnotations() const
{
  return m_defectAnnotations;
}

DefectAnnotation* ImageView::getDefectAnnotationAt(const QPoint& pos)
{
  QPointF scenePos = mapToScene(pos);
  for (auto& annotation : m_defectAnnotations) {
    if (isPointInAnnotation(scenePos, annotation)) {
      return &annotation;
    }
  }
  return nullptr;
}

// 手动标注模式
void ImageView::setAnnotationMode(bool enable)
{
  m_annotationMode = enable;

  if (enable) {
    setDragMode(QGraphicsView::NoDrag);
    setCursor(Qt::CrossCursor);
  } else {
    setDragMode(QGraphicsView::ScrollHandDrag);
    setCursor(Qt::ArrowCursor);
    cancelDrawing();
  }

  emit annotationModeChanged(enable);
}

void ImageView::setCurrentAnnotationShape(AnnotationShape shape)
{
  m_currentShape = shape;
  cancelDrawing();
}

void ImageView::setCurrentDefectType(DefectType type)
{
  m_currentDefectType = type;
}

void ImageView::setCurrentDefectSeverity(DefectSeverity severity)
{
  m_currentSeverity = severity;
}

// 显示控制
void ImageView::setHighlightSeverity(DefectSeverity severity, bool highlight)
{
  if (highlight) {
    m_highlightedSeverities.insert(severity);
  } else {
    m_highlightedSeverities.remove(severity);
  }
  updateDisplay();
}

// 兼容旧接口
void ImageView::drawDefectRegions(const std::vector<cv::Rect>& regions, const QColor& color)
{
  clearAnnotations();
  for (const auto& rect : regions) {
    DefectAnnotation annotation;
    annotation.boundingRect = rect;
    annotation.shape = AnnotationShape::Rectangle;
    annotation.type = DefectType::Other;
    annotation.severity = DefectSeverity::Minor;
    annotation.color = color;
    annotation.isManual = false;
    addDefectAnnotation(annotation);
  }
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
  // 转换为新的标注系统
  DefectAnnotation annotation;
  annotation.boundingRect = box.rect;
  annotation.shape = AnnotationShape::Rectangle;
  annotation.type = DefectType::Other;
  annotation.severity = DefectSeverity::Minor;
  annotation.color = box.color;
  annotation.confidence = box.confidence;
  annotation.isManual = false;
  annotation.description = box.label;

  addDefectAnnotation(annotation);
}

// ROI编辑
void ImageView::setROI(const cv::Rect& roi)
{
  if (roi.width <= 0 || roi.height <= 0) {
    m_roiItem->setRect(QRectF());
    m_roiItem->setVisible(false);
    return;
  }

  QRectF rect(roi.x, roi.y, roi.width, roi.height);
  rect = rect.intersected(m_scene->sceneRect());
  m_roiItem->setRect(rect);
  m_roiItem->setVisible(m_showROI);  // 根据显示设置决定是否可见
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
  if (m_displayMode == mode) return;
  
  m_displayMode = mode;
  
  // 根据模式控制标注显示
  if (mode == DisplayMode::Original) {
    // 原图模式：隐藏所有标注
    setShowAnnotations(false);
  } else if (mode == DisplayMode::Annotated) {
    // 标注图模式：显示所有标注
    setShowAnnotations(true);
  }
  
  updateDisplay();
}

void ImageView::setShowROI(bool show)
{
  m_showROI = show;
  m_roiItem->setVisible(show && m_roiItem->rect().isValid());
}

void ImageView::setShowAnnotations(bool show)
{
  m_showAnnotations = show;
  
  // 显示或隐藏所有标注图形项
  for (auto* item : m_annotationGraphicsItems) {
    if (item) item->setVisible(show);
  }
  for (auto* label : m_annotationLabels) {
    if (label) label->setVisible(show);
  }
  
  // 旧版兼容的缺陷项
  for (auto* item : m_defectItems) {
    if (item) item->setVisible(show);
  }
  for (auto* label : m_labelItems) {
    if (label) label->setVisible(show);
  }
}

// 缩放功能
void ImageView::zoomIn()
{
  QPoint mousePos = viewport()->mapFromGlobal(QCursor::pos());
  QPointF targetScenePos;

  if (viewport()->rect().contains(mousePos)) {
    targetScenePos = mapToScene(mousePos);
  } else {
    targetScenePos = mapToScene(viewport()->rect().center());
  }

  m_zoomFactor = std::min(m_zoomFactor * 1.25, 50.0);
  setTransform(QTransform::fromScale(m_zoomFactor, m_zoomFactor));
  centerOn(targetScenePos);

  emit zoomChanged(m_zoomFactor);
}

void ImageView::zoomOut()
{
  QPoint mousePos = viewport()->mapFromGlobal(QCursor::pos());
  QPointF targetScenePos;

  if (viewport()->rect().contains(mousePos)) {
    targetScenePos = mapToScene(mousePos);
  } else {
    targetScenePos = mapToScene(viewport()->rect().center());
  }

  m_zoomFactor = std::max(m_zoomFactor * 0.8, 0.1);
  setTransform(QTransform::fromScale(m_zoomFactor, m_zoomFactor));
  centerOn(targetScenePos);

  emit zoomChanged(m_zoomFactor);
}

void ImageView::zoomFit()
{
  if (!m_imageItem->pixmap().isNull()) {
    QPointF oldCenter = mapToScene(viewport()->rect().center());

    fitInView(m_imageItem, Qt::KeepAspectRatio);
    m_zoomFactor = transform().m11();

    m_zoomFactor = std::clamp(m_zoomFactor, 0.1, 50.0);
    setTransform(QTransform::fromScale(m_zoomFactor, m_zoomFactor));

    centerOn(oldCenter);

    emit zoomChanged(m_zoomFactor);
  }
}

void ImageView::zoomActual()
{
  resetTransform();
  m_zoomFactor = 1.0;
  emit zoomChanged(m_zoomFactor);
}

// 导出功能
QImage ImageView::exportAnnotatedImage() const
{
  if (m_currentImage.isNull()) {
    return QImage();
  }

  QImage result = m_currentImage.copy();
  QPainter painter(&result);
  painter.setRenderHint(QPainter::Antialiasing);

  for (const auto& annotation : m_defectAnnotations) {
    QColor color = annotation.color.isValid() ? annotation.color : severityToColor(annotation.severity);
    QPen pen(color, 2);
    painter.setPen(pen);

    QColor fillColor = color;
    fillColor.setAlpha(30);
    painter.setBrush(fillColor);

    QRect rect(annotation.boundingRect.x, annotation.boundingRect.y,
               annotation.boundingRect.width, annotation.boundingRect.height);

    if (annotation.shape == AnnotationShape::Rectangle) {
      painter.drawRect(rect);
    } else if (annotation.shape == AnnotationShape::Circle) {
      painter.drawEllipse(rect);
    }

    // 绘制标签
    if (m_showSeverityLabels) {
      QString label = QString("[%1] %2").arg(severityToString(annotation.severity))
                                        .arg(defectTypeToString(annotation.type));
      if (m_showConfidence && annotation.confidence > 0) {
        label += QString(" %1%").arg(static_cast<int>(annotation.confidence * 100));
      }

      QFont font("Arial", 10, QFont::Bold);
      painter.setFont(font);
      painter.setPen(Qt::white);
      painter.setBrush(color);

      QRectF textRect = painter.boundingRect(QRectF(), Qt::AlignLeft, label);
      textRect.moveTo(rect.x(), rect.y() - textRect.height() - 2);
      painter.fillRect(textRect, color);
      painter.drawText(textRect, Qt::AlignCenter, label);
    }
  }

  return result;
}

bool ImageView::saveAnnotations(const QString& filePath) const
{
  QJsonObject root;
  root["version"] = "1.0";
  root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

  QJsonArray annotationsArray;
  for (const auto& annotation : m_defectAnnotations) {
    QJsonObject obj;
    obj["id"] = annotation.id;
    obj["shape"] = static_cast<int>(annotation.shape);
    obj["type"] = static_cast<int>(annotation.type);
    obj["severity"] = static_cast<int>(annotation.severity);
    obj["description"] = annotation.description;
    obj["confidence"] = annotation.confidence;
    obj["isManual"] = annotation.isManual;

    QJsonObject rect;
    rect["x"] = annotation.boundingRect.x;
    rect["y"] = annotation.boundingRect.y;
    rect["width"] = annotation.boundingRect.width;
    rect["height"] = annotation.boundingRect.height;
    obj["boundingRect"] = rect;

    annotationsArray.append(obj);
  }
  root["annotations"] = annotationsArray;

  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  }

  QJsonDocument doc(root);
  file.write(doc.toJson());
  return true;
}

bool ImageView::loadAnnotations(const QString& filePath)
{
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    return false;
  }

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  if (!doc.isObject()) {
    return false;
  }

  clearAnnotations();

  QJsonObject root = doc.object();
  QJsonArray annotationsArray = root["annotations"].toArray();

  for (const QJsonValue& value : annotationsArray) {
    QJsonObject obj = value.toObject();
    DefectAnnotation annotation;

    annotation.id = obj["id"].toInt();
    annotation.shape = static_cast<AnnotationShape>(obj["shape"].toInt());
    annotation.type = static_cast<DefectType>(obj["type"].toInt());
    annotation.severity = static_cast<DefectSeverity>(obj["severity"].toInt());
    annotation.description = obj["description"].toString();
    annotation.confidence = obj["confidence"].toDouble();
    annotation.isManual = obj["isManual"].toBool();

    QJsonObject rect = obj["boundingRect"].toObject();
    annotation.boundingRect = cv::Rect(rect["x"].toInt(), rect["y"].toInt(),
                                       rect["width"].toInt(), rect["height"].toInt());

    annotation.color = severityToColor(annotation.severity);
    m_defectAnnotations.append(annotation);
    drawAnnotation(annotation);
  }

  m_nextAnnotationId = m_defectAnnotations.size() + 1;
  return true;
}

// 事件处理
void ImageView::wheelEvent(QWheelEvent* event)
{
  if (event->modifiers() & Qt::ControlModifier) {
    QPointF scenePos = mapToScene(event->position().toPoint());
    const double factor = event->angleDelta().y() > 0 ? 1.1 : 0.9;

    m_zoomFactor = std::clamp(m_zoomFactor * factor, 0.1, 50.0);
    setTransform(QTransform::fromScale(m_zoomFactor, m_zoomFactor));

    centerOn(scenePos);

    QPointF deltaViewportPos = event->position() - QPointF(viewport()->rect().center());
    QPointF viewportCenter = mapFromScene(scenePos) - deltaViewportPos;
    centerOn(mapToScene(viewportCenter.toPoint()));

    emit zoomChanged(m_zoomFactor);
    event->accept();
  } else {
    QGraphicsView::wheelEvent(event);
  }
}

void ImageView::mouseMoveEvent(QMouseEvent* event)
{
  const QPointF scenePos = mapToScene(event->pos());

  // 标注模式下的绘制
  if (m_annotationMode && m_isDrawing) {
    m_drawingEnd = scenePos;

    if (m_tempDrawingItem) {
      m_scene->removeItem(m_tempDrawingItem);
      delete m_tempDrawingItem;
      m_tempDrawingItem = nullptr;
    }

    QColor color = severityToColor(m_currentSeverity);
    QPen pen(color, 2);
    QColor fillColor = color;
    fillColor.setAlpha(30);

    if (m_currentShape == AnnotationShape::Rectangle) {
      QRectF rect(m_drawingStart, m_drawingEnd);
      rect = rect.normalized();
      auto* rectItem = new QGraphicsRectItem(rect);
      rectItem->setPen(pen);
      rectItem->setBrush(fillColor);
      m_tempDrawingItem = rectItem;
    } else if (m_currentShape == AnnotationShape::Circle) {
      QRectF rect(m_drawingStart, m_drawingEnd);
      rect = rect.normalized();
      auto* ellipseItem = new QGraphicsEllipseItem(rect);
      ellipseItem->setPen(pen);
      ellipseItem->setBrush(fillColor);
      m_tempDrawingItem = ellipseItem;
    }

    if (m_tempDrawingItem) {
      m_scene->addItem(m_tempDrawingItem);
    }
  }
  // ROI编辑模式
  else if (m_roiEditEnabled && m_isDragging) {
    QRectF rect(m_dragStart, scenePos);
    rect = rect.normalized();
    rect = rect.intersected(m_scene->sceneRect());
    m_roiItem->setRect(rect);
    m_roiItem->setVisible(true);
  }
  // 移动标注
  else if (m_isMovingAnnotation && m_selectedAnnotationId >= 0) {
    // 实现标注移动逻辑
  }

  // 更新鼠标位置信息
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
  const QPointF scenePos = mapToScene(event->pos());

  if (m_annotationMode && event->button() == Qt::LeftButton) {
    if (m_currentShape == AnnotationShape::Polygon) {
      m_polygonPoints.append(scenePos);
      // TODO: 绘制多边形顶点
    } else {
      m_isDrawing = true;
      m_drawingStart = scenePos;
      m_drawingEnd = scenePos;
    }
    event->accept();
    return;
  }
  else if (m_roiEditEnabled && event->button() == Qt::LeftButton) {
    m_isDragging = true;
    m_dragStart = scenePos;
    m_roiItem->setRect(QRectF(m_dragStart, QSizeF(0, 0)));
    m_roiItem->setVisible(true);
    event->accept();
    return;
  }
  else if (!m_annotationMode && event->button() == Qt::LeftButton) {
    // 选择标注
    for (const auto& annotation : m_defectAnnotations) {
      if (isPointInAnnotation(scenePos, annotation)) {
        selectAnnotation(annotation.id);
        m_isMovingAnnotation = true;
        m_moveOffset = scenePos - QPointF(annotation.boundingRect.x, annotation.boundingRect.y);
        event->accept();
        return;
      }
    }
  }

  QGraphicsView::mousePressEvent(event);
}

void ImageView::mouseReleaseEvent(QMouseEvent* event)
{
  if (m_annotationMode && m_isDrawing && event->button() == Qt::LeftButton) {
    m_isDrawing = false;

    if (m_tempDrawingItem) {
      m_scene->removeItem(m_tempDrawingItem);
      delete m_tempDrawingItem;
      m_tempDrawingItem = nullptr;
    }

    QRectF rect(m_drawingStart, m_drawingEnd);
    rect = rect.normalized();

    if (rect.width() > 5 && rect.height() > 5) { // 最小尺寸限制
      DefectAnnotation annotation = createAnnotationFromRect(rect);
      addDefectAnnotation(annotation);
    }

    event->accept();
    return;
  }
  else if (m_roiEditEnabled && m_isDragging && event->button() == Qt::LeftButton) {
    m_isDragging = false;
    emit roiChanged(m_roiItem->rect().toRect());
    event->accept();
    return;
  }
  else if (m_isMovingAnnotation) {
    m_isMovingAnnotation = false;
  }

  QGraphicsView::mouseReleaseEvent(event);
}

void ImageView::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (m_annotationMode && m_currentShape == AnnotationShape::Polygon) {
    finishPolygonDrawing();
    event->accept();
    return;
  }

  QGraphicsView::mouseDoubleClickEvent(event);
}

void ImageView::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Escape) {
    if (m_annotationMode) {
      cancelDrawing();
      event->accept();
      return;
    }
  }
  else if (event->key() == Qt::Key_Delete) {
    if (m_selectedAnnotationId >= 0) {
      removeDefectAnnotation(m_selectedAnnotationId);
      m_selectedAnnotationId = -1;
      event->accept();
      return;
    }
  }

  QGraphicsView::keyPressEvent(event);
}

void ImageView::contextMenuEvent(QContextMenuEvent* event)
{
  if (!m_annotationMode) {
    QPointF scenePos = mapToScene(event->pos());
    DefectAnnotation* annotation = nullptr;

    for (auto& ann : m_defectAnnotations) {
      if (isPointInAnnotation(scenePos, ann)) {
        annotation = &ann;
        m_selectedAnnotationId = ann.id;
        break;
      }
    }

    if (annotation) {
      m_contextMenu->exec(event->globalPos());
    }
  }

  QGraphicsView::contextMenuEvent(event);
}

void ImageView::resizeEvent(QResizeEvent* event)
{
  QGraphicsView::resizeEvent(event);
}

// 私有方法实现
void ImageView::updateDisplay()
{
  // 重新绘制所有标注
  for (auto& item : m_annotationGraphicsItems) {
    m_scene->removeItem(item);
    delete item;
  }
  m_annotationGraphicsItems.clear();

  for (auto& label : m_annotationLabels) {
    m_scene->removeItem(label);
    delete label;
  }
  m_annotationLabels.clear();

  for (const auto& annotation : m_defectAnnotations) {
    drawAnnotation(annotation);
  }
}

void ImageView::drawAnnotation(const DefectAnnotation& annotation)
{
  QColor color = annotation.color.isValid() ? annotation.color : severityToColor(annotation.severity);

  // 高亮处理
  bool highlight = m_highlightedSeverities.contains(annotation.severity);
  if (highlight) {
    color.setAlpha(255);
  }

  QPen pen(color, highlight ? 3 : 2);
  if (annotation.id == m_selectedAnnotationId) {
    pen.setStyle(Qt::DashLine);
  }

  QColor fillColor = color;
  fillColor.setAlpha(30);

  QGraphicsItem* item = nullptr;
  QRect rect(annotation.boundingRect.x, annotation.boundingRect.y,
             annotation.boundingRect.width, annotation.boundingRect.height);

  if (annotation.shape == AnnotationShape::Rectangle) {
    auto* rectItem = new QGraphicsRectItem(rect);
    rectItem->setPen(pen);
    rectItem->setBrush(fillColor);
    item = rectItem;
  } else if (annotation.shape == AnnotationShape::Circle) {
    auto* ellipseItem = new QGraphicsEllipseItem(rect);
    ellipseItem->setPen(pen);
    ellipseItem->setBrush(fillColor);
    item = ellipseItem;
  } else if (annotation.shape == AnnotationShape::Polygon && !annotation.points.empty()) {
    QPolygonF polygon;
    for (const auto& pt : annotation.points) {
      polygon << QPointF(pt.x, pt.y);
    }
    auto* polygonItem = new QGraphicsPolygonItem(polygon);
    polygonItem->setPen(pen);
    polygonItem->setBrush(fillColor);
    item = polygonItem;
  }

  if (item) {
    item->setVisible(m_showAnnotations);  // 根据显示模式设置可见性
    m_scene->addItem(item);
    m_annotationGraphicsItems[annotation.id] = item;

    // 添加标签
    if (m_showSeverityLabels) {
      auto* textItem = new QGraphicsTextItem();
      QString label = QString("[%1] %2").arg(severityToString(annotation.severity))
                                        .arg(defectTypeToString(annotation.type));

      if (!annotation.description.isEmpty()) {
        label = annotation.description + " " + label;
      }

      if (m_showConfidence && annotation.confidence > 0) {
        label += QString(" %1%").arg(static_cast<int>(annotation.confidence * 100));
      }

      textItem->setPlainText(label);
      QFont font("Arial", 10, QFont::Bold);
      textItem->setFont(font);
      textItem->setDefaultTextColor(Qt::white);

      // 背景
      QRectF textRect = textItem->boundingRect();
      auto* bgRect = new QGraphicsRectItem(textRect);
      bgRect->setBrush(color);
      bgRect->setPen(Qt::NoPen);

      bgRect->setPos(rect.x(), rect.y() - textRect.height() - 2);
      textItem->setPos(rect.x() + 2, rect.y() - textRect.height() - 2);

      bgRect->setVisible(m_showAnnotations);  // 根据显示模式设置可见性
      textItem->setVisible(m_showAnnotations);  // 根据显示模式设置可见性
      
      m_scene->addItem(bgRect);
      m_scene->addItem(textItem);
      m_annotationGraphicsItems[annotation.id + 10000] = bgRect; // 使用偏移ID存储背景
      m_annotationLabels[annotation.id] = textItem;
    }
  }
}

void ImageView::removeAnnotationGraphics(int id)
{
  if (m_annotationGraphicsItems.contains(id)) {
    m_scene->removeItem(m_annotationGraphicsItems[id]);
    delete m_annotationGraphicsItems[id];
    m_annotationGraphicsItems.remove(id);
  }

  if (m_annotationGraphicsItems.contains(id + 10000)) {
    m_scene->removeItem(m_annotationGraphicsItems[id + 10000]);
    delete m_annotationGraphicsItems[id + 10000];
    m_annotationGraphicsItems.remove(id + 10000);
  }

  if (m_annotationLabels.contains(id)) {
    m_scene->removeItem(m_annotationLabels[id]);
    delete m_annotationLabels[id];
    m_annotationLabels.remove(id);
  }
}

void ImageView::selectAnnotation(int id)
{
  m_selectedAnnotationId = id;
  updateDisplay();
  emit defectAnnotationSelected(id);
}

void ImageView::createContextMenu()
{
  m_contextMenu = new QMenu(this);

  QAction* editAction = m_contextMenu->addAction(tr("编辑标注"));
  connect(editAction, &QAction::triggered, this, &ImageView::onEditAnnotation);

  QAction* severityAction = m_contextMenu->addAction(tr("更改严重等级"));
  connect(severityAction, &QAction::triggered, this, &ImageView::onChangeAnnotationSeverity);

  QAction* typeAction = m_contextMenu->addAction(tr("更改缺陷类型"));
  connect(typeAction, &QAction::triggered, this, &ImageView::onChangeAnnotationType);

  m_contextMenu->addSeparator();

  QAction* deleteAction = m_contextMenu->addAction(tr("删除标注"));
  deleteAction->setShortcut(QKeySequence::Delete);
  connect(deleteAction, &QAction::triggered, this, &ImageView::onDeleteAnnotation);
}

void ImageView::updateAnnotationGraphics(int id)
{
  removeAnnotationGraphics(id);

  for (const auto& annotation : m_defectAnnotations) {
    if (annotation.id == id) {
      drawAnnotation(annotation);
      break;
    }
  }
}

void ImageView::finishPolygonDrawing()
{
  if (m_polygonPoints.size() >= 3) {
    DefectAnnotation annotation;
    annotation.shape = AnnotationShape::Polygon;
    annotation.type = m_currentDefectType;
    annotation.severity = m_currentSeverity;
    annotation.isManual = true;
    annotation.color = severityToColor(m_currentSeverity);

    // 计算边界框
    int minX = INT_MAX, minY = INT_MAX;
    int maxX = INT_MIN, maxY = INT_MIN;

    for (const auto& pt : m_polygonPoints) {
      annotation.points.push_back(cv::Point(pt.x(), pt.y()));
      minX = std::min(minX, static_cast<int>(pt.x()));
      minY = std::min(minY, static_cast<int>(pt.y()));
      maxX = std::max(maxX, static_cast<int>(pt.x()));
      maxY = std::max(maxY, static_cast<int>(pt.y()));
    }

    annotation.boundingRect = cv::Rect(minX, minY, maxX - minX, maxY - minY);
    addDefectAnnotation(annotation);
  }

  m_polygonPoints.clear();
}

void ImageView::cancelDrawing()
{
  m_isDrawing = false;
  m_polygonPoints.clear();

  if (m_tempDrawingItem) {
    m_scene->removeItem(m_tempDrawingItem);
    delete m_tempDrawingItem;
    m_tempDrawingItem = nullptr;
  }
}

// 辅助方法
QColor ImageView::severityToColor(DefectSeverity severity) const
{
  switch (severity) {
    case DefectSeverity::Critical:
      return QColor("#e53e3e"); // 红色
    case DefectSeverity::Major:
      return QColor("#dd6b20"); // 橙色
    case DefectSeverity::Minor:
      return QColor("#d69e2e"); // 黄色
    case DefectSeverity::Info:
      return QColor("#3182ce"); // 蓝色
    default:
      return QColor("#718096"); // 灰色
  }
}

QString ImageView::severityToString(DefectSeverity severity) const
{
  switch (severity) {
    case DefectSeverity::Critical: return tr("严重");
    case DefectSeverity::Major: return tr("重大");
    case DefectSeverity::Minor: return tr("轻微");
    case DefectSeverity::Info: return tr("信息");
    default: return tr("未知");
  }
}

QString ImageView::defectTypeToString(DefectType type) const
{
  switch (type) {
    case DefectType::Scratch: return tr("划痕");
    case DefectType::Crack: return tr("裂纹");
    case DefectType::Bubble: return tr("气泡");
    case DefectType::ForeignObject: return tr("异物");
    case DefectType::Deformation: return tr("变形");
    case DefectType::ColorDefect: return tr("色差");
    case DefectType::Other: return tr("其他");
    default: return tr("未知");
  }
}

DefectAnnotation ImageView::createAnnotationFromRect(const QRectF& rect)
{
  DefectAnnotation annotation;
  annotation.boundingRect = cv::Rect(rect.x(), rect.y(), rect.width(), rect.height());
  annotation.shape = m_currentShape;
  annotation.type = m_currentDefectType;
  annotation.severity = m_currentSeverity;
  annotation.isManual = true;
  annotation.color = severityToColor(m_currentSeverity);
  annotation.timestamp = QDateTime::currentDateTime();

  return annotation;
}

bool ImageView::isPointInAnnotation(const QPointF& point, const DefectAnnotation& annotation) const
{
  QRectF rect(annotation.boundingRect.x, annotation.boundingRect.y,
              annotation.boundingRect.width, annotation.boundingRect.height);

  if (annotation.shape == AnnotationShape::Rectangle) {
    return rect.contains(point);
  } else if (annotation.shape == AnnotationShape::Circle) {
    QPointF center = rect.center();
    double rx = rect.width() / 2.0;
    double ry = rect.height() / 2.0;
    double dx = (point.x() - center.x()) / rx;
    double dy = (point.y() - center.y()) / ry;
    return (dx * dx + dy * dy) <= 1.0;
  } else if (annotation.shape == AnnotationShape::Polygon && !annotation.points.empty()) {
    QPolygonF polygon;
    for (const auto& pt : annotation.points) {
      polygon << QPointF(pt.x, pt.y);
    }
    return polygon.containsPoint(point, Qt::OddEvenFill);
  }

  return false;
}

// Slots实现
void ImageView::onEditAnnotation()
{
  if (m_selectedAnnotationId < 0) return;

  for (auto& annotation : m_defectAnnotations) {
    if (annotation.id == m_selectedAnnotationId) {
      bool ok;
      QString text = QInputDialog::getText(this, tr("编辑标注"),
                                          tr("描述:"), QLineEdit::Normal,
                                          annotation.description, &ok);
      if (ok) {
        annotation.description = text;
        updateAnnotationGraphics(m_selectedAnnotationId);
        emit defectAnnotationUpdated(m_selectedAnnotationId, annotation);
      }
      break;
    }
  }
}

void ImageView::onDeleteAnnotation()
{
  if (m_selectedAnnotationId >= 0) {
    removeDefectAnnotation(m_selectedAnnotationId);
    m_selectedAnnotationId = -1;
  }
}

void ImageView::onChangeAnnotationSeverity()
{
  if (m_selectedAnnotationId < 0) return;

  QStringList items;
  items << tr("严重") << tr("重大") << tr("轻微") << tr("信息");

  bool ok;
  QString item = QInputDialog::getItem(this, tr("更改严重等级"),
                                       tr("选择新的严重等级:"), items, 0, false, &ok);
  if (ok) {
    int index = items.indexOf(item);
    if (index >= 0) {
      for (auto& annotation : m_defectAnnotations) {
        if (annotation.id == m_selectedAnnotationId) {
          annotation.severity = static_cast<DefectSeverity>(index);
          annotation.color = severityToColor(annotation.severity);
          updateAnnotationGraphics(m_selectedAnnotationId);
          emit defectAnnotationUpdated(m_selectedAnnotationId, annotation);
          break;
        }
      }
    }
  }
}

void ImageView::onChangeAnnotationType()
{
  if (m_selectedAnnotationId < 0) return;

  QStringList items;
  items << tr("划痕") << tr("裂纹") << tr("气泡") << tr("异物")
        << tr("变形") << tr("色差") << tr("其他");

  bool ok;
  QString item = QInputDialog::getItem(this, tr("更改缺陷类型"),
                                       tr("选择新的缺陷类型:"), items, 0, false, &ok);
  if (ok) {
    int index = items.indexOf(item);
    if (index >= 0) {
      for (auto& annotation : m_defectAnnotations) {
        if (annotation.id == m_selectedAnnotationId) {
          annotation.type = static_cast<DefectType>(index);
          updateAnnotationGraphics(m_selectedAnnotationId);
          emit defectAnnotationUpdated(m_selectedAnnotationId, annotation);
          break;
        }
      }
    }
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