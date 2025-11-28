#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QString>
#include <QVector>

class QTableWidget;
class QLabel;
class QComboBox;
class QDateEdit;
class QLineEdit;
class QPushButton;
class QGroupBox;

// 检测记录数据结构
struct DetectionRecord {
  QString recordId;
  QDateTime timestamp;
  QString productId;
  bool isOK;
  QString defectType;
  QString severity;
  QString imagePath;
  QString location;
  double confidence;
  QString size;
  QString operatorName;
};

class StatisticsDialog : public QDialog {
  Q_OBJECT
public:
  explicit StatisticsDialog(QWidget* parent = nullptr);

private slots:
  void onSearchClicked();
  void onExportClicked();
  void onRecordSelected();
  void onPageChanged(int page);
  void updatePagination();

private:
  void setupUI();
  void loadMockData();
  void updateTable();
  void updateDetailPanel(const DetectionRecord* record);
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
  QVector<DetectionRecord> m_allRecords;
  QVector<DetectionRecord> m_filteredRecords;
  int m_currentPage = 1;
  int m_recordsPerPage = 15;
  DetectionRecord* m_selectedRecord = nullptr;
};

#endif // STATISTICSDIALOG_H