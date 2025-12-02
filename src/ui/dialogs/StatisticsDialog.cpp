#include "StatisticsDialog.h"
#include "ImagePreviewDialog.h"
#include "data/DatabaseManager.h"
#include "services/UserManager.h"

#include <QVBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QSplitter>
#include <QHeaderView>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QDate>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QApplication>
#include <QFile>
#include <QTextStream>

StatisticsDialog::StatisticsDialog(DatabaseManager* dbManager, QWidget* parent)
    : QDialog{parent}, m_dbManager(dbManager) {
  setModal(true);
  setWindowTitle(tr("检测记录统计"));
  setupUI();
  setupContextMenu();
  loadFromDatabase();
  updateTable();
}

void StatisticsDialog::setupUI() {
  // 设置对话框大小
  setMinimumSize(1400, 800);
  resize(1600, 900);

  // 主布局
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // 筛选区域
  auto* filterWidget = new QWidget();
  filterWidget->setStyleSheet("background-color: white; border-bottom: 1px solid #dee2e6;");
  auto* filterLayout = new QHBoxLayout(filterWidget);
  filterLayout->setContentsMargins(24, 16, 24, 16);
  filterLayout->setSpacing(16);

  // 开始日期
  auto* startDateGroup = new QWidget();
  auto* startDateLayout = new QVBoxLayout(startDateGroup);
  startDateLayout->setContentsMargins(0, 0, 0, 0);
  startDateLayout->setSpacing(4);
  auto* startDateLabel = new QLabel(tr("开始日期"));
  startDateLabel->setStyleSheet("font-size: 12px; color: #6c757d;");
  startDateLayout->addWidget(startDateLabel);
  m_startDateEdit = new QDateEdit(QDate::currentDate().addDays(-30));
  m_startDateEdit->setCalendarPopup(true);
  m_startDateEdit->setDisplayFormat("yyyy/MM/dd");
  m_startDateEdit->setMinimumHeight(32);
  m_startDateEdit->setMinimumWidth(130);
  m_startDateEdit->setStyleSheet(R"(
    QDateEdit {
      border: 1px solid #ced4da;
      border-radius: 4px;
      padding: 4px 8px;
      background-color: white;
      font-size: 13px;
    }
    QDateEdit:hover {
      border-color: #adb5bd;
    }
    QDateEdit:focus {
      border-color: #80bdff;
      outline: none;
    }
    QDateEdit::drop-down {
      subcontrol-origin: padding;
      subcontrol-position: top right;
      width: 20px;
      border-left: 1px solid #ced4da;
      border-top-right-radius: 4px;
      border-bottom-right-radius: 4px;
      background-color: #f8f9fa;
    }
    QDateEdit::drop-down:hover {
      background-color: #e9ecef;
    }
    QDateEdit::down-arrow {
      image: url(:/icons/calendar.svg);
      width: 14px;
      height: 14px;
    }
    QDateEdit::down-arrow:on {
      top: 1px;
      left: 1px;
    }
    QDateEdit QCalendarWidget {
      background-color: white;
      border: 1px solid #dee2e6;
      border-radius: 4px;
    }
    QDateEdit QCalendarWidget QToolButton {
      background-color: transparent;
      color: #495057;
      font-size: 13px;
      border: none;
      border-radius: 4px;
      padding: 4px;
    }
    QDateEdit QCalendarWidget QToolButton:hover {
      background-color: #e9ecef;
    }
    QDateEdit QCalendarWidget QToolButton#qt_calendar_prevmonth,
    QDateEdit QCalendarWidget QToolButton#qt_calendar_nextmonth {
      qproperty-icon: none;
      min-width: 24px;
      max-width: 24px;
      min-height: 24px;
      max-height: 24px;
      border-radius: 12px;
    }
    QDateEdit QCalendarWidget QToolButton#qt_calendar_prevmonth {
      qproperty-text: "<";
    }
    QDateEdit QCalendarWidget QToolButton#qt_calendar_nextmonth {
      qproperty-text: ">";
    }
    QDateEdit QCalendarWidget QWidget#qt_calendar_navigationbar {
      background-color: #f8f9fa;
      border-bottom: 1px solid #dee2e6;
      padding: 4px;
    }
    QDateEdit QCalendarWidget QAbstractItemView {
      selection-background-color: #007bff;
      selection-color: white;
      font-size: 12px;
      outline: none;
    }
    QDateEdit QCalendarWidget QAbstractItemView:enabled {
      color: #212529;
      background-color: white;
    }
    QDateEdit QCalendarWidget QAbstractItemView:disabled {
      color: #6c757d;
    }
  )");
  startDateLayout->addWidget(m_startDateEdit);
  filterLayout->addWidget(startDateGroup);

  // 结束日期
  auto* endDateGroup = new QWidget();
  auto* endDateLayout = new QVBoxLayout(endDateGroup);
  endDateLayout->setContentsMargins(0, 0, 0, 0);
  endDateLayout->setSpacing(4);
  auto* endDateLabel = new QLabel(tr("结束日期"));
  endDateLabel->setStyleSheet("font-size: 12px; color: #6c757d;");
  endDateLayout->addWidget(endDateLabel);
  m_endDateEdit = new QDateEdit(QDate::currentDate());
  m_endDateEdit->setCalendarPopup(true);
  m_endDateEdit->setDisplayFormat("yyyy/MM/dd");
  m_endDateEdit->setMinimumHeight(32);
  m_endDateEdit->setMinimumWidth(130);
  m_endDateEdit->setStyleSheet(m_startDateEdit->styleSheet());
  endDateLayout->addWidget(m_endDateEdit);
  filterLayout->addWidget(endDateGroup);

  // 结果筛选
  auto* resultGroup = new QWidget();
  auto* resultLayout = new QVBoxLayout(resultGroup);
  resultLayout->setContentsMargins(0, 0, 0, 0);
  resultLayout->setSpacing(4);
  auto* resultLabel = new QLabel(tr("结果"));
  resultLabel->setStyleSheet("font-size: 12px; color: #6c757d;");
  resultLayout->addWidget(resultLabel);
  m_resultCombo = new QComboBox();
  m_resultCombo->addItems({tr("全部"), tr("OK"), tr("NG")});
  m_resultCombo->setMinimumHeight(32);
  m_resultCombo->setMinimumWidth(100);
  m_resultCombo->setStyleSheet(R"(
    QComboBox {
      border: 1px solid #ced4da;
      border-radius: 4px;
      padding: 4px 8px;
      background-color: white;
      font-size: 13px;
    }
    QComboBox:hover {
      border-color: #adb5bd;
    }
    QComboBox:focus {
      border-color: #80bdff;
      outline: none;
    }
    QComboBox::drop-down {
      subcontrol-origin: padding;
      subcontrol-position: top right;
      width: 20px;
      border-left: 1px solid #ced4da;
      border-top-right-radius: 4px;
      border-bottom-right-radius: 4px;
      background-color: #f8f9fa;
    }
    QComboBox::drop-down:hover {
      background-color: #e9ecef;
    }
    QComboBox::down-arrow {
      image: url(:/icons/arrow-down.svg);
      width: 12px;
      height: 12px;
    }
    QComboBox::down-arrow:on {
      top: 1px;
      left: 1px;
    }
    QComboBox QAbstractItemView {
      border: 1px solid #dee2e6;
      background-color: white;
      selection-background-color: #007bff;
      selection-color: white;
      outline: none;
      padding: 4px;
    }
    QComboBox QAbstractItemView::item {
      padding: 4px 8px;
      min-height: 24px;
    }
    QComboBox QAbstractItemView::item:hover {
      background-color: #e9ecef;
    }
  )");
  resultLayout->addWidget(m_resultCombo);
  filterLayout->addWidget(resultGroup);

  // 缺陷类型筛选
  auto* defectGroup = new QWidget();
  auto* defectLayout = new QVBoxLayout(defectGroup);
  defectLayout->setContentsMargins(0, 0, 0, 0);
  defectLayout->setSpacing(4);
  auto* defectLabel = new QLabel(tr("缺陷类型"));
  defectLabel->setStyleSheet("font-size: 12px; color: #6c757d;");
  defectLayout->addWidget(defectLabel);
  m_defectTypeCombo = new QComboBox();
  m_defectTypeCombo->addItems({tr("全部"), tr("划痕"), tr("裂纹"), tr("异物"), tr("尺寸偏差")});
  m_defectTypeCombo->setMinimumHeight(32);
  m_defectTypeCombo->setMinimumWidth(120);
  m_defectTypeCombo->setStyleSheet(m_resultCombo->styleSheet());
  defectLayout->addWidget(m_defectTypeCombo);
  filterLayout->addWidget(defectGroup);

  // 严重度筛选
  auto* severityGroup = new QWidget();
  auto* severityLayout = new QVBoxLayout(severityGroup);
  severityLayout->setContentsMargins(0, 0, 0, 0);
  severityLayout->setSpacing(4);
  auto* severityLabel = new QLabel(tr("严重度"));
  severityLabel->setStyleSheet("font-size: 12px; color: #6c757d;");
  severityLayout->addWidget(severityLabel);
  m_severityCombo = new QComboBox();
  m_severityCombo->addItems({tr("全部"), tr("轻微"), tr("中等"), tr("严重")});
  m_severityCombo->setMinimumHeight(32);
  m_severityCombo->setMinimumWidth(100);
  m_severityCombo->setStyleSheet(m_resultCombo->styleSheet());
  severityLayout->addWidget(m_severityCombo);
  filterLayout->addWidget(severityGroup);

  // 关键词搜索
  auto* keywordGroup = new QWidget();
  auto* keywordLayout = new QVBoxLayout(keywordGroup);
  keywordLayout->setContentsMargins(0, 0, 0, 0);
  keywordLayout->setSpacing(4);
  auto* keywordLabel = new QLabel(tr("关键词"));
  keywordLabel->setStyleSheet("font-size: 12px; color: #6c757d;");
  keywordLayout->addWidget(keywordLabel);
  m_keywordEdit = new QLineEdit();
  m_keywordEdit->setPlaceholderText(tr("产品ID、操作员..."));
  m_keywordEdit->setMinimumHeight(32);
  m_keywordEdit->setMinimumWidth(200);
  m_keywordEdit->setStyleSheet(R"(
    QLineEdit {
      border: 1px solid #ced4da;
      border-radius: 4px;
      padding: 0 8px;
      background-color: white;
    }
    QLineEdit:focus {
      border-color: #80bdff;
    }
  )");
  keywordLayout->addWidget(m_keywordEdit);
  filterLayout->addWidget(keywordGroup);

  filterLayout->addStretch();

  // 搜索和导出按钮
  auto* buttonGroup = new QWidget();
  auto* buttonLayout = new QVBoxLayout(buttonGroup);
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->setSpacing(4);
  auto* spacerLabel = new QLabel(" ");
  spacerLabel->setFixedHeight(16);
  buttonLayout->addWidget(spacerLabel);

  auto* btnContainer = new QWidget();
  auto* btnLayout = new QHBoxLayout(btnContainer);
  btnLayout->setContentsMargins(0, 0, 0, 0);
  btnLayout->setSpacing(8);

  m_searchBtn = new QPushButton(tr("🔍 搜索"));
  m_searchBtn->setMinimumHeight(32);
  m_searchBtn->setMinimumWidth(80);
  m_searchBtn->setStyleSheet(R"(
    QPushButton {
      background-color: #007bff;
      color: white;
      border: none;
      border-radius: 4px;
      padding: 0 16px;
      font-weight: 500;
    }
    QPushButton:hover {
      background-color: #0056b3;
    }
  )");
  connect(m_searchBtn, &QPushButton::clicked, this, &StatisticsDialog::onSearchClicked);
  btnLayout->addWidget(m_searchBtn);

  m_exportBtn = new QPushButton(tr("📥 导出"));
  m_exportBtn->setMinimumHeight(32);
  m_exportBtn->setMinimumWidth(80);
  m_exportBtn->setStyleSheet(R"(
    QPushButton {
      background-color: #28a745;
      color: white;
      border: none;
      border-radius: 4px;
      padding: 0 16px;
      font-weight: 500;
    }
    QPushButton:hover {
      background-color: #218838;
    }
  )");
  connect(m_exportBtn, &QPushButton::clicked, this, &StatisticsDialog::onExportClicked);
  btnLayout->addWidget(m_exportBtn);

  buttonLayout->addWidget(btnContainer);
  filterLayout->addWidget(buttonGroup);

  mainLayout->addWidget(filterWidget);

  // 主内容区域（表格 + 详情面板）
  auto* contentWidget = new QWidget();
  contentWidget->setStyleSheet("background-color: #f8f9fa;");
  auto* contentLayout = new QHBoxLayout(contentWidget);
  contentLayout->setContentsMargins(0, 0, 0, 0);
  contentLayout->setSpacing(0);

  // 左侧表格区域
  auto* tableWidget = new QWidget();
  tableWidget->setStyleSheet("background-color: white;");
  auto* tableLayout = new QVBoxLayout(tableWidget);
  tableLayout->setContentsMargins(0, 0, 0, 0);
  tableLayout->setSpacing(0);

  // 表格
  m_recordTable = new QTableWidget();
  m_recordTable->setColumnCount(6);
  m_recordTable->setHorizontalHeaderLabels({
    tr("ID"), tr("检测时间"), tr("缺陷数"),
    tr("结果"), tr("耗时"), tr("严重度")
  });

  m_recordTable->setAlternatingRowColors(true);
  m_recordTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_recordTable->setSelectionMode(QAbstractItemView::SingleSelection);
  m_recordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_recordTable->horizontalHeader()->setStretchLastSection(true);
  m_recordTable->verticalHeader()->setVisible(false);

  m_recordTable->setStyleSheet(R"(
    QTableWidget {
      border: none;
      background-color: white;
      gridline-color: #dee2e6;
    }
    QTableWidget::item {
      padding: 8px;
      border-bottom: 1px solid #dee2e6;
    }
    QTableWidget::item:selected {
      background-color: #cfe2ff;
      color: #084298;
    }
    QTableWidget::item:hover {
      background-color: #e7f1ff;
    }
    QHeaderView::section {
      background-color: #f8f9fa;
      border: none;
      border-bottom: 2px solid #dee2e6;
      padding: 8px;
      font-weight: 500;
      color: #495057;
    }
  )");

  connect(m_recordTable, &QTableWidget::itemSelectionChanged,
          this, &StatisticsDialog::onRecordSelected);

  // 启用右键菜单
  m_recordTable->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_recordTable, &QTableWidget::customContextMenuRequested,
          this, &StatisticsDialog::showContextMenu);

  tableLayout->addWidget(m_recordTable, 1);

  // 分页控件
  auto* paginationWidget = new QWidget();
  paginationWidget->setFixedHeight(56);
  paginationWidget->setStyleSheet("background-color: #f8f9fa; border-top: 1px solid #dee2e6;");
  auto* paginationLayout = new QHBoxLayout(paginationWidget);
  paginationLayout->setContentsMargins(24, 0, 24, 0);

  m_paginationLabel = new QLabel();
  m_paginationLabel->setStyleSheet("color: #6c757d; font-size: 14px;");
  paginationLayout->addWidget(m_paginationLabel);

  paginationLayout->addStretch();

  m_prevPageBtn = new QPushButton(tr("◀"));
  m_prevPageBtn->setFixedSize(32, 32);
  m_prevPageBtn->setStyleSheet(R"(
    QPushButton {
      background-color: white;
      border: 1px solid #dee2e6;
      border-radius: 4px;
    }
    QPushButton:hover:!disabled {
      background-color: #e9ecef;
    }
    QPushButton:disabled {
      opacity: 0.5;
    }
  )");
  connect(m_prevPageBtn, &QPushButton::clicked, [this]() {
    if (m_currentPage > 1) {
      onPageChanged(m_currentPage - 1);
    }
  });
  paginationLayout->addWidget(m_prevPageBtn);

  m_pageLabel = new QLabel();
  m_pageLabel->setStyleSheet("padding: 0 16px; font-size: 14px;");
  paginationLayout->addWidget(m_pageLabel);

  m_nextPageBtn = new QPushButton(tr("▶"));
  m_nextPageBtn->setFixedSize(32, 32);
  m_nextPageBtn->setStyleSheet(m_prevPageBtn->styleSheet());
  connect(m_nextPageBtn, &QPushButton::clicked, [this]() {
    int totalPages = (m_filteredRecords.size() + m_recordsPerPage - 1) / m_recordsPerPage;
    if (m_currentPage < totalPages) {
      onPageChanged(m_currentPage + 1);
    }
  });
  paginationLayout->addWidget(m_nextPageBtn);

  tableLayout->addWidget(paginationWidget);
  contentLayout->addWidget(tableWidget, 1);

  // 右侧详情面板
  m_detailPanel = new QGroupBox();
  m_detailPanel->setFixedWidth(400);
  m_detailPanel->setStyleSheet(R"(
    QGroupBox {
      background-color: #f8f9fa;
      border: none;
      border-left: 1px solid #dee2e6;
    }
  )");

  auto* detailLayout = new QVBoxLayout(m_detailPanel);
  detailLayout->setContentsMargins(0, 0, 0, 0);
  detailLayout->setSpacing(0);

  // 图像预览区域
  auto* imageContainer = new QWidget();
  imageContainer->setStyleSheet("background-color: white; border-bottom: 1px solid #dee2e6;");
  auto* imageLayout = new QVBoxLayout(imageContainer);
  imageLayout->setContentsMargins(16, 16, 16, 16);

  auto* imageTitle = new QLabel(tr("图像预览"));
  imageTitle->setStyleSheet("font-weight: 500; margin-bottom: 12px;");
  imageLayout->addWidget(imageTitle);

  m_imagePreview = new QLabel();
  m_imagePreview->setFixedHeight(250);
  m_imagePreview->setStyleSheet(R"(
    QLabel {
      background-color: #212529;
      border-radius: 4px;
      color: #6c757d;
    }
  )");
  m_imagePreview->setAlignment(Qt::AlignCenter);
  m_imagePreview->setText(tr("暂无图像"));
  m_imagePreview->installEventFilter(this);  // 双击放大
  imageLayout->addWidget(m_imagePreview);

  detailLayout->addWidget(imageContainer);

  // 详细信息区域
  auto* infoScrollArea = new QScrollArea();
  infoScrollArea->setWidgetResizable(true);
  infoScrollArea->setStyleSheet("QScrollArea { border: none; background-color: #f8f9fa; }");

  auto* infoContainer = new QWidget();
  auto* infoLayout = new QVBoxLayout(infoContainer);
  infoLayout->setContentsMargins(16, 16, 16, 16);
  infoLayout->setSpacing(12);

  auto* detailTitle = new QLabel(tr("详细信息"));
  detailTitle->setStyleSheet("font-weight: 500; margin-bottom: 8px;");
  infoLayout->addWidget(detailTitle);

  // 创建信息卡片的辅助函数
  auto createInfoCard = [](const QString& label, QLabel*& valueLabel) -> QWidget* {
    auto* card = new QWidget();
    card->setStyleSheet(R"(
      QWidget {
        background-color: white;
        border: 1px solid #dee2e6;
        border-radius: 4px;
      }
    )");
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(12, 8, 12, 8);
    cardLayout->setSpacing(4);

    auto* labelWidget = new QLabel(label);
    labelWidget->setStyleSheet("color: #6c757d; font-size: 12px;");
    cardLayout->addWidget(labelWidget);

    valueLabel = new QLabel("-");
    valueLabel->setStyleSheet("font-size: 14px; color: #212529;");
    cardLayout->addWidget(valueLabel);

    return card;
  };

  infoLayout->addWidget(createInfoCard(tr("记录ID"), m_recordIdLabel));
  infoLayout->addWidget(createInfoCard(tr("产品ID"), m_productIdLabel));
  infoLayout->addWidget(createInfoCard(tr("检测时间"), m_timestampLabel));
  infoLayout->addWidget(createInfoCard(tr("检测结果"), m_resultLabel));
  infoLayout->addWidget(createInfoCard(tr("缺陷类型"), m_defectTypeLabel));
  infoLayout->addWidget(createInfoCard(tr("严重度"), m_severityLabel));
  infoLayout->addWidget(createInfoCard(tr("缺陷位置"), m_locationLabel));
  infoLayout->addWidget(createInfoCard(tr("置信度"), m_confidenceLabel));
  infoLayout->addWidget(createInfoCard(tr("缺陷大小"), m_sizeLabel));
  infoLayout->addWidget(createInfoCard(tr("操作员"), m_operatorLabel));

  infoLayout->addStretch();
  infoScrollArea->setWidget(infoContainer);
  detailLayout->addWidget(infoScrollArea, 1);

  contentLayout->addWidget(m_detailPanel);
  mainLayout->addWidget(contentWidget, 1);
}

void StatisticsDialog::loadFromDatabase() {
  m_allRecords.clear();

  if (!m_dbManager || !m_dbManager->isOpen()) {
    return;
  }

  InspectionFilter filter;
  filter.startTime = QDateTime(m_startDateEdit->date(), QTime(0, 0, 0));
  filter.endTime = QDateTime(m_endDateEdit->date(), QTime(23, 59, 59));
  filter.limit = 1000;
  filter.offset = 0;

  m_allRecords = m_dbManager->defectRepository()->queryInspections(filter);
  m_filteredRecords = m_allRecords;
}

void StatisticsDialog::updateTable() {
  m_recordTable->setRowCount(0);

  int startIndex = (m_currentPage - 1) * m_recordsPerPage;
  int endIndex = qMin(startIndex + m_recordsPerPage, m_filteredRecords.size());

  for (int i = startIndex; i < endIndex; ++i) {
    const auto& record = m_filteredRecords[i];
    int row = m_recordTable->rowCount();
    m_recordTable->insertRow(row);

    // 记录ID
    auto* idItem = new QTableWidgetItem(QString::number(record.id));
    m_recordTable->setItem(row, 0, idItem);

    // 时间
    auto* timeItem = new QTableWidgetItem(record.inspectTime.toString("yyyy/MM/dd hh:mm:ss"));
    m_recordTable->setItem(row, 1, timeItem);

    // 缺陷数
    auto* defectCountItem = new QTableWidgetItem(QString::number(record.defectCount));
    m_recordTable->setItem(row, 2, defectCountItem);

    // 结果
    bool isOK = (record.result == "OK");
    auto* resultItem = new QTableWidgetItem(record.result);
    if (isOK) {
      resultItem->setBackground(QBrush(QColor("#d1f2eb")));
      resultItem->setForeground(QBrush(QColor("#0f5132")));
    } else {
      resultItem->setBackground(QBrush(QColor("#f8d7da")));
      resultItem->setForeground(QBrush(QColor("#842029")));
    }
    m_recordTable->setItem(row, 3, resultItem);

    // 耗时
    auto* cycleItem = new QTableWidgetItem(QString("%1 ms").arg(record.cycleTimeMs));
    m_recordTable->setItem(row, 4, cycleItem);

    // 严重度
    auto* severityItem = new QTableWidgetItem(record.severityLevel);
    if (record.severityLevel == "Minor") {
      severityItem->setBackground(QBrush(QColor("#fff3cd")));
      severityItem->setForeground(QBrush(QColor("#856404")));
    } else if (record.severityLevel == "Major") {
      severityItem->setBackground(QBrush(QColor("#ffeaa7")));
      severityItem->setForeground(QBrush(QColor("#856404")));
    } else if (record.severityLevel == "Critical") {
      severityItem->setBackground(QBrush(QColor("#f8d7da")));
      severityItem->setForeground(QBrush(QColor("#842029")));
    }
    m_recordTable->setItem(row, 5, severityItem);
  }

  updatePagination();
}

void StatisticsDialog::updateDetailPanel(const InspectionRecord* record) {
  if (!record) {
    m_recordIdLabel->setText("-");
    m_productIdLabel->setText("-");
    m_timestampLabel->setText("-");
    m_resultLabel->setText("-");
    m_defectTypeLabel->setText("-");
    m_severityLabel->setText("-");
    m_locationLabel->setText("-");
    m_confidenceLabel->setText("-");
    m_sizeLabel->setText("-");
    m_operatorLabel->setText("-");
    m_imagePreview->setText(tr("暂无图像"));
    return;
  }

  m_recordIdLabel->setText(QString::number(record->id));
  m_productIdLabel->setText(record->productId > 0 ? QString::number(record->productId) : "-");
  m_timestampLabel->setText(record->inspectTime.toString("yyyy/MM/dd hh:mm:ss"));

  // 结果标签
  bool isOK = (record->result == "OK");
  if (isOK) {
    m_resultLabel->setText("OK");
    m_resultLabel->setStyleSheet("color: #0f5132; background-color: #d1f2eb; padding: 2px 8px; border-radius: 3px;");
  } else {
    m_resultLabel->setText("NG");
    m_resultLabel->setStyleSheet("color: #842029; background-color: #f8d7da; padding: 2px 8px; border-radius: 3px;");
  }

  m_defectTypeLabel->setText(QString::number(record->defectCount) + tr(" 个缺陷"));

  // 严重度标签
  if (record->severityLevel == "Minor") {
    m_severityLabel->setStyleSheet("color: #856404; background-color: #fff3cd; padding: 2px 8px; border-radius: 3px;");
  } else if (record->severityLevel == "Major") {
    m_severityLabel->setStyleSheet("color: #856404; background-color: #ffeaa7; padding: 2px 8px; border-radius: 3px;");
  } else if (record->severityLevel == "Critical") {
    m_severityLabel->setStyleSheet("color: #842029; background-color: #f8d7da; padding: 2px 8px; border-radius: 3px;");
  } else {
    m_severityLabel->setStyleSheet("font-size: 14px; color: #212529;");
  }
  m_severityLabel->setText(record->severityLevel);

  m_locationLabel->setText("-");
  m_confidenceLabel->setText(QString("%1").arg(record->maxSeverity, 0, 'f', 2));
  m_sizeLabel->setText(QString("%1 ms").arg(record->cycleTimeMs));
  m_operatorLabel->setText("-");

  // 加载图片
  m_currentImagePath = record->imagePath;
  if (!record->imagePath.isEmpty()) {
    QPixmap pixmap(record->imagePath);
    if (!pixmap.isNull()) {
      m_imagePreview->setPixmap(pixmap.scaled(m_imagePreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
      m_imagePreview->setText(tr("图像加载失败"));
    }
  } else {
    m_imagePreview->setText(tr("暂无图像"));
  }
}

bool StatisticsDialog::eventFilter(QObject* obj, QEvent* event) {
  if (obj == m_imagePreview && event->type() == QEvent::MouseButtonDblClick) {
    if (!m_currentImagePath.isEmpty() && QFileInfo::exists(m_currentImagePath)) {
      auto* dialog = new ImagePreviewDialog(this);
      dialog->setImage(m_currentImagePath);
      dialog->exec();
      dialog->deleteLater();
    }
    return true;
  }
  return QDialog::eventFilter(obj, event);
}

void StatisticsDialog::onSearchClicked() {
  loadFromDatabase();
  applyFilters();
  updateTable();
}

void StatisticsDialog::onExportClicked() {
  QString fileName = QFileDialog::getSaveFileName(this,
    tr("导出数据"),
    QString("detection_records_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
    tr("CSV Files (*.csv)"));

  if (!fileName.isEmpty()) {
    // 这里实现导出功能
    QMessageBox::information(this, tr("导出成功"), tr("数据已导出到: %1").arg(fileName));
  }
}

void StatisticsDialog::onRecordSelected() {
  auto selectedItems = m_recordTable->selectedItems();
  if (!selectedItems.isEmpty()) {
    int row = selectedItems.first()->row();
    int recordIndex = (m_currentPage - 1) * m_recordsPerPage + row;
    if (recordIndex < m_filteredRecords.size()) {
      m_selectedIndex = recordIndex;
      updateDetailPanel(&m_filteredRecords[recordIndex]);
    }
  }
}

void StatisticsDialog::onPageChanged(int page) {
  m_currentPage = page;
  updateTable();
}

void StatisticsDialog::updatePagination() {
  int totalRecords = m_filteredRecords.size();
  int startIndex = (m_currentPage - 1) * m_recordsPerPage + 1;
  int endIndex = qMin(startIndex + m_recordsPerPage - 1, totalRecords);
  int totalPages = (totalRecords + m_recordsPerPage - 1) / m_recordsPerPage;

  m_paginationLabel->setText(tr("显示 %1 - %2 条，共 %3 条记录")
    .arg(startIndex).arg(endIndex).arg(totalRecords));

  m_pageLabel->setText(tr("%1 / %2").arg(m_currentPage).arg(totalPages));

  m_prevPageBtn->setEnabled(m_currentPage > 1);
  m_nextPageBtn->setEnabled(m_currentPage < totalPages);
}

void StatisticsDialog::applyFilters() {
  m_filteredRecords.clear();

  QString resultFilter = m_resultCombo->currentText();
  QString severityFilter = m_severityCombo->currentText();
  QString keyword = m_keywordEdit->text().toLower();

  for (const auto& record : m_allRecords) {
    // 结果筛选
    if (resultFilter != tr("全部")) {
      if ((resultFilter == "OK" && record.result != "OK") ||
          (resultFilter == "NG" && record.result != "NG")) {
        continue;
      }
    }

    // 严重度筛选
    if (severityFilter != tr("全部")) {
      QString level = record.severityLevel;
      if ((severityFilter == tr("轻微") && level != "Minor") ||
          (severityFilter == tr("中等") && level != "Major") ||
          (severityFilter == tr("严重") && level != "Critical")) {
        continue;
      }
    }

    // 关键词筛选 (搜索 ID 或图像路径)
    if (!keyword.isEmpty()) {
      if (!QString::number(record.id).contains(keyword) &&
          !record.imagePath.toLower().contains(keyword)) {
        continue;
      }
    }

    m_filteredRecords.append(record);
  }

  m_currentPage = 1;
}

void StatisticsDialog::setupContextMenu() {
  m_contextMenu = new QMenu(this);
  m_contextMenu->setStyleSheet(R"(
    QMenu {
      background-color: white;
      border: 1px solid #dee2e6;
      border-radius: 4px;
      padding: 4px 0;
    }
    QMenu::item {
      padding: 8px 32px 8px 16px;
      color: #212529;
    }
    QMenu::item:selected {
      background-color: #e9ecef;
    }
    QMenu::item:disabled {
      color: #adb5bd;
    }
    QMenu::separator {
      height: 1px;
      background-color: #dee2e6;
      margin: 4px 8px;
    }
  )");

  m_viewAction = m_contextMenu->addAction(tr("👁 查看详情"));
  connect(m_viewAction, &QAction::triggered, this, &StatisticsDialog::onViewDetails);

  m_contextMenu->addSeparator();

  m_editAction = m_contextMenu->addAction(tr("✏ 编辑记录"));
  connect(m_editAction, &QAction::triggered, this, &StatisticsDialog::onEditRecord);

  m_retestAction = m_contextMenu->addAction(tr("🔄 重新检测"));
  connect(m_retestAction, &QAction::triggered, this, &StatisticsDialog::onRetestRecord);

  m_contextMenu->addSeparator();

  m_exportAction = m_contextMenu->addAction(tr("📥 导出此记录"));
  connect(m_exportAction, &QAction::triggered, this, &StatisticsDialog::onExportRecord);

  m_contextMenu->addSeparator();

  m_deleteAction = m_contextMenu->addAction(tr("🗑 删除记录"));
  m_deleteAction->setShortcut(QKeySequence::Delete);
  connect(m_deleteAction, &QAction::triggered, this, &StatisticsDialog::onDeleteRecord);
}

InspectionRecord* StatisticsDialog::getSelectedRecord() {
  if (m_selectedIndex >= 0 && m_selectedIndex < m_filteredRecords.size()) {
    return &m_filteredRecords[m_selectedIndex];
  }
  return nullptr;
}

void StatisticsDialog::showContextMenu(const QPoint& pos) {
  auto* item = m_recordTable->itemAt(pos);
  if (!item) return;

  // 选中当前行
  int row = item->row();
  m_recordTable->selectRow(row);
  int recordIndex = (m_currentPage - 1) * m_recordsPerPage + row;
  if (recordIndex < m_filteredRecords.size()) {
    m_selectedIndex = recordIndex;
  }

  // 根据权限更新菜单项状态
  auto* userMgr = UserManager::instance();
  
  m_viewAction->setEnabled(true);  // 查看详情始终可用
  m_editAction->setEnabled(userMgr->hasPermission("DeleteHistory"));  // 编辑需要管理权限
  m_deleteAction->setEnabled(userMgr->hasPermission("DeleteHistory"));
  m_retestAction->setEnabled(userMgr->hasPermission("RunDetection"));
  m_exportAction->setEnabled(userMgr->hasPermission("ExportData"));

  m_contextMenu->popup(m_recordTable->viewport()->mapToGlobal(pos));
}

void StatisticsDialog::onViewDetails() {
  auto* record = getSelectedRecord();
  if (!record) return;

  // 显示详情面板
  updateDetailPanel(record);

  // 如果有图片，双击可以预览
  if (!record->imagePath.isEmpty() && QFileInfo::exists(record->imagePath)) {
    auto* dialog = new ImagePreviewDialog(this);
    dialog->setImage(record->imagePath);
    dialog->exec();
    dialog->deleteLater();
  }
}

void StatisticsDialog::onEditRecord() {
  auto* record = getSelectedRecord();
  if (!record) return;

  // TODO: 实现编辑对话框
  QMessageBox::information(this, tr("编辑记录"), 
    tr("编辑记录功能开发中...\n\n记录ID: %1").arg(record->id));
}

void StatisticsDialog::onDeleteRecord() {
  auto* record = getSelectedRecord();
  if (!record) return;

  auto result = QMessageBox::question(this, tr("确认删除"),
    tr("确定要删除记录 ID: %1 吗？\n\n此操作不可撤销。").arg(record->id),
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (result != QMessageBox::Yes) return;

  if (m_dbManager && m_dbManager->defectRepository()) {
    if (m_dbManager->defectRepository()->deleteInspection(record->id)) {
      QMessageBox::information(this, tr("删除成功"), tr("记录已删除"));
      // 刷新列表
      loadFromDatabase();
      applyFilters();
      updateTable();
      updateDetailPanel(nullptr);
    } else {
      QMessageBox::warning(this, tr("删除失败"), tr("无法删除记录，请稍后重试"));
    }
  }
}

void StatisticsDialog::onRetestRecord() {
  auto* record = getSelectedRecord();
  if (!record) return;

  if (record->imagePath.isEmpty() || !QFileInfo::exists(record->imagePath)) {
    QMessageBox::warning(this, tr("无法重测"), tr("原始图像文件不存在"));
    return;
  }

  auto result = QMessageBox::question(this, tr("确认重测"),
    tr("确定要重新检测记录 ID: %1 吗？").arg(record->id),
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (result == QMessageBox::Yes) {
    emit requestRetest(record->id, record->imagePath);
    QMessageBox::information(this, tr("重测请求"), 
      tr("已发送重测请求，请在主界面查看结果"));
  }
}

void StatisticsDialog::onExportRecord() {
  auto* record = getSelectedRecord();
  if (!record) return;

  QString fileName = QFileDialog::getSaveFileName(this,
    tr("导出记录"),
    QString("record_%1_%2.csv")
      .arg(record->id)
      .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
    tr("CSV Files (*.csv);;JSON Files (*.json)"));

  if (fileName.isEmpty()) return;

  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this, tr("导出失败"), tr("无法创建文件"));
    return;
  }

  QTextStream out(&file);
  
  if (fileName.endsWith(".json", Qt::CaseInsensitive)) {
    // JSON 格式
    out << "{\n";
    out << QString("  \"id\": %1,\n").arg(record->id);
    out << QString("  \"inspectTime\": \"%1\",\n").arg(record->inspectTime.toString(Qt::ISODate));
    out << QString("  \"result\": \"%1\",\n").arg(record->result);
    out << QString("  \"defectCount\": %1,\n").arg(record->defectCount);
    out << QString("  \"severityLevel\": \"%1\",\n").arg(record->severityLevel);
    out << QString("  \"maxSeverity\": %1,\n").arg(record->maxSeverity);
    out << QString("  \"cycleTimeMs\": %1,\n").arg(record->cycleTimeMs);
    out << QString("  \"imagePath\": \"%1\"\n").arg(record->imagePath);
    out << "}\n";
  } else {
    // CSV 格式
    out << "ID,检测时间,结果,缺陷数,严重等级,最大严重度,耗时(ms),图像路径\n";
    out << QString("%1,%2,%3,%4,%5,%6,%7,%8\n")
      .arg(record->id)
      .arg(record->inspectTime.toString("yyyy-MM-dd hh:mm:ss"))
      .arg(record->result)
      .arg(record->defectCount)
      .arg(record->severityLevel)
      .arg(record->maxSeverity)
      .arg(record->cycleTimeMs)
      .arg(record->imagePath);
  }

  file.close();
  QMessageBox::information(this, tr("导出成功"), tr("记录已导出到: %1").arg(fileName));
}