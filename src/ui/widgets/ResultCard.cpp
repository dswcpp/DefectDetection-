#include "ResultCard.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>
#include <QtGlobal>

ResultCard::ResultCard(QWidget* parent) : QFrame{parent}
{
  setupUI();
}

void ResultCard::setupUI()
{
  setObjectName(QStringLiteral("ResultsPanel"));
  setFrameShape(QFrame::NoFrame);
  setFrameShadow(QFrame::Plain);

  auto* rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(16, 16, 16, 16);
  rootLayout->setSpacing(12);

  m_titleLabel = new QLabel(tr("检测结果"), this);
  m_titleLabel->setObjectName(QStringLiteral("ResultsPanelTitle"));
  rootLayout->addWidget(m_titleLabel);

  auto* statusLayout = new QHBoxLayout();
  statusLayout->setSpacing(12);

  m_statusIcon = new QLabel(QStringLiteral("—"), this);
  m_statusIcon->setObjectName(QStringLiteral("ResultsStatusIcon"));
  m_statusIcon->setAlignment(Qt::AlignCenter);
  m_statusIcon->setFixedSize(36, 36);
  statusLayout->addWidget(m_statusIcon);

  m_statusText = new QLabel(tr("--"), this);
  m_statusText->setObjectName(QStringLiteral("ResultsStatusText"));
  statusLayout->addWidget(m_statusText);
  statusLayout->addStretch();

  rootLayout->addLayout(statusLayout);

  m_defectList = new QWidget(this);
  m_defectList->setObjectName(QStringLiteral("ResultsDefectList"));
  m_defectListLayout = new QVBoxLayout(m_defectList);
  m_defectListLayout->setContentsMargins(0, 0, 0, 0);
  m_defectListLayout->setSpacing(8);
  rootLayout->addWidget(m_defectList);

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

  rebuildDefectList(result.defectType, result.defects);
}

void ResultCard::clear()
{
  updateStatus(true);
  m_statusIcon->setText(QStringLiteral("—"));
  m_statusText->setText(QStringLiteral("--"));
  m_timestampLabel->setText(tr("更新时间: --"));
  clearDefectList();
  m_defectList->setVisible(false);
  m_emptyHintLabel->setVisible(true);
}

void ResultCard::updateStatus(bool isOk)
{
  setProperty("status", isOk ? "ok" : "ng");
  m_statusIcon->setText(isOk ? QStringLiteral("✔") : QStringLiteral("✖"));
  m_statusText->setText(isOk ? tr("OK") : tr("NG"));
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
    m_defectList->setVisible(false);
    m_emptyHintLabel->setVisible(true);
    return;
  }

  m_defectList->setVisible(true);
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
