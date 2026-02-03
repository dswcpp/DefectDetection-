#include "ImagePreviewDialog.h"
#include "../widgets/ImageView.h"
#include "../widgets/AnnotationPanel.h"
#include "data/DatabaseManager.h"
#include "data/repositories/DefectRepository.h"
#include <opencv2/imgproc.hpp>  // for cvtColor

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QSlider>
#include <QStatusBar>
#include <QSplitter>
#include <QKeyEvent>
#include <QShowEvent>
#include <QCloseEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QStyle>

ImagePreviewDialog::ImagePreviewDialog(QWidget *parent)
    : FramelessDialog(parent) {
  setDialogTitle(tr("图片预览 - 缺陷标注"));
  setShowMaxButton(true);
  setupUI();
}

ImagePreviewDialog::~ImagePreviewDialog() {
}

void ImagePreviewDialog::setupUI() {
  // 设置窗口大小为屏幕的85%
  QScreen* screen = QGuiApplication::primaryScreen();
  QSize screenSize = screen->availableSize();
  resize(screenSize.width() * 0.85, screenSize.height() * 0.85);

  // 主布局
  auto* mainLayout = contentLayout();
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // 创建工具栏
  createToolBar();
  mainLayout->addWidget(m_toolbar);

  // 创建分割器（图片视图 + 标注面板）
  auto* splitter = new QSplitter(Qt::Horizontal);
  splitter->setHandleWidth(1);
  splitter->setStyleSheet(R"(
    QSplitter::handle {
      background-color: #334155;
    }
  )");

  // 图片视图
  m_imageView = new ImageView();
  m_imageView->setMinimumWidth(400);
  splitter->addWidget(m_imageView);

  // 标注面板
  m_annotationPanel = new AnnotationPanel();
  m_annotationPanel->setImageView(m_imageView);
  m_annotationPanel->setFixedWidth(280);
  splitter->addWidget(m_annotationPanel);

  // 设置分割比例
  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 0);
  splitter->setSizes({width() - 280, 280});

  mainLayout->addWidget(splitter, 1);

  // 创建状态栏
  createStatusBar();

  // 连接信号
  connect(m_imageView, &ImageView::zoomChanged, this, &ImagePreviewDialog::onZoomChanged);
  connect(m_imageView, &ImageView::mousePositionChanged, this, &ImagePreviewDialog::updateStatusInfo);
  connect(m_imageView, &ImageView::roiChanged, this, [this](const QRect& roi) {
    emit roiChanged(cv::Rect(roi.x(), roi.y(), roi.width(), roi.height()));
  });
  connect(m_imageView, &ImageView::defectAnnotationAdded, this, [this](const DefectAnnotation&) {
    emit annotationsChanged();
  });
  connect(m_imageView, &ImageView::defectAnnotationRemoved, this, [this](int) {
    emit annotationsChanged();
  });
  connect(m_imageView, &ImageView::defectAnnotationUpdated, this, [this](int, const DefectAnnotation&) {
    emit annotationsChanged();
  });

  // 设置样式
  setStyleSheet(R"(
    ImagePreviewDialog {
      background-color: #1e293b;
    }
  )");
}

void ImagePreviewDialog::createToolBar() {
  m_toolbar = new QToolBar();
  m_toolbar->setMovable(false);
  m_toolbar->setIconSize(QSize(20, 20));
  m_toolbar->setStyleSheet(R"(
    QToolBar {
      background-color: #0f172a;
      border: none;
      border-bottom: 1px solid #334155;
      padding: 4px 8px;
      spacing: 4px;
    }
    QToolButton {
      background-color: transparent;
      color: #e2e8f0;
      border: none;
      border-radius: 4px;
      padding: 6px 10px;
      margin: 0 2px;
    }
    QToolButton:hover {
      background-color: #334155;
    }
    QToolButton:pressed {
      background-color: #475569;
    }
    QToolButton:checked {
      background-color: #3b82f6;
      color: white;
    }
  )");

  // 缩放工具组
  m_zoomOutAction = m_toolbar->addAction(tr("缩小"));
  m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
  connect(m_zoomOutAction, &QAction::triggered, this, &ImagePreviewDialog::onZoomOut);

  // 缩放滑块
  m_zoomSlider = new QSlider(Qt::Horizontal);
  m_zoomSlider->setRange(10, 500);  // 10% - 500%
  m_zoomSlider->setValue(100);
  m_zoomSlider->setFixedWidth(120);
  m_zoomSlider->setStyleSheet(R"(
    QSlider::groove:horizontal {
      background: #334155;
      height: 4px;
      border-radius: 2px;
    }
    QSlider::handle:horizontal {
      background: #3b82f6;
      width: 12px;
      height: 12px;
      margin: -4px 0;
      border-radius: 6px;
    }
    QSlider::handle:horizontal:hover {
      background: #60a5fa;
    }
  )");
  connect(m_zoomSlider, &QSlider::valueChanged, this, &ImagePreviewDialog::onZoomSliderChanged);
  m_toolbar->addWidget(m_zoomSlider);

  m_zoomInAction = m_toolbar->addAction(tr("放大"));
  m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
  connect(m_zoomInAction, &QAction::triggered, this, &ImagePreviewDialog::onZoomIn);

  // 缩放比例标签
  m_zoomLabel = new QLabel("100%");
  m_zoomLabel->setFixedWidth(50);
  m_zoomLabel->setAlignment(Qt::AlignCenter);
  m_zoomLabel->setStyleSheet("color: #94a3b8; font-size: 12px;");
  m_toolbar->addWidget(m_zoomLabel);

  m_toolbar->addSeparator();

  m_zoomFitAction = m_toolbar->addAction(tr("适应窗口"));
  m_zoomFitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
  connect(m_zoomFitAction, &QAction::triggered, this, &ImagePreviewDialog::onZoomFit);

  m_zoomActualAction = m_toolbar->addAction(tr("1:1"));
  m_zoomActualAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));
  connect(m_zoomActualAction, &QAction::triggered, this, &ImagePreviewDialog::onZoomActual);

  m_toolbar->addSeparator();

  // 标注模式切换
  m_annotateAction = m_toolbar->addAction(tr("标注模式"));
  m_annotateAction->setCheckable(true);
  m_annotateAction->setShortcut(QKeySequence(Qt::Key_A));
  connect(m_annotateAction, &QAction::toggled, this, &ImagePreviewDialog::onToggleAnnotationMode);

  // ROI编辑切换
  m_roiAction = m_toolbar->addAction(tr("ROI编辑"));
  m_roiAction->setCheckable(true);
  m_roiAction->setShortcut(QKeySequence(Qt::Key_R));
  connect(m_roiAction, &QAction::toggled, this, &ImagePreviewDialog::onToggleROIMode);

  m_toolbar->addSeparator();

  // 保存和复制
  m_saveAction = m_toolbar->addAction(tr("保存"));
  m_saveAction->setShortcut(QKeySequence::Save);
  connect(m_saveAction, &QAction::triggered, this, &ImagePreviewDialog::onSaveImage);

  m_copyAction = m_toolbar->addAction(tr("复制"));
  m_copyAction->setShortcut(QKeySequence::Copy);
  connect(m_copyAction, &QAction::triggered, this, &ImagePreviewDialog::onCopyImage);

  m_toolbar->addSeparator();

  // 保存到数据库按钮
  m_saveToDbAction = m_toolbar->addAction(tr("保存标注"));
  m_saveToDbAction->setToolTip(tr("将标注保存到数据库"));
  m_saveToDbAction->setEnabled(false);  // 默认禁用，设置了数据库后启用
  connect(m_saveToDbAction, &QAction::triggered, this, &ImagePreviewDialog::onSaveToDatabase);

  // 右侧弹簧
  auto* spacer = new QWidget();
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_toolbar->addWidget(spacer);

  // 关闭按钮
  auto* closeAction = m_toolbar->addAction(tr("关闭"));
  closeAction->setShortcut(QKeySequence::Close);
  connect(closeAction, &QAction::triggered, this, &QDialog::close);
}

void ImagePreviewDialog::createStatusBar() {
  auto* statusWidget = new QWidget();
  statusWidget->setFixedHeight(28);
  statusWidget->setStyleSheet(R"(
    QWidget {
      background-color: #0f172a;
      border-top: 1px solid #334155;
    }
    QLabel {
      color: #94a3b8;
      font-size: 12px;
      padding: 0 8px;
    }
  )");

  auto* statusLayout = new QHBoxLayout(statusWidget);
  statusLayout->setContentsMargins(8, 0, 8, 0);
  statusLayout->setSpacing(16);

  m_positionLabel = new QLabel(tr("位置: -"));
  m_grayValueLabel = new QLabel(tr("灰度: -"));
  m_imageSizeLabel = new QLabel(tr("尺寸: -"));

  statusLayout->addWidget(m_positionLabel);
  statusLayout->addWidget(m_grayValueLabel);
  statusLayout->addStretch();
  statusLayout->addWidget(m_imageSizeLabel);

  auto* mainLayout = qobject_cast<QVBoxLayout*>(layout());
  if (mainLayout) {
    mainLayout->addWidget(statusWidget);
  }
}

void ImagePreviewDialog::setImage(const QImage& image) {
  if (image.isNull()) {
    return;
  }

  // 转换为cv::Mat传给ImageView
  cv::Mat mat;
  if (image.format() == QImage::Format_RGB888) {
    mat = cv::Mat(image.height(), image.width(), CV_8UC3, 
                  const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();
    cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
  } else if (image.format() == QImage::Format_Grayscale8) {
    mat = cv::Mat(image.height(), image.width(), CV_8UC1,
                  const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();
  } else {
    QImage converted = image.convertToFormat(QImage::Format_RGB888);
    mat = cv::Mat(converted.height(), converted.width(), CV_8UC3,
                  const_cast<uchar*>(converted.bits()), converted.bytesPerLine()).clone();
    cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
  }

  m_imageView->setImage(mat);
  m_imageSizeLabel->setText(tr("尺寸: %1 x %2").arg(image.width()).arg(image.height()));
  m_firstShow = true;
}

void ImagePreviewDialog::setImage(const QString& imagePath) {
  m_currentImagePath = imagePath;
  QImage image(imagePath);
  if (!image.isNull()) {
    setImage(image);
  }
}

void ImagePreviewDialog::setImage(const cv::Mat& image) {
  if (image.empty()) {
    return;
  }

  m_imageView->setImage(image);
  m_imageSizeLabel->setText(tr("尺寸: %1 x %2").arg(image.cols).arg(image.rows));
  m_firstShow = true;
}

void ImagePreviewDialog::setDefectRegions(const std::vector<cv::Rect>& regions, 
                                          const QColor& color) {
  m_imageView->drawDefectRegions(regions, color);
}

void ImagePreviewDialog::setDetectionResults(const std::vector<cv::Rect>& boxes,
                                             const QStringList& labels,
                                             const std::vector<double>& confidences) {
  m_imageView->clearAnnotations();

  for (size_t i = 0; i < boxes.size(); ++i) {
    DefectAnnotation annotation;
    annotation.boundingRect = boxes[i];
    annotation.shape = AnnotationShape::Rectangle;
    annotation.type = DefectType::Other;
    annotation.severity = DefectSeverity::Minor;
    annotation.isManual = false;
    
    if (i < static_cast<size_t>(labels.size())) {
      annotation.description = labels[i];
    }
    if (i < confidences.size()) {
      annotation.confidence = confidences[i];
    }

    m_imageView->addDefectAnnotation(annotation);
  }
}

void ImagePreviewDialog::setROI(const cv::Rect& roi) {
  m_imageView->setROI(roi);
}

cv::Rect ImagePreviewDialog::getROI() const {
  return m_imageView->getROI();
}

void ImagePreviewDialog::enableROIEdit(bool enable) {
  m_imageView->enableROIEdit(enable);
  m_roiAction->setChecked(enable);
}

QVector<DefectAnnotation> ImagePreviewDialog::getAnnotations() const {
  return m_imageView->getDefectAnnotations();
}

void ImagePreviewDialog::setToolbarVisible(bool visible) {
  m_toolbar->setVisible(visible);
}

void ImagePreviewDialog::setAnnotationPanelVisible(bool visible) {
  m_annotationPanel->setVisible(visible);
}

void ImagePreviewDialog::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    // 如果在标注模式或ROI模式，先退出模式
    if (m_annotateAction->isChecked()) {
      m_annotateAction->setChecked(false);
      return;
    }
    if (m_roiAction->isChecked()) {
      m_roiAction->setChecked(false);
      return;
    }
    close();
    return;
  }
  
  if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
    onZoomIn();
    return;
  }
  
  if (event->key() == Qt::Key_Minus) {
    onZoomOut();
    return;
  }

  QDialog::keyPressEvent(event);
}

void ImagePreviewDialog::showEvent(QShowEvent* event) {
  QDialog::showEvent(event);
  
  if (m_firstShow) {
    m_firstShow = false;
    QMetaObject::invokeMethod(this, [this]() {
      m_imageView->zoomFit();
    }, Qt::QueuedConnection);
  }
}

void ImagePreviewDialog::closeEvent(QCloseEvent* event) {
  // 退出标注模式
  if (m_annotateAction->isChecked()) {
    m_annotateAction->setChecked(false);
  }
  if (m_roiAction->isChecked()) {
    m_roiAction->setChecked(false);
  }
  
  QDialog::closeEvent(event);
}

void ImagePreviewDialog::onZoomIn() {
  m_imageView->zoomIn();
}

void ImagePreviewDialog::onZoomOut() {
  m_imageView->zoomOut();
}

void ImagePreviewDialog::onZoomFit() {
  m_imageView->zoomFit();
}

void ImagePreviewDialog::onZoomActual() {
  m_imageView->zoomActual();
}

void ImagePreviewDialog::onZoomSliderChanged(int value) {
  double zoom = value / 100.0;
  if (std::abs(zoom - m_currentZoom) > 0.01) {
    m_currentZoom = zoom;
    // 直接设置缩放（需要在ImageView中添加setZoom方法，暂时跳过）
    updateZoomLabel();
  }
}

void ImagePreviewDialog::onToggleAnnotationMode() {
  bool enable = m_annotateAction->isChecked();
  
  // 关闭ROI模式
  if (enable && m_roiAction->isChecked()) {
    m_roiAction->setChecked(false);
  }
  
  m_imageView->setAnnotationMode(enable);
}

void ImagePreviewDialog::onToggleROIMode() {
  bool enable = m_roiAction->isChecked();
  
  // 关闭标注模式
  if (enable && m_annotateAction->isChecked()) {
    m_annotateAction->setChecked(false);
  }
  
  m_imageView->enableROIEdit(enable);
}

void ImagePreviewDialog::onSaveImage() {
  QString defaultName;
  if (!m_currentImagePath.isEmpty()) {
    QFileInfo fi(m_currentImagePath);
    defaultName = fi.baseName() + "_annotated." + fi.suffix();
  } else {
    defaultName = QString("annotated_%1.png")
                    .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
  }

  QString filename = QFileDialog::getSaveFileName(this,
      tr("保存标注图像"),
      defaultName,
      tr("图像文件 (*.png *.jpg *.jpeg *.bmp)"));

  if (!filename.isEmpty()) {
    QImage annotatedImage = m_imageView->exportAnnotatedImage();
    if (!annotatedImage.isNull() && annotatedImage.save(filename)) {
      emit imageSaved(filename);
      QMessageBox::information(this, tr("成功"), 
          tr("图像已保存到: %1").arg(filename));
    } else {
      QMessageBox::warning(this, tr("错误"), tr("保存图像失败"));
    }
  }
}

void ImagePreviewDialog::onCopyImage() {
  QImage annotatedImage = m_imageView->exportAnnotatedImage();
  if (!annotatedImage.isNull()) {
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setImage(annotatedImage);
    // 可以添加状态栏提示
  }
}

void ImagePreviewDialog::onZoomChanged(double factor) {
  m_currentZoom = factor;
  
  // 更新滑块（阻止信号循环）
  m_zoomSlider->blockSignals(true);
  m_zoomSlider->setValue(static_cast<int>(factor * 100));
  m_zoomSlider->blockSignals(false);
  
  updateZoomLabel();
}

void ImagePreviewDialog::updateZoomLabel() {
  m_zoomLabel->setText(QString("%1%").arg(static_cast<int>(m_currentZoom * 100)));
}

void ImagePreviewDialog::updateStatusInfo(const QPoint& pos, int grayValue) {
  m_positionLabel->setText(tr("位置: (%1, %2)").arg(pos.x()).arg(pos.y()));
  m_grayValueLabel->setText(tr("灰度: %1").arg(grayValue));
}

void ImagePreviewDialog::setDatabaseManager(DatabaseManager* dbManager) {
  m_dbManager = dbManager;
  // 如果同时设置了数据库和记录ID，启用保存按钮
  if (m_saveToDbAction) {
    m_saveToDbAction->setEnabled(m_dbManager != nullptr && m_inspectionId > 0);
  }
}

void ImagePreviewDialog::setInspectionId(qint64 inspectionId) {
  m_inspectionId = inspectionId;
  // 如果同时设置了数据库和记录ID，启用保存按钮
  if (m_saveToDbAction) {
    m_saveToDbAction->setEnabled(m_dbManager != nullptr && m_inspectionId > 0);
  }
}

void ImagePreviewDialog::onSaveToDatabase() {
  if (!m_dbManager || !m_dbManager->isOpen() || m_inspectionId <= 0) {
    QMessageBox::warning(this, tr("错误"), tr("无法保存：数据库未连接或记录ID无效"));
    return;
  }

  auto* repo = m_dbManager->defectRepository();
  if (!repo) {
    QMessageBox::warning(this, tr("错误"), tr("无法获取数据仓储"));
    return;
  }

  // 获取当前标注
  QVector<DefectAnnotation> annotations = m_imageView->getDefectAnnotations();
  if (annotations.isEmpty()) {
    QMessageBox::information(this, tr("提示"), tr("没有标注需要保存"));
    return;
  }

  // 先删除该记录的旧缺陷数据（可选，根据需求决定是追加还是替换）
  // 这里选择替换模式：先获取旧数据，再插入新数据
  
  // 转换标注为 DefectRecord
  QVector<DefectRecord> defects;
  for (const auto& ann : annotations) {
    DefectRecord record;
    record.inspectionId = m_inspectionId;
    record.defectType = ann.description.isEmpty() ? tr("手动标注") : ann.description;
    record.confidence = ann.confidence > 0 ? ann.confidence : 1.0;
    record.severityScore = static_cast<int>(ann.severity) * 0.25;
    
    // 转换严重度等级
    switch (ann.severity) {
      case DefectSeverity::Critical:
        record.severityLevel = "Critical";
        break;
      case DefectSeverity::Major:
        record.severityLevel = "Major";
        break;
      case DefectSeverity::Minor:
        record.severityLevel = "Minor";
        break;
      default:
        record.severityLevel = "Minor";
        break;
    }
    
    record.region = QRect(ann.boundingRect.x, ann.boundingRect.y,
                          ann.boundingRect.width, ann.boundingRect.height);
    
    defects.append(record);
  }

  // 保存到数据库
  if (repo->insertDefects(defects)) {
    QMessageBox::information(this, tr("成功"), 
        tr("已保存 %1 条标注到数据库").arg(defects.size()));
    emit annotationsSavedToDatabase();
  } else {
    QMessageBox::warning(this, tr("错误"), tr("保存标注失败"));
  }
}
