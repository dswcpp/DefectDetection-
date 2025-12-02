#include "ResultCard.h"
#include "SeverityBar.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QStyle>
#include <QVBoxLayout>
#include <QtGlobal>

ResultCard::ResultCard(QWidget* parent) : QFrame{parent}
{
  setupUI();
}

void ResultCard::setupUI()
{
  setObjectName(QStringLiteral("ResultCard"));
  setFrameShape(QFrame::NoFrame);
  setFrameShadow(QFrame::Plain);

  auto* rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(16, 16, 16, 16);
  rootLayout->setSpacing(12);

  m_titleLabel = new QLabel(tr("检测结果"), this);
  m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
  m_titleLabel->setAlignment(Qt::AlignLeft);
  rootLayout->addWidget(m_titleLabel);

  // 状态显示区域 - 更大的OK/NG显示
  auto* statusLayout = new QHBoxLayout();
  statusLayout->setSpacing(8);

  m_statusIcon = new QLabel(this);
  m_statusIcon->setObjectName(QStringLiteral("statusIcon"));
  m_statusIcon->setAlignment(Qt::AlignCenter);
  m_statusIcon->setFixedSize(48, 48);
  statusLayout->addWidget(m_statusIcon);

  m_statusText = new QLabel(tr("待检测"), this);
  m_statusText->setObjectName(QStringLiteral("statusText"));
  m_statusText->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  statusLayout->addWidget(m_statusText);
  statusLayout->addStretch();

  rootLayout->addLayout(statusLayout);

  // 缺陷计数和严重度
  auto* infoLayout = new QHBoxLayout();
  infoLayout->setSpacing(16);

  m_defectCountLabel = new QLabel(tr("缺陷数: 0"), this);
  m_defectCountLabel->setObjectName(QStringLiteral("defectCountLabel"));
  infoLayout->addWidget(m_defectCountLabel);

  auto* severityLabel = new QLabel(tr("严重度:"), this);
  infoLayout->addWidget(severityLabel);

  m_severityBar = new SeverityBar(this);
  m_severityBar->setDisplayMode(SeverityBar::DisplayMode::LevelOnly);
  m_severityBar->setFixedHeight(22);
  m_severityBar->setMinimumWidth(80);
  infoLayout->addWidget(m_severityBar, 1);

  rootLayout->addLayout(infoLayout);

  // 使用 QScrollArea 包装缺陷列表以支持滚动
  m_scrollArea = new QScrollArea(this);
  m_scrollArea->setObjectName(QStringLiteral("ResultsScrollArea"));
  m_scrollArea->setWidgetResizable(true);
  m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_scrollArea->setFrameShape(QFrame::NoFrame);

  m_defectList = new QWidget();
  m_defectList->setObjectName(QStringLiteral("ResultsDefectList"));
  m_defectListLayout = new QVBoxLayout(m_defectList);
  m_defectListLayout->setContentsMargins(0, 0, 0, 0);
  m_defectListLayout->setSpacing(8);

  m_scrollArea->setWidget(m_defectList);
  rootLayout->addWidget(m_scrollArea, 1);

  m_emptyHintLabel = new QLabel(tr("暂无缺陷"), this);
  m_emptyHintLabel->setObjectName(QStringLiteral("ResultsEmptyHint"));
  rootLayout->addWidget(m_emptyHintLabel);

  m_timestampLabel = new QLabel(tr("更新时间: --"), this);
  m_timestampLabel->setObjectName(QStringLiteral("ResultsTimestamp"));
  rootLayout->addWidget(m_timestampLabel, 0, Qt::AlignLeft);

  clear();
}

void ResultCard::setResult(const DetectResult& result)
{
  updateStatus(result.isOK);

  const QString timestamp =
      result.timestamp > 0
          ? QDateTime::fromMSecsSinceEpoch(result.timestamp).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
          : QStringLiteral("--");
  m_timestampLabel->setText(tr("更新时间: %1").arg(timestamp));

  // 更新缺陷计数和严重度
  m_defectCountLabel->setText(tr("缺陷数: %1").arg(result.defects.size()));
  QString severityLevel = calculateSeverityLevel(result.defects);
  m_severityBar->setLevel(severityLevel);

  rebuildDefectList(result.defectType, result.defects);
}

void ResultCard::clear()
{
  updateStatus(true);
  m_statusIcon->setText(QStringLiteral("—"));
  m_statusText->setText(QStringLiteral("--"));
  m_defectCountLabel->setText(tr("缺陷数: 0"));
  m_severityBar->setLevel("None");
  m_timestampLabel->setText(tr("更新时间: --"));
  clearDefectList();
  m_scrollArea->setVisible(false);
  m_emptyHintLabel->setVisible(true);
}

void ResultCard::updateStatus(bool isOk)
{
  setProperty("status", isOk ? "ok" : "ng");

  // 设置图标和文字
  if (isOk) {
    m_statusIcon->setText(QStringLiteral("✔"));
    m_statusIcon->setObjectName(QStringLiteral("okLabel"));
    m_statusText->setText(tr("OK"));
    m_statusText->setObjectName(QStringLiteral("okLabel"));
  } else {
    m_statusIcon->setText(QStringLiteral("✖"));
    m_statusIcon->setObjectName(QStringLiteral("ngLabel"));
    m_statusText->setText(tr("NG"));
    m_statusText->setObjectName(QStringLiteral("ngLabel"));
  }

  // 刷新样式
  m_statusIcon->style()->unpolish(m_statusIcon);
  m_statusIcon->style()->polish(m_statusIcon);
  m_statusText->style()->unpolish(m_statusText);
  m_statusText->style()->polish(m_statusText);
  style()->unpolish(this);
  style()->polish(this);
}

void ResultCard::clearDefectList()
{
  if (!m_defectListLayout) {
    return;
  }
  while (auto* item = m_defectListLayout->takeAt(0)) {
    if (auto* widget = item->widget()) {
      widget->deleteLater();
    }
    delete item;
  }
}

void ResultCard::rebuildDefectList(const QString& typeName, const std::vector<DefectRegion>& defects)
{
  clearDefectList();

  if (defects.empty()) {
    m_scrollArea->setVisible(false);
    m_emptyHintLabel->setVisible(true);
    return;
  }

  m_scrollArea->setVisible(true);
  m_emptyHintLabel->setVisible(false);

  for (int i = 0; i < static_cast<int>(defects.size()); ++i) {
    const auto& defect = defects.at(i);
    auto* entry = new QWidget(m_defectList);
    entry->setObjectName(QStringLiteral("ResultsDefectEntry"));
    auto* entryLayout = new QHBoxLayout(entry);
    entryLayout->setContentsMargins(12, 8, 12, 8);
    entryLayout->setSpacing(4);

    const QString displayName =
        typeName.isEmpty() ? tr("缺陷 #%1").arg(i + 1) : QStringLiteral("%1 #%2").arg(typeName).arg(i + 1);

    auto* nameLabel = new QLabel(displayName, entry);
    nameLabel->setObjectName(QStringLiteral("ResultsDefectName"));
    entryLayout->addWidget(nameLabel, 1);

    const double confidence = qBound(0.0, defect.confidence * 100.0, 100.0);
    auto* confidenceLabel =
        new QLabel(QStringLiteral("%1%").arg(confidence, 0, 'f', 1), entry);
    confidenceLabel->setObjectName(QStringLiteral("ResultsDefectConfidence"));
    entryLayout->addWidget(confidenceLabel, 0, Qt::AlignRight);

    m_defectListLayout->addWidget(entry);
  }
}

QString ResultCard::calculateSeverityLevel(const std::vector<DefectRegion>& defects)
{
  if (defects.empty()) {
    return QStringLiteral("OK");
  }

  // 根据缺陷数量和置信度计算严重度
  double maxConfidence = 0.0;
  for (const auto& defect : defects) {
    maxConfidence = qMax(maxConfidence, defect.confidence);
  }

  int count = static_cast<int>(defects.size());

  // 多个缺陷或高置信度 = Critical
  if (count >= 3 || maxConfidence >= 0.9) {
    return QStringLiteral("Critical");
  }
  // 2个缺陷或中等置信度 = Major
  if (count >= 2 || maxConfidence >= 0.7) {
    return QStringLiteral("Major");
  }
  // 1个低置信度缺陷 = Minor
  return QStringLiteral("Minor");
}
