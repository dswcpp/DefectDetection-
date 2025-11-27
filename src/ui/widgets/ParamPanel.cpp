#include "ParamPanel.h"
#include <QAbstractSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFormLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSpinBox>
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>

namespace {
constexpr auto kScratchDetector = "scratch";
constexpr auto kCrackDetector = "crack";
constexpr auto kForeignDetector = "foreign";
constexpr auto kDimensionDetector = "dimension";
}

ParamPanel::ParamPanel(QWidget* parent) : QWidget{parent}
{
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  m_tabWidget = new QTabWidget(this);
  setObjectName(QStringLiteral("ParametersPanelWidget"));
  m_tabWidget->setObjectName(QStringLiteral("ParamTabs"));
  layout->addWidget(m_tabWidget);

  if (auto* tabBar = m_tabWidget->tabBar()) {
    tabBar->setObjectName(QStringLiteral("ParamTabBar"));
  }

  m_tabWidget->addTab(createScratchPage(), tr("划痕检测"));
  m_tabWidget->addTab(createCrackPage(), tr("裂纹检测"));
  m_tabWidget->addTab(createForeignPage(), tr("异物检测"));
  m_tabWidget->addTab(createDimensionPage(), tr("尺寸测量"));
}

void ParamPanel::loadParams(const QString& configPath)
{
  QFile file(configPath);
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }

  const auto doc = QJsonDocument::fromJson(file.readAll());
  if (!doc.isObject()) {
    return;
  }

  const auto obj = doc.object();
  for (auto it = obj.begin(); it != obj.end(); ++it) {
    if (it.value().isObject()) {
      setDetectorParams(it.key(), it.value().toObject().toVariantMap());
    }
  }
}

void ParamPanel::saveParams(const QString& configPath) const
{
  QFile file(configPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return;
  }

  QJsonObject root;
  for (auto it = m_controls.cbegin(); it != m_controls.cend(); ++it) {
    root.insert(it.key(), QJsonObject::fromVariantMap(getDetectorParams(it.key())));
  }

  QJsonDocument doc(root);
  file.write(doc.toJson(QJsonDocument::Indented));
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
    }
  }
  return params;
}

void ParamPanel::setDetectorParams(const QString& detector, const QVariantMap& params)
{
  auto controls = m_controls.value(detector);
  for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
    const auto controlInfo = controls.value(it.key());
    switch (controlInfo.type) {
    case ControlType::Bool: {
      if (auto* check = qobject_cast<QCheckBox*>(controlInfo.widget)) {
        check->setChecked(it.value().toBool());
      }
      break;
    }
    case ControlType::Int: {
      if (auto* spin = qobject_cast<QSpinBox*>(controlInfo.widget)) {
        spin->setValue(it.value().toInt());
      }
      break;
    }
    case ControlType::Double: {
      if (auto* spin = qobject_cast<QDoubleSpinBox*>(controlInfo.widget)) {
        spin->setValue(it.value().toDouble());
      }
      break;
    }
    }
  }
  emitParamsChanged(detector);
}

QWidget* ParamPanel::createScratchPage()
{
  auto* page = createPage();
  auto* form = qobject_cast<QFormLayout*>(page->layout());

  form->addRow(tr("启用检测"), createCheckBox(kScratchDetector, "enabled", true));
  form->addRow(tr("Canny低阈值"),
               createDoubleSpin(kScratchDetector, "canny_low", 0, 255, 1, 50));
  form->addRow(tr("Canny高阈值"),
               createDoubleSpin(kScratchDetector, "canny_high", 0, 255, 1, 150));
  form->addRow(tr("最小长度"),
               createIntSpin(kScratchDetector, "min_length", 5, 200, 1, 20, tr(" px")));
  form->addRow(tr("最大间隙"),
               createIntSpin(kScratchDetector, "max_gap", 0, 50, 1, 10, tr(" px")));

  return page;
}

QWidget* ParamPanel::createCrackPage()
{
  auto* page = createPage();
  auto* form = qobject_cast<QFormLayout*>(page->layout());

  form->addRow(tr("启用检测"), createCheckBox(kCrackDetector, "enabled", true));
  form->addRow(tr("阈值"),
               createIntSpin(kCrackDetector, "threshold", 0, 255, 1, 70));
  form->addRow(tr("最小宽度"),
               createIntSpin(kCrackDetector, "min_width", 1, 20, 1, 2, tr(" px")));
  form->addRow(tr("最大宽度"),
               createIntSpin(kCrackDetector, "max_width", 5, 50, 1, 12, tr(" px")));

  return page;
}

QWidget* ParamPanel::createForeignPage()
{
  auto* page = createPage();
  auto* form = qobject_cast<QFormLayout*>(page->layout());

  form->addRow(tr("启用检测"), createCheckBox(kForeignDetector, "enabled", true));
  form->addRow(tr("阈值"),
               createIntSpin(kForeignDetector, "threshold", 0, 255, 1, 40));
  form->addRow(tr("最小面积"),
               createIntSpin(kForeignDetector, "min_area", 10, 5000, 10, 80, tr(" px²")));
  form->addRow(tr("最大面积"),
               createIntSpin(kForeignDetector, "max_area", 100, 20000, 10, 600, tr(" px²")));

  return page;
}

QWidget* ParamPanel::createDimensionPage()
{
  auto* page = createPage();
  auto* form = qobject_cast<QFormLayout*>(page->layout());

  form->addRow(tr("启用检测"), createCheckBox(kDimensionDetector, "enabled", true));
  form->addRow(tr("目标宽度"),
               createDoubleSpin(kDimensionDetector, "target_width", 0, 1000, 0.1, 50.0, tr(" mm")));
  form->addRow(tr("目标高度"),
               createDoubleSpin(kDimensionDetector, "target_height", 0, 1000, 0.1, 50.0, tr(" mm")));
  form->addRow(tr("尺寸公差"),
               createDoubleSpin(kDimensionDetector, "tolerance", 0, 10, 0.01, 0.10, tr(" mm")));

  return page;
}

QWidget* ParamPanel::createPage() const
{
  auto* page = new QWidget();
  page->setObjectName(QStringLiteral("ParamSectionPage"));
  auto* form = new QFormLayout(page);
  form->setLabelAlignment(Qt::AlignRight);
  form->setContentsMargins(16, 16, 16, 16);
  form->setSpacing(12);
  page->setLayout(form);
  return page;
}

QAbstractSpinBox* ParamPanel::createDoubleSpin(const QString& detector, const QString& key,
                                               double min, double max, double step, double value,
                                               const QString& suffix)
{
  auto* spin = new QDoubleSpinBox(this);
  spin->setRange(min, max);
  spin->setSingleStep(step);
  spin->setValue(value);
  spin->setDecimals(2);
  if (!suffix.isEmpty()) {
    spin->setSuffix(QStringLiteral(" ") + suffix);
  }
  registerControl(detector, key, ControlType::Double, spin);
  QObject::connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                   this, [this, detector]() { emitParamsChanged(detector); });
  return spin;
}

QAbstractSpinBox* ParamPanel::createIntSpin(const QString& detector, const QString& key,
                                            int min, int max, int step, int value,
                                            const QString& suffix)
{
  auto* spin = new QSpinBox(this);
  spin->setRange(min, max);
  spin->setSingleStep(step);
  spin->setValue(value);
  if (!suffix.isEmpty()) {
    spin->setSuffix(QStringLiteral(" ") + suffix);
  }
  registerControl(detector, key, ControlType::Int, spin);
  QObject::connect(spin, QOverload<int>::of(&QSpinBox::valueChanged),
                   this, [this, detector]() { emitParamsChanged(detector); });
  return spin;
}

QCheckBox* ParamPanel::createCheckBox(const QString& detector, const QString& key, bool value)
{
  auto* check = new QCheckBox(this);
  check->setChecked(value);
  registerControl(detector, key, ControlType::Bool, check);
  QObject::connect(check, &QCheckBox::toggled,
                   this, [this, detector]() { emitParamsChanged(detector); });
  return check;
}

void ParamPanel::registerControl(const QString& detector, const QString& key,
                                 ControlType type, QWidget* widget)
{
  m_controls[detector][key] = ControlInfo{type, widget};
}

void ParamPanel::emitParamsChanged(const QString& detector)
{
  emit paramsChanged(detector, getDetectorParams(detector));
}
