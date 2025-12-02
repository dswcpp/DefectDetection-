#include "ImagePreviewDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QShowEvent>
#include <QScreen>
#include <QGuiApplication>

ImagePreviewDialog::ImagePreviewDialog(QWidget *parent)
    : QDialog(parent) {
  setupUI();
}

void ImagePreviewDialog::setupUI() {
  setWindowTitle(tr("图片预览"));
  setWindowFlags(Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
  setStyleSheet("background-color: #1a1a1a;");

  // 获取屏幕大小，设置为屏幕的80%
  QScreen* screen = QGuiApplication::primaryScreen();
  QSize screenSize = screen->availableSize();
  resize(screenSize.width() * 0.8, screenSize.height() * 0.8);

  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // 滚动区域
  m_scrollArea = new QScrollArea(this);
  m_scrollArea->setWidgetResizable(false);
  m_scrollArea->setAlignment(Qt::AlignCenter);
  m_scrollArea->setStyleSheet(R"(
    QScrollArea {
      border: none;
      background-color: #1a1a1a;
    }
    QScrollBar:vertical, QScrollBar:horizontal {
      background-color: #2a2a2a;
      width: 10px;
      height: 10px;
    }
    QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
      background-color: #4a4a4a;
      border-radius: 5px;
      min-height: 30px;
      min-width: 30px;
    }
    QScrollBar::handle:hover {
      background-color: #5a5a5a;
    }
  )");

  m_imageLabel = new QLabel();
  m_imageLabel->setAlignment(Qt::AlignCenter);
  m_imageLabel->setStyleSheet("background-color: transparent;");
  m_scrollArea->setWidget(m_imageLabel);

  mainLayout->addWidget(m_scrollArea, 1);

  // 底部工具栏
  auto* toolBar = new QWidget();
  toolBar->setFixedHeight(50);
  toolBar->setStyleSheet("background-color: #0a0a0a; border-top: 1px solid #333;");
  auto* toolLayout = new QHBoxLayout(toolBar);
  toolLayout->setContentsMargins(16, 0, 16, 0);

  auto* hintLabel = new QLabel(tr("滚轮缩放 | 双击/ESC关闭"));
  hintLabel->setStyleSheet("color: #666; font-size: 12px;");
  toolLayout->addWidget(hintLabel);

  toolLayout->addStretch();

  auto* zoomOutBtn = new QPushButton(tr("缩小 (-)"));
  zoomOutBtn->setStyleSheet(R"(
    QPushButton {
      background-color: #333;
      color: #ccc;
      border: none;
      border-radius: 4px;
      padding: 6px 16px;
    }
    QPushButton:hover { background-color: #444; }
  )");
  connect(zoomOutBtn, &QPushButton::clicked, this, [this]() {
    m_zoomFactor = qMax(0.1, m_zoomFactor - 0.1);
    updateZoom();
  });
  toolLayout->addWidget(zoomOutBtn);

  auto* zoomResetBtn = new QPushButton(tr("1:1"));
  zoomResetBtn->setStyleSheet(zoomOutBtn->styleSheet());
  connect(zoomResetBtn, &QPushButton::clicked, this, [this]() {
    m_zoomFactor = 1.0;
    updateZoom();
  });
  toolLayout->addWidget(zoomResetBtn);

  auto* zoomInBtn = new QPushButton(tr("放大 (+)"));
  zoomInBtn->setStyleSheet(zoomOutBtn->styleSheet());
  connect(zoomInBtn, &QPushButton::clicked, this, [this]() {
    m_zoomFactor = qMin(5.0, m_zoomFactor + 0.1);
    updateZoom();
  });
  toolLayout->addWidget(zoomInBtn);

  auto* zoomFitBtn = new QPushButton(tr("适应窗口"));
  zoomFitBtn->setStyleSheet(zoomOutBtn->styleSheet());
  connect(zoomFitBtn, &QPushButton::clicked, this, [this]() {
    if (m_originalImage.isNull()) return;
    QSize viewSize = m_scrollArea->viewport()->size();
    double scaleW = (double)viewSize.width() / m_originalImage.width();
    double scaleH = (double)viewSize.height() / m_originalImage.height();
    m_zoomFactor = qMin(scaleW, scaleH);
    updateZoom();
  });
  toolLayout->addWidget(zoomFitBtn);

  toolLayout->addSpacing(16);

  auto* closeBtn = new QPushButton(tr("关闭"));
  closeBtn->setStyleSheet(R"(
    QPushButton {
      background-color: #c62828;
      color: white;
      border: none;
      border-radius: 4px;
      padding: 6px 24px;
    }
    QPushButton:hover { background-color: #e53935; }
  )");
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
  toolLayout->addWidget(closeBtn);

  mainLayout->addWidget(toolBar);
}

void ImagePreviewDialog::setImage(const QImage& image) {
  m_originalImage = image;
  m_firstShow = true;  // 标记需要在显示时适配窗口
  
  if (image.isNull()) {
    m_imageLabel->setText(tr("无图片"));
    m_imageLabel->setStyleSheet("color: #666; font-size: 16px;");
    return;
  }

  // 如果对话框已经显示，立即适配
  if (isVisible()) {
    fitToWindow();
  }
}

void ImagePreviewDialog::setImage(const QString& imagePath) {
  QImage image(imagePath);
  setImage(image);
}

void ImagePreviewDialog::updateZoom() {
  if (m_originalImage.isNull()) return;

  int newWidth = m_originalImage.width() * m_zoomFactor;
  int newHeight = m_originalImage.height() * m_zoomFactor;

  QPixmap pixmap = QPixmap::fromImage(m_originalImage.scaled(
      newWidth, newHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  m_imageLabel->setPixmap(pixmap);
  m_imageLabel->resize(pixmap.size());
}

void ImagePreviewDialog::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    close();
    return;
  }
  if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
    m_zoomFactor = qMin(5.0, m_zoomFactor + 0.1);
    updateZoom();
    return;
  }
  if (event->key() == Qt::Key_Minus) {
    m_zoomFactor = qMax(0.1, m_zoomFactor - 0.1);
    updateZoom();
    return;
  }
  QDialog::keyPressEvent(event);
}

void ImagePreviewDialog::mouseDoubleClickEvent(QMouseEvent* event) {
  Q_UNUSED(event)
  close();
}

void ImagePreviewDialog::showEvent(QShowEvent* event) {
  QDialog::showEvent(event);
  
  // 首次显示时适配窗口
  if (m_firstShow && !m_originalImage.isNull()) {
    m_firstShow = false;
    // 延迟一帧确保布局完成
    QMetaObject::invokeMethod(this, &ImagePreviewDialog::fitToWindow, Qt::QueuedConnection);
  }
}

void ImagePreviewDialog::fitToWindow() {
  if (m_originalImage.isNull()) return;
  
  QSize viewSize = m_scrollArea->viewport()->size();
  if (viewSize.width() <= 0 || viewSize.height() <= 0) {
    viewSize = m_scrollArea->size() - QSize(4, 4);  // 备用尺寸
  }
  
  double scaleW = (double)viewSize.width() / m_originalImage.width();
  double scaleH = (double)viewSize.height() / m_originalImage.height();
  m_zoomFactor = qMin(scaleW, scaleH);
  m_zoomFactor = qMin(m_zoomFactor, 1.0);  // 不超过原始大小
  
  updateZoom();
}

void ImagePreviewDialog::wheelEvent(QWheelEvent* event) {
  double delta = event->angleDelta().y() / 1200.0;
  m_zoomFactor = qBound(0.1, m_zoomFactor + delta, 5.0);
  updateZoom();
  event->accept();
}
