/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * CalibrationDialog.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：标定对话框接口定义
 * 描述：相机标定向导对话框，引导用户完成棋盘格拍摄、标定计算等步骤
 *
 * 当前版本：1.0
 */

#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include <QWidget>

class CalibrationDialog : public QWidget {
  Q_OBJECT
public:
  explicit CalibrationDialog(QWidget *parent = nullptr);

signals:
};

#endif // CALIBRATIONDIALOG_H
