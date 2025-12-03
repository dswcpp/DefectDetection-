/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * HistoryView.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：历史视图接口定义
 * 描述：历史记录查看界面，显示检测历史列表、支持筛选、排序、导出
 *
 * 当前版本：1.0
 */

#ifndef HISTORYVIEW_H
#define HISTORYVIEW_H

#include <QWidget>
#include <QVector>
#include "ui_global.h"
#include "data/repositories/DefectRepository.h"

class QTableView;
class QDateTimeEdit;
class QComboBox;
class QPushButton;
class QLabel;
class QSpinBox;
class HistoryTableModel;
class DatabaseManager;
class ImageView;
class SeverityBar;

class UI_LIBRARY HistoryView : public QWidget {
  Q_OBJECT
public:
  explicit HistoryView(QWidget *parent = nullptr);

  void setDatabaseManager(DatabaseManager* dbManager);
  void refresh();

signals:
  void recordSelected(qint64 id);

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
  void onSearchClicked();
  void onResetClicked();
  void onTableSelectionChanged();
  void onPageChanged(int page);
  void onDataLoaded(int count);
  void onDeleteClicked();

private:
  void setupUI();
  void setupConnections();
  void updatePagination();
  void displayRecord(const InspectionRecord& record);
  void drawDefectsOnPreview(const QVector<DefectRecord>& defects);

  // 筛选控件
  QDateTimeEdit* m_startTimeEdit = nullptr;
  QDateTimeEdit* m_endTimeEdit = nullptr;
  QComboBox* m_resultCombo = nullptr;
  QPushButton* m_searchBtn = nullptr;
  QPushButton* m_resetBtn = nullptr;
  QPushButton* m_deleteBtn = nullptr;

  // 表格
  QTableView* m_tableView = nullptr;
  HistoryTableModel* m_model = nullptr;

  // 分页
  QLabel* m_pageInfoLabel = nullptr;
  QPushButton* m_prevPageBtn = nullptr;
  QPushButton* m_nextPageBtn = nullptr;
  QSpinBox* m_pageSizeBox = nullptr;
  int m_currentPage = 0;
  int m_totalCount = 0;

  // 图像预览
  ImageView* m_previewView = nullptr;
  QLabel* m_detailLabel = nullptr;
  QLabel* m_currentInfoLabel = nullptr;

  // 底部信息栏
  QWidget* m_infoTime = nullptr;
  QWidget* m_infoResult = nullptr;
  QWidget* m_infoDefects = nullptr;
  SeverityBar* m_severityBar = nullptr;
  QWidget* m_infoCycleTime = nullptr;

  // 数据
  DatabaseManager* m_dbManager = nullptr;
  QString m_currentImagePath;
  QVector<DefectRecord> m_currentDefects;  // 当前选中记录的缺陷数据
  qint64 m_currentRecordId = 0;            // 当前选中记录的ID
};

#endif // HISTORYVIEW_H
