#include "ImageViewControls.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

ImageViewControls::ImageViewControls(QWidget* parent) : QWidget{parent}
{
  setupUI();
}

void ImageViewControls::setupUI()
{
  setObjectName(QStringLiteral("ImageViewControls"));
  setMaximumHeight(50);

  auto* mainLayout = new QHBoxLayout(this);
  mainLayout->setContentsMargins(8, 4, 8, 4);
  mainLayout->setSpacing(12);

  // 显示模式选择组
  auto* modeFrame = new QFrame(this);
  modeFrame->setFrameShape(QFrame::StyledPanel);
  auto* modeLayout = new QHBoxLayout(modeFrame);
  modeLayout->setContentsMargins(8, 4, 8, 4);
  modeLayout->setSpacing(8);

  auto* modeLabel = new QLabel(tr("显示模式:"), this);
  modeLayout->addWidget(modeLabel);

  m_displayModeGroup = new QButtonGroup(this);

  m_originalButton = new QRadioButton(tr("原图"), this);
  m_originalButton->setChecked(true);
  m_originalButton->setObjectName(QStringLiteral("originalModeButton"));
  m_displayModeGroup->addButton(m_originalButton, 0);
  modeLayout->addWidget(m_originalButton);

  m_annotatedButton = new QRadioButton(tr("标注图"), this);
  m_annotatedButton->setObjectName(QStringLiteral("annotatedModeButton"));
  m_displayModeGroup->addButton(m_annotatedButton, 1);
  modeLayout->addWidget(m_annotatedButton);

  mainLayout->addWidget(modeFrame);

  // 分隔线
  auto* separator1 = new QFrame(this);
  separator1->setFrameShape(QFrame::VLine);
  separator1->setFrameShadow(QFrame::Sunken);
  mainLayout->addWidget(separator1);

  // ROI显示选项
  m_showROICheck = new QCheckBox(tr("显示ROI"), this);
  m_showROICheck->setObjectName(QStringLiteral("showROICheck"));
  mainLayout->addWidget(m_showROICheck);

  // 分隔线
  auto* separator2 = new QFrame(this);
  separator2->setFrameShape(QFrame::VLine);
  separator2->setFrameShadow(QFrame::Sunken);
  mainLayout->addWidget(separator2);

  // 缩放控制组
  auto* zoomFrame = new QFrame(this);
  zoomFrame->setFrameShape(QFrame::StyledPanel);
  auto* zoomLayout = new QHBoxLayout(zoomFrame);
  zoomLayout->setContentsMargins(8, 4, 8, 4);
  zoomLayout->setSpacing(4);

  auto* zoomLabel = new QLabel(tr("缩放:"), this);
  zoomLayout->addWidget(zoomLabel);

  m_zoomOutButton = new QPushButton(tr("-"), this);
  m_zoomOutButton->setObjectName(QStringLiteral("zoomOutButton"));
  m_zoomOutButton->setMaximumWidth(30);
  m_zoomOutButton->setToolTip(tr("缩小"));
  zoomLayout->addWidget(m_zoomOutButton);

  m_zoomLabel = new QLabel(tr("100%"), this);
  m_zoomLabel->setObjectName(QStringLiteral("zoomLabel"));
  m_zoomLabel->setMinimumWidth(50);
  m_zoomLabel->setAlignment(Qt::AlignCenter);
  zoomLayout->addWidget(m_zoomLabel);

  m_zoomInButton = new QPushButton(tr("+"), this);
  m_zoomInButton->setObjectName(QStringLiteral("zoomInButton"));
  m_zoomInButton->setMaximumWidth(30);
  m_zoomInButton->setToolTip(tr("放大"));
  zoomLayout->addWidget(m_zoomInButton);

  m_zoomFitButton = new QPushButton(tr("适应"), this);
  m_zoomFitButton->setObjectName(QStringLiteral("zoomFitButton"));
  m_zoomFitButton->setToolTip(tr("适应窗口"));
  zoomLayout->addWidget(m_zoomFitButton);

  m_zoom100Button = new QPushButton(tr("100%"), this);
  m_zoom100Button->setObjectName(QStringLiteral("zoom100Button"));
  m_zoom100Button->setToolTip(tr("原始大小"));
  zoomLayout->addWidget(m_zoom100Button);

  mainLayout->addWidget(zoomFrame);

  // 添加弹性空间
  mainLayout->addStretch();

  // 连接信号
  connect(m_displayModeGroup, QOverload<int>::of(&QButtonGroup::idClicked),
          this, &ImageViewControls::displayModeChanged);

  connect(m_showROICheck, &QCheckBox::toggled,
          this, &ImageViewControls::showROIChanged);

  connect(m_zoomInButton, &QPushButton::clicked,
          this, &ImageViewControls::zoomInRequested);

  connect(m_zoomOutButton, &QPushButton::clicked,
          this, &ImageViewControls::zoomOutRequested);

  connect(m_zoomFitButton, &QPushButton::clicked,
          this, &ImageViewControls::zoomFitRequested);

  connect(m_zoom100Button, &QPushButton::clicked,
          this, &ImageViewControls::zoomActualRequested);
}

void ImageViewControls::setZoomLevel(double zoom)
{
  m_zoomLabel->setText(QString::number(static_cast<int>(zoom * 100)) + "%");
}