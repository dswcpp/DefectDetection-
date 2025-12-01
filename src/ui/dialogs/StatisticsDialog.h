#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QString>
#include <QVector>
#include "ui_global.h"

class QTableWidget;
class QLabel;
class QComboBox;
class QDateEdit;
class QLineEdit;
class QPushButton;
class QGroupBox;
class DatabaseManager;
struct InspectionRecord;

class UI_LIBRARY StatisticsDialog : public QDialog {
  Q_OBJECT
public:
  explicit StatisticsDialog(DatabaseManager* dbManager = nullptr, QWidget* parent = nullptr);

private slots:
  void onSearchClicked();
  void onExportClicked();
  void onRecordSelected();
  void onPageChanged(int page);
  void updatePagination();

private:
  void setupUI();
  void loadFromDatabase();
  void updateTable();
  void updateDetailPanel(const InspectionRecord* record);
  void applyFilters();

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

  // Data
  DatabaseManager* m_dbManager = nullptr;
  QVector<InspectionRecord> m_allRecords;
  QVector<InspectionRecord> m_filteredRecords;
  int m_currentPage = 1;
  int m_recordsPerPage = 15;
  int m_selectedIndex = -1;
};

#endif // STATISTICSDIALOG_H