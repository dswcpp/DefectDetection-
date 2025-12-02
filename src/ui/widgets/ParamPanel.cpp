#include "ParamPanel.h"
#include "config/ConfigManager.h"
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSlider>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPropertyAnimation>
#include <QScrollArea>

namespace {
constexpr auto kScratchDetector = "scratch";
constexpr auto kCrackDetector = "crack";
constexpr auto kForeignDetector = "foreign";
constexpr auto kDimensionDetector = "dimension";
}

ParamPanel::ParamPanel(QWidget* parent) : QWidget{parent}
{
  setupUI();
  loadFromConfig();

  // 监听配置变化
  connect(&gConfig, &ConfigManager::configChanged, this, [this](const QString& section) {
    if (section.startsWith("detectors")) {
      loadFromConfig();
    }
  });
}

void ParamPanel::setupUI()
{
  setObjectName(QStringLiteral("ParamPanel"));

  // 设置最小高度，确保有足够空间显示内容
  setMinimumHeight(500);

  // 创建主布局
  auto* scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);

  auto* scrollWidget = new QWidget();
  m_mainLayout = new QVBoxLayout(scrollWidget);
  m_mainLayout->setContentsMargins(12, 12, 12, 12);  // 增加边距
  m_mainLayout->setSpacing(8);  // 增加组之间的间距

  scrollArea->setWidget(scrollWidget);

  auto* rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->addWidget(scrollArea);

  // 创建划痕检测参数组
  auto* scratchWidget = createCollapsibleSection(tr("划痕检测"),
                                                 createScratchParams(),
                                                 true);

  // 创建裂纹检测参数组
  auto* crackWidget = createCollapsibleSection(tr("裂纹检测"),
                                               createCrackParams(),
                                               false);

  // 创建异物检测参数组
  auto* foreignWidget = createCollapsibleSection(tr("异物检测"),
                                                 createForeignParams(),
                                                 false);

  // 创建尺寸测量参数组
  auto* dimensionWidget = createCollapsibleSection(tr("尺寸测量"),
                                                   createDimensionParams(),
                                                   false);

  // 添加弹性空间
  m_mainLayout->addStretch();
}

QWidget* ParamPanel::createCollapsibleSection(const QString& title, QWidget* content, bool expanded)
{
  auto* container = new QWidget(this);
  container->setObjectName(QStringLiteral("paramSection"));

  auto* layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 8);  // 增加底部边距
  layout->setSpacing(0);

  // 创建标题按钮
  auto* headerButton = new QToolButton(this);
  headerButton->setText(title);
  headerButton->setObjectName(QStringLiteral("section-header"));
  headerButton->setCheckable(true);
  headerButton->setChecked(expanded);
  headerButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  headerButton->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
  headerButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  headerButton->setMinimumHeight(36);  // 设置标题按钮最小高度
  headerButton->setStyleSheet(QStringLiteral("QToolButton { text-align: left; padding: 10px; font-size: 14px; font-weight: 500; }"));

  layout->addWidget(headerButton);

  // 设置内容区域
  content->setVisible(expanded);
  layout->addWidget(content);

  // 连接信号
  connect(headerButton, &QToolButton::toggled, [this, headerButton, content, title](bool checked) {
    // 如果是手风琴模式且要展开当前section，则折叠其他所有section
    if (m_accordionMode && checked) {
      for (auto it = m_sections.begin(); it != m_sections.end(); ++it) {
        if (it.key() != title && it.value().button && it.value().button->isChecked()) {
          it.value().button->setChecked(false);
        }
      }
    }

    content->setVisible(checked);
    headerButton->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
    m_sections[title].expanded = checked;
  });

  // 保存section信息
  SectionInfo info;
  info.button = headerButton;
  info.content = content;
  info.expanded = expanded;
  m_sections[title] = info;

  m_mainLayout->addWidget(container);

  return container;
}

QWidget* ParamPanel::createScratchParams()
{
  auto* widget = new QWidget(this);
  widget->setMinimumHeight(180);  // 设置最小高度
  auto* layout = new QFormLayout(widget);
  layout->setContentsMargins(20, 12, 12, 12);  // 增加内边距
  layout->setSpacing(16);  // 增加行间距
  layout->setVerticalSpacing(16);
  layout->setHorizontalSpacing(12);

  // 灵敏度滑块
  auto* sensitivityLayout = new QVBoxLayout();
  sensitivityLayout->setSpacing(4);
  auto* sensitivitySlider = new QSlider(Qt::Horizontal);
  sensitivitySlider->setRange(0, 100);
  sensitivitySlider->setValue(75);
  sensitivitySlider->setMinimumHeight(20);
  auto* sensitivityValue = new QLabel(QStringLiteral("75"));
  sensitivityValue->setAlignment(Qt::AlignRight);
  sensitivityValue->setMinimumWidth(30);

  connect(sensitivitySlider, &QSlider::valueChanged, [sensitivityValue](int value) {
    sensitivityValue->setText(QString::number(value));
  });

  auto* sliderLayout = new QHBoxLayout();
  sliderLayout->setSpacing(8);
  sliderLayout->addWidget(sensitivitySlider);
  sliderLayout->addWidget(sensitivityValue);
  sensitivityLayout->addLayout(sliderLayout);

  layout->addRow(tr("灵敏度:"), sensitivityLayout);
  registerControl(kScratchDetector, "sensitivity", ControlType::Slider, sensitivitySlider, sensitivityValue);

  // 最小长度
  auto* minLengthSpin = createIntSpin(kScratchDetector, "minLength", 1, 100, 1, 10, tr("px"));
  layout->addRow(tr("最小长度:"), minLengthSpin);

  // 最大宽度
  auto* maxWidthSpin = createIntSpin(kScratchDetector, "maxWidth", 1, 50, 1, 5, tr("px"));
  layout->addRow(tr("最大宽度:"), maxWidthSpin);

  // 启用检测
  auto* enabledCheck = createCheckBox(kScratchDetector, "enabled", true);
  layout->addRow(tr("启用检测:"), enabledCheck);

  return widget;
}

QWidget* ParamPanel::createCrackParams()
{
  auto* widget = new QWidget(this);
  widget->setMinimumHeight(150);  // 设置最小高度
  auto* layout = new QFormLayout(widget);
  layout->setContentsMargins(20, 12, 12, 12);  // 增加内边距
  layout->setSpacing(16);  // 增加行间距
  layout->setVerticalSpacing(16);
  layout->setHorizontalSpacing(12);

  // 阈值滑块
  auto* thresholdLayout = new QVBoxLayout();
  thresholdLayout->setSpacing(4);
  auto* thresholdSlider = new QSlider(Qt::Horizontal);
  thresholdSlider->setRange(0, 100);
  thresholdSlider->setValue(80);
  thresholdSlider->setMinimumHeight(20);
  auto* thresholdValue = new QLabel(QStringLiteral("80"));
  thresholdValue->setAlignment(Qt::AlignRight);
  thresholdValue->setMinimumWidth(30);

  connect(thresholdSlider, &QSlider::valueChanged, [thresholdValue](int value) {
    thresholdValue->setText(QString::number(value));
  });

  auto* sliderLayout = new QHBoxLayout();
  sliderLayout->setSpacing(8);
  sliderLayout->addWidget(thresholdSlider);
  sliderLayout->addWidget(thresholdValue);
  thresholdLayout->addLayout(sliderLayout);

  layout->addRow(tr("检测阈值:"), thresholdLayout);
  registerControl(kCrackDetector, "threshold", ControlType::Slider, thresholdSlider, thresholdValue);

  // 最小面积
  auto* minAreaSpin = createIntSpin(kCrackDetector, "minArea", 1, 500, 1, 20, tr("px²"));
  layout->addRow(tr("最小面积:"), minAreaSpin);

  // 启用检测
  auto* enabledCheck = createCheckBox(kCrackDetector, "enabled", true);
  layout->addRow(tr("启用检测:"), enabledCheck);

  return widget;
}

QWidget* ParamPanel::createForeignParams()
{
  auto* widget = new QWidget(this);
  widget->setMinimumHeight(120);  // 设置最小高度
  auto* layout = new QFormLayout(widget);
  layout->setContentsMargins(20, 12, 12, 12);  // 增加内边距
  layout->setSpacing(16);  // 增加行间距
  layout->setVerticalSpacing(16);
  layout->setHorizontalSpacing(12);

  // 最小面积
  auto* minAreaSpin = createIntSpin(kForeignDetector, "minArea", 1, 100, 1, 5, tr("px²"));
  layout->addRow(tr("最小面积:"), minAreaSpin);

  // 对比度阈值
  auto* contrastSpin = createDoubleSpin(kForeignDetector, "contrast", 0.1, 1.0, 0.1, 0.3);
  layout->addRow(tr("对比度:"), contrastSpin);

  // 启用检测
  auto* enabledCheck = createCheckBox(kForeignDetector, "enabled", true);
  layout->addRow(tr("启用检测:"), enabledCheck);

  return widget;
}

QWidget* ParamPanel::createDimensionParams()
{
  auto* widget = new QWidget(this);
  widget->setMinimumHeight(120);  // 设置最小高度
  auto* layout = new QFormLayout(widget);
  layout->setContentsMargins(20, 12, 12, 12);  // 增加内边距
  layout->setSpacing(16);  // 增加行间距
  layout->setVerticalSpacing(16);
  layout->setHorizontalSpacing(12);

  // 公差设置
  auto* toleranceSpin = createDoubleSpin(kDimensionDetector, "tolerance", 0.01, 10.0, 0.01, 0.5, tr("mm"));
  layout->addRow(tr("公差:"), toleranceSpin);

  // 标定系数
  auto* calibrationSpin = createDoubleSpin(kDimensionDetector, "calibration", 0.001, 1.0, 0.001, 0.1, tr("mm/px"));
  layout->addRow(tr("标定系数:"), calibrationSpin);

  // 启用检测
  auto* enabledCheck = createCheckBox(kDimensionDetector, "enabled", false);
  layout->addRow(tr("启用检测:"), enabledCheck);

  return widget;
}

QSlider* ParamPanel::createSlider(const QString& detector, const QString& key,
                                  int min, int max, int value, QLabel* valueLabel)
{
  auto* slider = new QSlider(Qt::Horizontal, this);
  slider->setRange(min, max);
  slider->setValue(value);

  if (valueLabel) {
    connect(slider, &QSlider::valueChanged, [valueLabel](int val) {
      valueLabel->setText(QString::number(val));
    });
  }

  connect(slider, &QSlider::valueChanged, [this, detector]() {
    emitParamsChanged(detector);
  });

  return slider;
}

QAbstractSpinBox* ParamPanel::createIntSpin(const QString& detector, const QString& key,
                                            int min, int max, int step, int value,
                                            const QString& suffix)
{
  auto* spin = new QSpinBox(this);
  spin->setRange(min, max);
  spin->setSingleStep(step);
  spin->setValue(value);
  spin->setMinimumHeight(28);  // 设置最小高度
  spin->setStyleSheet(QStringLiteral("QSpinBox { padding: 4px; }"));  // 增加内边距
  if (!suffix.isEmpty()) {
    spin->setSuffix(QStringLiteral(" ") + suffix);
  }

  connect(spin, QOverload<int>::of(&QSpinBox::valueChanged),
          [this, detector]() { emitParamsChanged(detector); });

  registerControl(detector, key, ControlType::Int, spin);
  return spin;
}

QAbstractSpinBox* ParamPanel::createDoubleSpin(const QString& detector, const QString& key,
                                               double min, double max, double step, double value,
                                               const QString& suffix)
{
  auto* spin = new QDoubleSpinBox(this);
  spin->setRange(min, max);
  spin->setSingleStep(step);
  spin->setValue(value);
  spin->setDecimals(3);
  spin->setMinimumHeight(28);  // 设置最小高度
  spin->setStyleSheet(QStringLiteral("QDoubleSpinBox { padding: 4px; }"));  // 增加内边距
  if (!suffix.isEmpty()) {
    spin->setSuffix(QStringLiteral(" ") + suffix);
  }

  connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          [this, detector]() { emitParamsChanged(detector); });

  registerControl(detector, key, ControlType::Double, spin);
  return spin;
}

QCheckBox* ParamPanel::createCheckBox(const QString& detector, const QString& key, bool value)
{
  auto* check = new QCheckBox(this);
  check->setChecked(value);
  check->setMinimumHeight(24);  // 设置最小高度

  connect(check, &QCheckBox::toggled,
          [this, detector]() { emitParamsChanged(detector); });

  registerControl(detector, key, ControlType::Bool, check);
  return check;
}

void ParamPanel::registerControl(const QString& detector, const QString& key,
                                 ControlType type, QWidget* widget, QWidget* linkedWidget)
{
  ControlInfo info;
  info.type = type;
  info.widget = widget;
  info.linkedWidget = linkedWidget;
  m_controls[detector][key] = info;
}

void ParamPanel::emitParamsChanged(const QString& detector)
{
  emit paramsChanged(detector, getDetectorParams(detector));
  
  // 自动保存到配置
  saveToConfig();
}

QVariantMap ParamPanel::getDetectorParams(const QString& detector) const
{
  QVariantMap params;
  const auto detectorControls = m_controls.value(detector);

  for (auto it = detectorControls.cbegin(); it != detectorControls.cend(); ++it) {
    const auto& info = it.value();
    switch (info.type) {
    case ControlType::Bool: {
      auto* check = qobject_cast<QCheckBox*>(info.widget);
      params.insert(it.key(), check ? check->isChecked() : false);
      break;
    }
    case ControlType::Int: {
      auto* spin = qobject_cast<QSpinBox*>(info.widget);
      params.insert(it.key(), spin ? spin->value() : 0);
      break;
    }
    case ControlType::Double: {
      auto* spin = qobject_cast<QDoubleSpinBox*>(info.widget);
      params.insert(it.key(), spin ? spin->value() : 0.0);
      break;
    }
    case ControlType::Slider: {
      auto* slider = qobject_cast<QSlider*>(info.widget);
      params.insert(it.key(), slider ? slider->value() : 0);
      break;
    }
    }
  }

  return params;
}

void ParamPanel::setDetectorParams(const QString& detector, const QVariantMap& params)
{
  const auto detectorControls = m_controls.value(detector);

  for (auto it = params.cbegin(); it != params.cend(); ++it) {
    const auto info = detectorControls.value(it.key());
    if (!info.widget) continue;

    switch (info.type) {
    case ControlType::Bool: {
      auto* check = qobject_cast<QCheckBox*>(info.widget);
      if (check) check->setChecked(it.value().toBool());
      break;
    }
    case ControlType::Int: {
      auto* spin = qobject_cast<QSpinBox*>(info.widget);
      if (spin) spin->setValue(it.value().toInt());
      break;
    }
    case ControlType::Double: {
      auto* spin = qobject_cast<QDoubleSpinBox*>(info.widget);
      if (spin) spin->setValue(it.value().toDouble());
      break;
    }
    case ControlType::Slider: {
      auto* slider = qobject_cast<QSlider*>(info.widget);
      if (slider) slider->setValue(it.value().toInt());
      break;
    }
    }
  }
}

void ParamPanel::loadParams(const QString& configPath)
{
  Q_UNUSED(configPath)
  loadFromConfig();
}

void ParamPanel::saveParams(const QString& configPath) const
{
  Q_UNUSED(configPath)
  const_cast<ParamPanel*>(this)->saveToConfig();
}

void ParamPanel::loadFromConfig()
{
  // 划痕检测参数
  auto scratch = gConfig.scratchConfig();
  QVariantMap scratchParams;
  scratchParams["enabled"] = scratch.enabled;
  scratchParams["sensitivity"] = scratch.sensitivity;
  scratchParams["minLength"] = scratch.minLength;
  scratchParams["maxWidth"] = scratch.maxWidth;
  setDetectorParams(kScratchDetector, scratchParams);

  // 裂纹检测参数
  auto crack = gConfig.crackConfig();
  QVariantMap crackParams;
  crackParams["enabled"] = crack.enabled;
  crackParams["threshold"] = crack.threshold;
  crackParams["minArea"] = crack.minArea;
  setDetectorParams(kCrackDetector, crackParams);

  // 异物检测参数
  auto foreign = gConfig.foreignConfig();
  QVariantMap foreignParams;
  foreignParams["enabled"] = foreign.enabled;
  foreignParams["minArea"] = foreign.minArea;
  foreignParams["contrast"] = foreign.contrast;
  setDetectorParams(kForeignDetector, foreignParams);

  // 尺寸测量参数
  auto dimension = gConfig.dimensionConfig();
  QVariantMap dimensionParams;
  dimensionParams["enabled"] = dimension.enabled;
  dimensionParams["tolerance"] = dimension.tolerance;
  dimensionParams["calibration"] = dimension.calibration;
  setDetectorParams(kDimensionDetector, dimensionParams);
}

void ParamPanel::saveToConfig()
{
  // 划痕检测参数
  auto scratchParams = getDetectorParams(kScratchDetector);
  ScratchDetectorConfig scratch;
  scratch.enabled = scratchParams.value("enabled", true).toBool();
  scratch.sensitivity = scratchParams.value("sensitivity", 75).toInt();
  scratch.minLength = scratchParams.value("minLength", 10).toInt();
  scratch.maxWidth = scratchParams.value("maxWidth", 5).toInt();
  gConfig.setScratchConfig(scratch);

  // 裂纹检测参数
  auto crackParams = getDetectorParams(kCrackDetector);
  CrackDetectorConfig crack;
  crack.enabled = crackParams.value("enabled", true).toBool();
  crack.threshold = crackParams.value("threshold", 80).toInt();
  crack.minArea = crackParams.value("minArea", 20).toInt();
  gConfig.setCrackConfig(crack);

  // 异物检测参数
  auto foreignParams = getDetectorParams(kForeignDetector);
  ForeignDetectorConfig foreign;
  foreign.enabled = foreignParams.value("enabled", true).toBool();
  foreign.minArea = foreignParams.value("minArea", 5).toInt();
  foreign.contrast = foreignParams.value("contrast", 0.3).toDouble();
  gConfig.setForeignConfig(foreign);

  // 尺寸测量参数
  auto dimensionParams = getDetectorParams(kDimensionDetector);
  DimensionDetectorConfig dimension;
  dimension.enabled = dimensionParams.value("enabled", false).toBool();
  dimension.tolerance = dimensionParams.value("tolerance", 0.5).toDouble();
  dimension.calibration = dimensionParams.value("calibration", 0.1).toDouble();
  gConfig.setDimensionConfig(dimension);

  // 保存到文件
  gConfig.save();
}

void ParamPanel::toggleSection(const QString& sectionName)
{
  if (m_sections.contains(sectionName)) {
    auto& info = m_sections[sectionName];
    if (info.button) {
      info.button->setChecked(!info.button->isChecked());
    }
  }
}