/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ParamPanel.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：参数面板控件接口定义
 * 描述：检测参数调节面板，显示和修改各检测器的参数，
 *       支持实时预览参数效果
 *
 * 当前版本：1.0
 */

#ifndef PARAMPANEL_H
#define PARAMPANEL_H

#include <QVariantMap>
#include <QWidget>
#include <QHash>

class QToolButton;
class QVBoxLayout;
class QSlider;
class QAbstractSpinBox;
class QCheckBox;
class QLabel;
class ParamPanel : public QWidget {
  Q_OBJECT
public:
  explicit ParamPanel(QWidget* parent = nullptr);

  void loadParams(const QString& configPath = QString());
  void saveParams(const QString& configPath = QString()) const;
  QVariantMap getDetectorParams(const QString& detector) const;
  void setDetectorParams(const QString& detector, const QVariantMap& params);

  // 从/到 ConfigManager 加载/保存
  void loadFromConfig();
  void saveToConfig();

  // 设置手风琴模式（true = 一次只能打开一个，false = 可以同时打开多个）
  void setAccordionMode(bool enabled) { m_accordionMode = enabled; }

signals:
  void paramsChanged(const QString& detector, const QVariantMap& params);

private:
  enum class ControlType { Bool, Int, Double, Slider };
  struct ControlInfo {
    ControlType type;
    QWidget* widget = nullptr;
    QWidget* linkedWidget = nullptr; // 用于滑块的关联数值显示
  };

  struct SectionInfo {
    QToolButton* button = nullptr;
    QWidget* content = nullptr;
    bool expanded = false;
  };

  void setupUI();
  QWidget* createCollapsibleSection(const QString& title, QWidget* content, bool expanded = false);
  void toggleSection(const QString& sectionName);

  QWidget* createScratchParams();
  QWidget* createCrackParams();
  QWidget* createForeignParams();
  QWidget* createDimensionParams();

  QSlider* createSlider(const QString& detector, const QString& key,
                       int min, int max, int value, QLabel* valueLabel = nullptr);
  QAbstractSpinBox* createDoubleSpin(const QString& detector, const QString& key,
                                     double min, double max, double step, double value,
                                     const QString& suffix = QString());
  QAbstractSpinBox* createIntSpin(const QString& detector, const QString& key,
                                  int min, int max, int step, int value,
                                  const QString& suffix = QString());
  QCheckBox* createCheckBox(const QString& detector, const QString& key, bool value);

  void registerControl(const QString& detector, const QString& key,
                       ControlType type, QWidget* widget, QWidget* linkedWidget = nullptr);
  void emitParamsChanged(const QString& detector);

  QVBoxLayout* m_mainLayout;
  QHash<QString, SectionInfo> m_sections;
  QHash<QString, QHash<QString, ControlInfo>> m_controls;
  bool m_accordionMode = true;  // 默认为手风琴模式
};

#endif // PARAMPANEL_H
