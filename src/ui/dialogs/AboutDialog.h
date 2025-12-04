/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * AboutDialog.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：关于对话框接口定义
 * 描述：显示软件版本、版权信息、开源许可等内容的对话框
 *
 * 当前版本：1.0
 */

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "widgets/FramelessDialog.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
QT_END_NAMESPACE

class AboutDialog : public FramelessDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog() = default;

private:
    void setupUI();
};

#endif // ABOUTDIALOG_H
