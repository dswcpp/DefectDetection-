/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ResultCard.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：结果卡片控件接口定义
 * 描述：检测结果显示卡片，展示OK/NG状态、缺陷类型、
 *       严重等级、置信度等信息
 *
 * 当前版本：1.0
 */

#ifndef RESULTCARD_H
#define RESULTCARD_H

#include <QFrame>
#include <QVector>
#include "Types.h"

class QLabel;
class QVBoxLayout;
class QWidget;
class QScrollArea;
class SeverityBar;

class ResultCard : public QFrame {
  Q_OBJECT
public:
  explicit ResultCard(QWidget* parent = nullptr);

  void setResult(const DetectResult& result);
  void clear();

private:
  void setupUI();
  void updateStatus(bool isOk);
  void clearDefectList();
  void rebuildDefectList(const QString& typeName, const std::vector<DefectRegion>& defects);
  QString calculateSeverityLevel(const std::vector<DefectRegion>& defects);

  QLabel* m_titleLabel = nullptr;
  QLabel* m_statusIcon = nullptr;
  QLabel* m_statusText = nullptr;
  QLabel* m_defectCountLabel = nullptr;
  SeverityBar* m_severityBar = nullptr;
  QScrollArea* m_scrollArea = nullptr;
  QWidget* m_defectList = nullptr;
  QVBoxLayout* m_defectListLayout = nullptr;
  QLabel* m_emptyHintLabel = nullptr;
  QLabel* m_timestampLabel = nullptr;
};

#endif // RESULTCARD_H
