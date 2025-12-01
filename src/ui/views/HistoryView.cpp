#include "HistoryView.h"
#include "models/HistoryTableModel.h"
#include "widgets/ImageView.h"
#include "data/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QTableView>
#include <QHeaderView>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QFileInfo>
#include <QImage>

HistoryView::HistoryView(QWidget *parent) : QWidget{parent} {
  setupUI();
  setupConnections();
}

void HistoryView::setDatabaseManager(DatabaseManager* dbManager) {
  m_dbManager = dbManager;
  if (m_dbManager && m_dbManager->isOpen()) {
    m_model->setRepository(m_dbManager->defectRepository());
  }
}

void HistoryView::refresh() {
  onSearchClicked();
}

void HistoryView::setupUI() {
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(8, 8, 8, 8);
  mainLayout->setSpacing(8);

  // 筛选区域
  auto* filterGroup = new QGroupBox(tr("筛选条件"), this);
  auto* filterLayout = new QHBoxLayout(filterGroup);

  filterLayout->addWidget(new QLabel(tr("开始时间:"), this));
  m_startTimeEdit = new QDateTimeEdit(this);
  m_startTimeEdit->setCalendarPopup(true);
  m_startTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-7));
  m_startTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
  filterLayout->addWidget(m_startTimeEdit);

  filterLayout->addWidget(new QLabel(tr("结束时间:"), this));
  m_endTimeEdit = new QDateTimeEdit(this);
  m_endTimeEdit->setCalendarPopup(true);
  m_endTimeEdit->setDateTime(QDateTime::currentDateTime());
  m_endTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
  filterLayout->addWidget(m_endTimeEdit);

  filterLayout->addWidget(new QLabel(tr("结果:"), this));
  m_resultCombo = new QComboBox(this);
  m_resultCombo->addItem(tr("全部"), "");
  m_resultCombo->addItem(tr("OK"), "OK");
  m_resultCombo->addItem(tr("NG"), "NG");
  filterLayout->addWidget(m_resultCombo);

  filterLayout->addStretch();

  m_searchBtn = new QPushButton(tr("查询"), this);
  m_searchBtn->setObjectName("primaryButton");
  filterLayout->addWidget(m_searchBtn);

  m_resetBtn = new QPushButton(tr("重置"), this);
  filterLayout->addWidget(m_resetBtn);

  mainLayout->addWidget(filterGroup);

  // 主内容区域（分割器）
  auto* splitter = new QSplitter(Qt::Horizontal, this);

  // 左侧：表格
  auto* tableContainer = new QWidget(this);
  auto* tableLayout = new QVBoxLayout(tableContainer);
  tableLayout->setContentsMargins(0, 0, 0, 0);

  m_tableView = new QTableView(this);
  m_model = new HistoryTableModel(this);
  m_tableView->setModel(m_model);
  m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tableView->setAlternatingRowColors(true);
  m_tableView->setSortingEnabled(false);
  m_tableView->horizontalHeader()->setStretchLastSection(true);
  m_tableView->verticalHeader()->setDefaultSectionSize(28);
  m_tableView->setColumnWidth(0, 60);   // ID
  m_tableView->setColumnWidth(1, 150);  // 时间
  m_tableView->setColumnWidth(2, 60);   // 结果
  m_tableView->setColumnWidth(3, 60);   // 缺陷数
  m_tableView->setColumnWidth(4, 80);   // 严重度
  m_tableView->setColumnWidth(5, 80);   // 耗时
  tableLayout->addWidget(m_tableView);

  // 分页控件
  auto* pageLayout = new QHBoxLayout();
  m_pageInfoLabel = new QLabel(tr("共 0 条记录"), this);
  pageLayout->addWidget(m_pageInfoLabel);
  pageLayout->addStretch();

  pageLayout->addWidget(new QLabel(tr("每页:"), this));
  m_pageSizeBox = new QSpinBox(this);
  m_pageSizeBox->setRange(10, 500);
  m_pageSizeBox->setValue(100);
  m_pageSizeBox->setSingleStep(50);
  pageLayout->addWidget(m_pageSizeBox);

  m_prevPageBtn = new QPushButton(tr("上一页"), this);
  m_prevPageBtn->setEnabled(false);
  pageLayout->addWidget(m_prevPageBtn);

  m_nextPageBtn = new QPushButton(tr("下一页"), this);
  m_nextPageBtn->setEnabled(false);
  pageLayout->addWidget(m_nextPageBtn);

  tableLayout->addLayout(pageLayout);
  splitter->addWidget(tableContainer);

  // 右侧：图像预览
  auto* previewContainer = new QWidget(this);
  auto* previewLayout = new QVBoxLayout(previewContainer);
  previewLayout->setContentsMargins(0, 0, 0, 0);

  auto* previewLabel = new QLabel(tr("图像预览"), this);
  previewLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
  previewLayout->addWidget(previewLabel);

  m_previewView = new ImageView(this);
  m_previewView->setMinimumSize(400, 300);
  previewLayout->addWidget(m_previewView, 1);

  m_detailLabel = new QLabel(this);
  m_detailLabel->setWordWrap(true);
  m_detailLabel->setStyleSheet("padding: 8px; background: #f5f5f5; border-radius: 4px;");
  m_detailLabel->setMinimumHeight(80);
  previewLayout->addWidget(m_detailLabel);

  splitter->addWidget(previewContainer);
  splitter->setStretchFactor(0, 60);
  splitter->setStretchFactor(1, 40);

  mainLayout->addWidget(splitter, 1);
}

void HistoryView::setupConnections() {
  connect(m_searchBtn, &QPushButton::clicked, this, &HistoryView::onSearchClicked);
  connect(m_resetBtn, &QPushButton::clicked, this, &HistoryView::onResetClicked);
  connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &HistoryView::onTableSelectionChanged);
  connect(m_model, &HistoryTableModel::dataLoaded, this, &HistoryView::onDataLoaded);
  connect(m_prevPageBtn, &QPushButton::clicked, this, [this]() {
    if (m_currentPage > 0) {
      onPageChanged(m_currentPage - 1);
    }
  });
  connect(m_nextPageBtn, &QPushButton::clicked, this, [this]() {
    onPageChanged(m_currentPage + 1);
  });
  connect(m_pageSizeBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() {
    m_currentPage = 0;
    onSearchClicked();
  });
}

void HistoryView::onSearchClicked() {
  if (!m_dbManager || !m_dbManager->isOpen()) {
    return;
  }

  InspectionFilter filter;
  filter.startTime = m_startTimeEdit->dateTime();
  filter.endTime = m_endTimeEdit->dateTime();
  filter.result = m_resultCombo->currentData().toString();
  filter.limit = m_pageSizeBox->value();
  filter.offset = m_currentPage * filter.limit;

  m_model->refresh(filter);

  // 获取总数
  m_totalCount = m_dbManager->defectRepository()->countInspections(filter);
  updatePagination();
}

void HistoryView::onResetClicked() {
  m_startTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-7));
  m_endTimeEdit->setDateTime(QDateTime::currentDateTime());
  m_resultCombo->setCurrentIndex(0);
  m_currentPage = 0;
  onSearchClicked();
}

void HistoryView::onTableSelectionChanged() {
  auto indexes = m_tableView->selectionModel()->selectedRows();
  if (indexes.isEmpty()) {
    m_previewView->clear();
    m_detailLabel->clear();
    return;
  }

  int row = indexes.first().row();
  auto record = m_model->recordAt(row);
  displayRecord(record);
  emit recordSelected(record.id);
}

void HistoryView::onPageChanged(int page) {
  m_currentPage = page;
  onSearchClicked();
}

void HistoryView::onDataLoaded(int count) {
  Q_UNUSED(count)
  updatePagination();
}

void HistoryView::updatePagination() {
  int pageSize = m_pageSizeBox->value();
  int totalPages = (m_totalCount + pageSize - 1) / pageSize;

  m_pageInfoLabel->setText(tr("共 %1 条记录，第 %2/%3 页")
                               .arg(m_totalCount)
                               .arg(m_currentPage + 1)
                               .arg(qMax(1, totalPages)));

  m_prevPageBtn->setEnabled(m_currentPage > 0);
  m_nextPageBtn->setEnabled((m_currentPage + 1) * pageSize < m_totalCount);
}

void HistoryView::displayRecord(const InspectionRecord& record) {
  // 显示图像
  if (!record.imagePath.isEmpty() && QFileInfo::exists(record.imagePath)) {
    QImage img(record.imagePath);
    if (!img.isNull()) {
      m_previewView->setImage(img);
      m_previewView->zoomFit();
    } else {
      m_previewView->clear();
    }
  } else {
    m_previewView->clear();
  }

  // 获取并绘制缺陷框
  if (m_dbManager && m_dbManager->isOpen() && record.id > 0) {
    auto defects = m_dbManager->defectRepository()->getDefects(record.id);
    drawDefectsOnPreview(defects);
  }

  // 显示详情
  QString detail = tr(
      "<b>检测时间:</b> %1<br>"
      "<b>结果:</b> <span style='color:%2'>%3</span><br>"
      "<b>缺陷数量:</b> %4<br>"
      "<b>严重度:</b> %5 (分数: %6)<br>"
      "<b>检测耗时:</b> %7 ms<br>"
      "<b>图像路径:</b> %8"
  ).arg(record.inspectTime.toString("yyyy-MM-dd hh:mm:ss"))
   .arg(record.result == "OK" ? "#2e7d32" : "#c62828")
   .arg(record.result)
   .arg(record.defectCount)
   .arg(record.severityLevel)
   .arg(record.maxSeverity, 0, 'f', 2)
   .arg(record.cycleTimeMs)
   .arg(record.imagePath.isEmpty() ? tr("无") : record.imagePath);

  m_detailLabel->setText(detail);
}

void HistoryView::drawDefectsOnPreview(const QVector<DefectRecord>& defects) {
  m_previewView->clearDetectionBoxes();

  if (defects.isEmpty()) {
    return;
  }

  QVector<DetectionBox> boxes;
  for (const auto& defect : defects) {
    DetectionBox box;
    box.rect = cv::Rect(defect.region.x(), defect.region.y(),
                        defect.region.width(), defect.region.height());
    box.label = defect.defectType;
    box.confidence = defect.confidence;

    if (defect.severityLevel == "Minor") {
      box.color = QColor(255, 193, 7);   // yellow
    } else if (defect.severityLevel == "Major") {
      box.color = QColor(255, 152, 0);   // orange
    } else {
      box.color = QColor(244, 67, 54);   // red
    }
    boxes.append(box);
  }

  m_previewView->drawDetectionBoxes(boxes);
}
