/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * StatisticsDialog.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：统计对话框接口定义
 * 描述：统计数据查看对话框，显示指定时间范围的统计信息
 *
 * 当前版本：1.0
 */

#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include "widgets/FramelessDialog.h"
#include <QDateTime>
#include <QString>
#include <QVector>
#include "ui_global.h"
#include "data/repositories/DefectRepository.h"

class QTableWidget;
class QLabel;
class QComboBox;
class QDateEdit;
class QLineEdit;
class QPushButton;
class QGroupBox;
class QMenu;
class QAction;
class DatabaseManager;

class UI_LIBRARY StatisticsDialog : public FramelessDialog {
  Q_OBJECT
public:
  explicit StatisticsDialog(DatabaseManager* dbManager = nullptr, QWidget* parent = nullptr);

signals:
  void requestRetest(qint64 recordId, const QString& imagePath);

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
  void onSearchClicked();
  void onExportClicked();
  void onRecordSelected();
  void onPageChanged(int page);
  void updatePagination();
  
  // 右键菜单操作
  void showContextMenu(const QPoint& pos);
  void onViewDetails();
  void onEditRecord();
  void onDeleteRecord();
  void onRetestRecord();
  void onExportRecord();

private:
  void setupUI();
  void setupContextMenu();
  void loadFromDatabase();
  void updateTable();
  void updateDetailPanel(const InspectionRecord* record);
  void applyFilters();
  InspectionRecord* getSelectedRecord();

  // UI Elements - Filter Section
  QDateEdit* m_startDateEdit;
  QDateEdit* m_endDateEdit;
  QComboBox* m_resultCombo;
  QComboBox* m_defectTypeCombo;
  QComboBox* m_severityCombo;
  QLineEdit* m_keywordEdit;
  QPushButton* m_searchBtn;
  QPushButton* m_exportBtn;

  // UI Elements - Table Section
  QTableWidget* m_recordTable;
  QLabel* m_paginationLabel;
  QPushButton* m_prevPageBtn;
  QPushButton* m_nextPageBtn;
  QLabel* m_pageLabel;

  // UI Elements - Detail Panel
  QGroupBox* m_detailPanel;
  QLabel* m_imagePreview;
  QLabel* m_recordIdLabel;
  QLabel* m_productIdLabel;
  QLabel* m_timestampLabel;
  QLabel* m_resultLabel;
  QLabel* m_defectTypeLabel;
  QLabel* m_severityLabel;
  QLabel* m_locationLabel;
  QLabel* m_confidenceLabel;
  QLabel* m_sizeLabel;
  QLabel* m_operatorLabel;

  // Context Menu
  QMenu* m_contextMenu = nullptr;
  QAction* m_viewAction = nullptr;
  QAction* m_editAction = nullptr;
  QAction* m_deleteAction = nullptr;
  QAction* m_retestAction = nullptr;
  QAction* m_exportAction = nullptr;

  // Data
  DatabaseManager* m_dbManager = nullptr;
  QVector<InspectionRecord> m_allRecords;
  QVector<InspectionRecord> m_filteredRecords;
  int m_currentPage = 1;
  int m_recordsPerPage = 15;
  int m_selectedIndex = -1;
  QString m_currentImagePath;
};

#endif // STATISTICSDIALOG_H