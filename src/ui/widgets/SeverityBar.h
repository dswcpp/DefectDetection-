/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * SeverityBar.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：严重度条控件接口定义
 * 描述：严重度可视化条形控件，用颜色和长度直观展示缺陷严重程度
 *
 * 当前版本：1.0
 */

#ifndef SEVERITYBAR_H
#define SEVERITYBAR_H

#include <QWidget>
#include "ui_global.h"

class UI_LIBRARY SeverityBar : public QWidget {
  Q_OBJECT
  Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
  Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue)
  Q_PROPERTY(QString level READ level WRITE setLevel NOTIFY levelChanged)

public:
  enum class DisplayMode {
    Gradient,    // 渐变进度条模式
    Segmented,   // 分段模式（Minor/Major/Critical）
    LevelOnly    // 仅显示等级标签
  };
  Q_ENUM(DisplayMode)

  explicit SeverityBar(QWidget *parent = nullptr);

  // 值模式
  double value() const { return m_value; }
  void setValue(double value);
  double maxValue() const { return m_maxValue; }
  void setMaxValue(double max);

  // 等级模式
  QString level() const { return m_level; }
  void setLevel(const QString& level);

  // 显示模式
  DisplayMode displayMode() const { return m_displayMode; }
  void setDisplayMode(DisplayMode mode);

  // 是否显示文本
  bool showText() const { return m_showText; }
  void setShowText(bool show);

  // 尺寸
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

signals:
  void valueChanged(double value);
  void levelChanged(const QString& level);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  QColor colorForValue(double ratio) const;
  QColor colorForLevel(const QString& level) const;
  QString textForDisplay() const;

  double m_value = 0.0;
  double m_maxValue = 1.0;
  QString m_level;
  DisplayMode m_displayMode = DisplayMode::Gradient;
  bool m_showText = true;
};

#endif // SEVERITYBAR_H
