/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * HistoryDialog.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：历史对话框接口定义
 * 描述：历史记录详情对话框，显示单次检测的完整信息和图像
 *
 * 当前版本：1.0
 */

#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include "widgets/FramelessDialog.h"
#include "ui_global.h"

class HistoryView;
class DatabaseManager;

class UI_LIBRARY HistoryDialog : public FramelessDialog {
  Q_OBJECT
public:
  explicit HistoryDialog(DatabaseManager* dbManager, QWidget *parent = nullptr);

private:
  HistoryView* m_historyView = nullptr;
};

#endif // HISTORYDIALOG_H
