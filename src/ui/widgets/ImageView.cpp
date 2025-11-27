#include "ImageView.h"
#include <QPixmap>
#include <QWheelEvent>
#include <QGraphicsPixmapItem>

ImageView::ImageView(QWidget *parent) : QGraphicsView{parent}
{

}

void ImageView::setImage(const cv::Mat &image)
{
  if (image.empty()) {
    clear();
    return;
  }

  // cv::Mat → QImage 转换
  QImage qimg = cvMatToQImage(image);
  m_imageItem->setPixmap(QPixmap::fromImage(qimg));

  // 首次显示时自适应窗口
  if (m_zoomFactor == 1.0) {
    zoomFit();
  }
}

void ImageView::setImage(const QImage &image)
{

}

void ImageView::clear()
{

}

void ImageView::drawDefectRegions(const std::vector<cv::Rect> &regions, const QColor &color)
{

}

void ImageView::clearAnnotations()
{

}

void ImageView::setROI(const cv::Rect &roi)
{

}

cv::Rect ImageView::getROI() const
{

}

void ImageView::enableROIEdit(bool enable)
{

}

void ImageView::setDisplayMode(DisplayMode mode)
{

}

void ImageView::zoomIn()
{

}

void ImageView::zoomOut()
{

}

void ImageView::zoomFit()
{

}

void ImageView::zoomActual()
{

}

void ImageView::wheelEvent(QWheelEvent *event)
{
  // Ctrl + 滚轮缩放
  if (event->modifiers() & Qt::ControlModifier) {
    double factor = event->angleDelta().y() > 0 ? 1.15 : 0.85;
    m_zoomFactor *= factor;
    m_zoomFactor = std::clamp(m_zoomFactor, 0.1, 10.0);

    setTransform(QTransform::fromScale(m_zoomFactor, m_zoomFactor));
    emit zoomChanged(m_zoomFactor);
  } else {
    QGraphicsView::wheelEvent(event);
  }
}

void ImageView::mouseMoveEvent(QMouseEvent *event)
{

}

void ImageView::mousePressEvent(QMouseEvent *event)
{

}

void ImageView::mouseReleaseEvent(QMouseEvent *event)
{

}

void ImageView::resizeEvent(QResizeEvent *event)
{

}

QImage ImageView::cvMatToQImage(const cv::Mat &mat)
{
  switch (mat.type()) {
  case CV_8UC1:  // 灰度图
  {
    return QImage(mat.data, mat.cols, mat.rows,
                  mat.step, QImage::Format_Grayscale8).copy();
  }
  case CV_8UC3:  // BGR
  {
    cv::Mat rgb;
    cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    return QImage(rgb.data, rgb.cols, rgb.rows,
                  rgb.step, QImage::Format_RGB888).copy();
  }
  default:
    return QImage();
  }
}
