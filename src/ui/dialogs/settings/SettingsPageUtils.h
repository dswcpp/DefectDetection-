/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * SettingsPageUtils.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：设置页面工具类定义
 * 描述：设置页面公共工具函数和样式定义，提供统一的UI组件创建方法
 *
 * 当前版本：1.0
 */

#ifndef SETTINGSPAGEUTILS_H
#define SETTINGSPAGEUTILS_H

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

namespace SettingsUtils {

inline QGroupBox* createStyledGroupBox(const QString& title, QWidget* parent = nullptr) {
  auto* group = new QGroupBox(title, parent);
  group->setStyleSheet(R"(
    QGroupBox {
      font-weight: normal;
      border: 1px solid #d0d0d0;
      border-radius: 4px;
      margin-top: 12px;
      padding-top: 8px;
      background-color: #ffffff;
    }
    QGroupBox::title {
      subcontrol-origin: margin;
      left: 12px;
      padding: 0 8px;
      background-color: #f5f5f5;
      border: 1px solid #d0d0d0;
      border-radius: 2px;
    }
  )");
  return group;
}

inline QWidget* createSpinBoxWithUnit(int min, int max, int value, const QString& suffix,
                                       QWidget* parent = nullptr) {
  auto* container = new QWidget(parent);
  auto* layout = new QHBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(4);

  auto* spinBox = new QSpinBox(container);
  spinBox->setRange(min, max);
  spinBox->setValue(value);
  spinBox->setMinimumHeight(32);
  spinBox->setMinimumWidth(100);
  layout->addWidget(spinBox);

  if (!suffix.isEmpty()) {
    auto* label = new QLabel(suffix, container);
    label->setStyleSheet("color: #666666;");
    layout->addWidget(label);
  }

  layout->addStretch();
  return container;
}

inline QWidget* createSliderGroup(int min, int max, int value, const QString& suffix,
                                   QWidget* parent = nullptr) {
  auto* container = new QWidget(parent);
  auto* layout = new QHBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(12);

  auto* slider = new QSlider(Qt::Horizontal, container);
  slider->setRange(min, max);
  slider->setValue(value);
  slider->setMinimumWidth(200);
  layout->addWidget(slider, 1);

  auto* spinBox = new QSpinBox(container);
  spinBox->setRange(min, max);
  spinBox->setValue(value);
  spinBox->setMinimumHeight(32);
  spinBox->setMinimumWidth(80);
  layout->addWidget(spinBox);

  if (!suffix.isEmpty()) {
    auto* label = new QLabel(suffix, container);
    label->setStyleSheet("color: #666666;");
    layout->addWidget(label);
  }

  QObject::connect(slider, &QSlider::valueChanged, spinBox, &QSpinBox::setValue);
  QObject::connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), slider, &QSlider::setValue);

  return container;
}

inline QWidget* createCheckableSliderGroup(const QString& label, int min, int max, int value,
                                            bool checked, QWidget* parent = nullptr) {
  auto* container = new QWidget(parent);
  auto* layout = new QHBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(12);

  auto* checkBox = new QCheckBox(label, container);
  checkBox->setChecked(checked);
  checkBox->setMinimumWidth(120);
  layout->addWidget(checkBox);

  auto* slider = new QSlider(Qt::Horizontal, container);
  slider->setRange(min, max);
  slider->setValue(value);
  slider->setEnabled(checked);
  slider->setMinimumWidth(200);
  layout->addWidget(slider, 1);

  auto* spinBox = new QSpinBox(container);
  spinBox->setRange(min, max);
  spinBox->setValue(value);
  spinBox->setEnabled(checked);
  spinBox->setMinimumHeight(32);
  spinBox->setMinimumWidth(80);
  layout->addWidget(spinBox);

  QObject::connect(checkBox, &QCheckBox::toggled, slider, &QSlider::setEnabled);
  QObject::connect(checkBox, &QCheckBox::toggled, spinBox, &QSpinBox::setEnabled);
  QObject::connect(slider, &QSlider::valueChanged, spinBox, &QSpinBox::setValue);
  QObject::connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), slider, &QSlider::setValue);

  return container;
}

}  // namespace SettingsUtils

#endif  // SETTINGSPAGEUTILS_H
