#include "StatisticsDialog.h"

#include <QVBoxLayout>
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

StatisticsDialog::StatisticsDialog(QWidget* parent) : QDialog{parent} {
  setModal(true);
  setWindowTitle(tr("检测记录统计"));
  setupUI();
  loadMockData();
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
    tr("记录ID"), tr("时间"), tr("产品ID"),
    tr("结果"), tr("缺陷类型"), tr("严重度")
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

void StatisticsDialog::loadMockData() {
  // 生成模拟数据
  m_allRecords.clear();
  QStringList defectTypes = {tr("划痕"), tr("裂纹"), tr("异物"), tr("尺寸偏差")};
  QStringList severities = {tr("轻微"), tr("中等"), tr("严重")};

  for (int i = 0; i < 100; ++i) {
    DetectionRecord record;
    record.recordId = QString("REC%1").arg(i + 1, 6, 10, QChar('0'));
    record.timestamp = QDateTime::currentDateTime().addDays(-rand() % 30).addSecs(-rand() % 86400);
    record.productId = QString("PRD%1").arg(rand() % 10000, 6, 10, QChar('0'));
    record.isOK = (rand() % 10) > 3;  // 70% OK率

    if (!record.isOK) {
      record.defectType = defectTypes[rand() % defectTypes.size()];
      record.severity = severities[rand() % severities.size()];
      record.location = QString("(%1, %2)").arg(rand() % 1000).arg(rand() % 1000);
      record.confidence = 0.75 + (rand() % 25) / 100.0;
      record.size = QString("%1px").arg(10 + rand() % 50);
    } else {
      record.defectType = "-";
      record.severity = "-";
      record.location = "-";
      record.confidence = 0.0;
      record.size = "-";
    }

    record.operatorName = "admin";
    record.imagePath = "";  // 实际应用中应设置实际图片路径

    m_allRecords.append(record);
  }

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
    auto* idItem = new QTableWidgetItem(record.recordId);
    m_recordTable->setItem(row, 0, idItem);

    // 时间
    auto* timeItem = new QTableWidgetItem(record.timestamp.toString("yyyy/MM/dd hh:mm:ss"));
    m_recordTable->setItem(row, 1, timeItem);

    // 产品ID
    auto* productItem = new QTableWidgetItem(record.productId);
    m_recordTable->setItem(row, 2, productItem);

    // 结果
    auto* resultItem = new QTableWidgetItem(record.isOK ? "OK" : "NG");
    if (record.isOK) {
      resultItem->setBackground(QBrush(QColor("#d1f2eb")));
      resultItem->setForeground(QBrush(QColor("#0f5132")));
    } else {
      resultItem->setBackground(QBrush(QColor("#f8d7da")));
      resultItem->setForeground(QBrush(QColor("#842029")));
    }
    m_recordTable->setItem(row, 3, resultItem);

    // 缺陷类型
    auto* defectItem = new QTableWidgetItem(record.defectType);
    m_recordTable->setItem(row, 4, defectItem);

    // 严重度
    auto* severityItem = new QTableWidgetItem(record.severity);
    if (record.severity == tr("轻微")) {
      severityItem->setBackground(QBrush(QColor("#fff3cd")));
      severityItem->setForeground(QBrush(QColor("#856404")));
    } else if (record.severity == tr("中等")) {
      severityItem->setBackground(QBrush(QColor("#ffeaa7")));
      severityItem->setForeground(QBrush(QColor("#856404")));
    } else if (record.severity == tr("严重")) {
      severityItem->setBackground(QBrush(QColor("#f8d7da")));
      severityItem->setForeground(QBrush(QColor("#842029")));
    }
    m_recordTable->setItem(row, 5, severityItem);
  }

  updatePagination();
}

void StatisticsDialog::updateDetailPanel(const DetectionRecord* record) {
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

  m_recordIdLabel->setText(record->recordId);
  m_productIdLabel->setText(record->productId);
  m_timestampLabel->setText(record->timestamp.toString("yyyy/MM/dd hh:mm:ss"));

  // 结果标签
  if (record->isOK) {
    m_resultLabel->setText("OK");
    m_resultLabel->setStyleSheet("color: #0f5132; background-color: #d1f2eb; padding: 2px 8px; border-radius: 3px;");
  } else {
    m_resultLabel->setText("NG");
    m_resultLabel->setStyleSheet("color: #842029; background-color: #f8d7da; padding: 2px 8px; border-radius: 3px;");
  }

  m_defectTypeLabel->setText(record->defectType);

  // 严重度标签
  if (record->severity == tr("轻微")) {
    m_severityLabel->setStyleSheet("color: #856404; background-color: #fff3cd; padding: 2px 8px; border-radius: 3px;");
  } else if (record->severity == tr("中等")) {
    m_severityLabel->setStyleSheet("color: #856404; background-color: #ffeaa7; padding: 2px 8px; border-radius: 3px;");
  } else if (record->severity == tr("严重")) {
    m_severityLabel->setStyleSheet("color: #842029; background-color: #f8d7da; padding: 2px 8px; border-radius: 3px;");
  } else {
    m_severityLabel->setStyleSheet("font-size: 14px; color: #212529;");
  }
  m_severityLabel->setText(record->severity);

  m_locationLabel->setText(record->location);

  if (record->confidence > 0) {
    m_confidenceLabel->setText(QString("%1%").arg(record->confidence * 100, 0, 'f', 1));
  } else {
    m_confidenceLabel->setText("-");
  }

  m_sizeLabel->setText(record->size);
  m_operatorLabel->setText(record->operatorName);

  // 这里应该加载实际图片
  if (!record->imagePath.isEmpty()) {
    // QPixmap pixmap(record->imagePath);
    // m_imagePreview->setPixmap(pixmap.scaled(m_imagePreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
  } else {
    m_imagePreview->setText(tr("暂无图像"));
  }
}

void StatisticsDialog::onSearchClicked() {
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
      m_selectedRecord = &m_filteredRecords[recordIndex];
      updateDetailPanel(m_selectedRecord);
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
  QString defectFilter = m_defectTypeCombo->currentText();
  QString severityFilter = m_severityCombo->currentText();
  QString keyword = m_keywordEdit->text().toLower();
  QDate startDate = m_startDateEdit->date();
  QDate endDate = m_endDateEdit->date();

  for (const auto& record : m_allRecords) {
    // 日期筛选
    if (record.timestamp.date() < startDate || record.timestamp.date() > endDate) {
      continue;
    }

    // 结果筛选
    if (resultFilter != tr("全部")) {
      if ((resultFilter == "OK" && !record.isOK) ||
          (resultFilter == "NG" && record.isOK)) {
        continue;
      }
    }

    // 缺陷类型筛选
    if (defectFilter != tr("全部") && record.defectType != defectFilter) {
      continue;
    }

    // 严重度筛选
    if (severityFilter != tr("全部") && record.severity != severityFilter) {
      continue;
    }

    // 关键词筛选
    if (!keyword.isEmpty()) {
      if (!record.recordId.toLower().contains(keyword) &&
          !record.productId.toLower().contains(keyword) &&
          !record.operatorName.toLower().contains(keyword)) {
        continue;
      }
    }

    m_filteredRecords.append(record);
  }

  m_currentPage = 1;
}