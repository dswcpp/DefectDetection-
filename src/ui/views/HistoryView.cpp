#include "HistoryView.h"
#include "models/HistoryTableModel.h"
#include "widgets/ImageView.h"
#include "widgets/SeverityBar.h"
#include "dialogs/ImagePreviewDialog.h"
#include "services/UserManager.h"
#include "data/DatabaseManager.h"
#include "common/Logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
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
#include <QFrame>
#include <QToolButton>
#include <QMouseEvent>
#include <QMessageBox>

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
  auto* mainLayout = new QHBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // ==================== 左侧：图像回放区域（主角）====================
  auto* previewPanel = new QWidget();
  previewPanel->setStyleSheet("background-color: #1a1a2e;");
  auto* previewLayout = new QVBoxLayout(previewPanel);
  previewLayout->setContentsMargins(0, 0, 0, 0);
  previewLayout->setSpacing(0);

  // 图像显示区域
  m_previewView = new ImageView(this);
  m_previewView->setStyleSheet("background-color: #16213e;");
  previewLayout->addWidget(m_previewView, 1);

  // 底部控制栏
  auto* controlBar = new QWidget();
  controlBar->setFixedHeight(64);
  controlBar->setStyleSheet("background-color: #0f0f23; border-top: 1px solid #2a2a4a;");
  auto* controlLayout = new QHBoxLayout(controlBar);
  controlLayout->setContentsMargins(16, 0, 16, 0);

  // 导航按钮
  auto* prevBtn = new QToolButton();
  prevBtn->setText(tr("< 上一条"));
  prevBtn->setStyleSheet(R"(
    QToolButton {
      background-color: #2a2a4a;
      color: #e0e0e0;
      border: none;
      border-radius: 4px;
      padding: 8px 16px;
      font-size: 13px;
    }
    QToolButton:hover { background-color: #3a3a5a; }
    QToolButton:disabled { color: #666; }
  )");
  connect(prevBtn, &QToolButton::clicked, this, [this]() {
    auto sel = m_tableView->selectionModel()->selectedRows();
    if (!sel.isEmpty() && sel.first().row() > 0) {
      m_tableView->selectRow(sel.first().row() - 1);
    }
  });
  controlLayout->addWidget(prevBtn);

  controlLayout->addStretch();

  // 当前记录信息
  m_currentInfoLabel = new QLabel(tr("请选择记录"));
  m_currentInfoLabel->setStyleSheet("color: #a0a0a0; font-size: 14px;");
  m_currentInfoLabel->setAlignment(Qt::AlignCenter);
  controlLayout->addWidget(m_currentInfoLabel);

  controlLayout->addStretch();

  auto* nextBtn = new QToolButton();
  nextBtn->setText(tr("下一条 >"));
  nextBtn->setStyleSheet(prevBtn->styleSheet());
  connect(nextBtn, &QToolButton::clicked, this, [this]() {
    auto sel = m_tableView->selectionModel()->selectedRows();
    if (!sel.isEmpty() && sel.first().row() < m_model->rowCount() - 1) {
      m_tableView->selectRow(sel.first().row() + 1);
    }
  });
  controlLayout->addWidget(nextBtn);

  previewLayout->addWidget(controlBar);

  // 详情信息条
  auto* infoBar = new QWidget();
  infoBar->setFixedHeight(80);
  infoBar->setStyleSheet("background-color: #1a1a2e; border-top: 1px solid #2a2a4a;");
  auto* infoLayout = new QHBoxLayout(infoBar);
  infoLayout->setContentsMargins(24, 12, 24, 12);
  infoLayout->setSpacing(32);

  // 详情标签组
  auto createInfoItem = [](const QString& label) -> QWidget* {
    auto* widget = new QWidget();
    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    auto* titleLabel = new QLabel(label);
    titleLabel->setStyleSheet("color: #6c757d; font-size: 11px;");
    auto* valueLabel = new QLabel("-");
    valueLabel->setObjectName("value");
    valueLabel->setStyleSheet("color: #e0e0e0; font-size: 15px; font-weight: 500;");
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    return widget;
  };

  m_infoTime = createInfoItem(tr("检测时间"));
  m_infoResult = createInfoItem(tr("结果"));
  m_infoDefects = createInfoItem(tr("缺陷数"));
  m_infoCycleTime = createInfoItem(tr("耗时"));

  infoLayout->addWidget(m_infoTime);
  infoLayout->addWidget(m_infoResult);
  infoLayout->addWidget(m_infoDefects);

  // 严重度使用 SeverityBar
  auto* severityContainer = new QWidget();
  auto* severityLayout = new QVBoxLayout(severityContainer);
  severityLayout->setContentsMargins(0, 0, 0, 0);
  severityLayout->setSpacing(2);
  auto* severityTitle = new QLabel(tr("严重度"));
  severityTitle->setStyleSheet("color: #6c757d; font-size: 11px;");
  severityLayout->addWidget(severityTitle);
  m_severityBar = new SeverityBar(this);
  m_severityBar->setDisplayMode(SeverityBar::DisplayMode::LevelOnly);
  m_severityBar->setFixedHeight(20);
  m_severityBar->setMinimumWidth(80);
  severityLayout->addWidget(m_severityBar);
  infoLayout->addWidget(severityContainer);

  infoLayout->addWidget(m_infoCycleTime);
  infoLayout->addStretch();

  previewLayout->addWidget(infoBar);

  mainLayout->addWidget(previewPanel, 7);  // 70% 宽度

  // ==================== 右侧：记录列表 ====================
  auto* listPanel = new QWidget();
  listPanel->setStyleSheet("background-color: #2C2C2E; border-left: 1px solid #48484A;");
  auto* listLayout = new QVBoxLayout(listPanel);
  listLayout->setContentsMargins(0, 0, 0, 0);
  listLayout->setSpacing(0);

  // 筛选区域（精简版）
  auto* filterWidget = new QWidget();
  filterWidget->setStyleSheet("background-color: #3C3C3E; border-bottom: 1px solid #48484A;");
  auto* filterLayout = new QVBoxLayout(filterWidget);
  filterLayout->setContentsMargins(12, 12, 12, 12);
  filterLayout->setSpacing(8);

  auto* filterTitle = new QLabel(tr("筛选条件"));
  filterTitle->setStyleSheet("font-weight: 500; font-size: 13px; color: #E0E0E0;");
  filterLayout->addWidget(filterTitle);

  // 时间范围（单行）
  auto* timeRow = new QHBoxLayout();
  m_startTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-7));
  m_startTimeEdit->setCalendarPopup(true);
  m_startTimeEdit->setDisplayFormat("MM/dd hh:mm");
  m_startTimeEdit->setFixedHeight(28);
  m_startTimeEdit->setStyleSheet("QDateTimeEdit { border: 1px solid #555; border-radius: 3px; padding: 2px 6px; font-size: 12px; background: #2C2C2E; color: #E0E0E0; }");
  timeRow->addWidget(m_startTimeEdit);
  
  auto* timeSeparator = new QLabel("-");
  timeSeparator->setStyleSheet("color: #E0E0E0;");
  timeRow->addWidget(timeSeparator);
  
  m_endTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime());
  m_endTimeEdit->setCalendarPopup(true);
  m_endTimeEdit->setDisplayFormat("MM/dd hh:mm");
  m_endTimeEdit->setFixedHeight(28);
  m_endTimeEdit->setStyleSheet(m_startTimeEdit->styleSheet());
  timeRow->addWidget(m_endTimeEdit);
  filterLayout->addLayout(timeRow);

  // 结果筛选 + 搜索按钮
  auto* actionRow = new QHBoxLayout();
  m_resultCombo = new QComboBox();
  m_resultCombo->addItem(tr("全部"), "");
  m_resultCombo->addItem(tr("OK"), "OK");
  m_resultCombo->addItem(tr("NG"), "NG");
  m_resultCombo->setFixedHeight(28);
  m_resultCombo->setStyleSheet("QComboBox { border: 1px solid #555; border-radius: 3px; padding: 2px 6px; font-size: 12px; background: #2C2C2E; color: #E0E0E0; }");
  actionRow->addWidget(m_resultCombo);

  m_searchBtn = new QPushButton(tr("查询"));
  m_searchBtn->setFixedHeight(28);
  m_searchBtn->setStyleSheet(R"(
    QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 3px; padding: 0 12px; font-size: 12px; }
    QPushButton:hover { background-color: #43A047; }
  )");
  actionRow->addWidget(m_searchBtn);

  m_resetBtn = new QPushButton(tr("重置"));
  m_resetBtn->setFixedHeight(28);
  m_resetBtn->setStyleSheet(R"(
    QPushButton { background-color: #555; color: white; border: none; border-radius: 3px; padding: 0 12px; font-size: 12px; }
    QPushButton:hover { background-color: #666; }
  )");
  actionRow->addWidget(m_resetBtn);

  m_deleteBtn = new QPushButton(tr("删除"));
  m_deleteBtn->setFixedHeight(28);
  m_deleteBtn->setEnabled(false);
  m_deleteBtn->setStyleSheet(R"(
    QPushButton { background-color: #e53935; color: white; border: none; border-radius: 3px; padding: 0 12px; font-size: 12px; }
    QPushButton:hover { background-color: #d32f2f; }
    QPushButton:disabled { background-color: #555; color: #888; }
  )");
  // 只有有删除权限才显示
  m_deleteBtn->setVisible(UserManager::instance()->hasPermission(Permission::DeleteHistory));
  actionRow->addWidget(m_deleteBtn);

  filterLayout->addLayout(actionRow);

  listLayout->addWidget(filterWidget);

  // 记录列表
  m_tableView = new QTableView();
  m_model = new HistoryTableModel(this);
  m_tableView->setModel(m_model);
  m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tableView->setAlternatingRowColors(true);
  m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tableView->setShowGrid(false);
  m_tableView->horizontalHeader()->setStretchLastSection(true);
  m_tableView->verticalHeader()->setVisible(false);
  m_tableView->verticalHeader()->setDefaultSectionSize(32);
  m_tableView->setColumnWidth(0, 40);   // ID
  m_tableView->setColumnWidth(1, 100);  // 时间
  m_tableView->setColumnWidth(2, 40);   // 结果
  m_tableView->setColumnHidden(3, true);  // 隐藏缺陷数
  m_tableView->setColumnHidden(4, true);  // 隐藏严重度
  m_tableView->setColumnHidden(5, true);  // 隐藏耗时
  m_tableView->setColumnHidden(6, true);  // 隐藏路径
  m_tableView->setStyleSheet(R"(
    QTableView {
      border: none;
      background-color: #2C2C2E;
      color: #E0E0E0;
      font-size: 12px;
    }
    QTableView::item {
      padding: 6px;
      border-bottom: 1px solid #48484A;
    }
    QTableView::item:selected {
      background-color: #4CAF50;
      color: white;
    }
    QTableView::item:hover:!selected {
      background-color: #3C3C3E;
    }
    QHeaderView::section {
      background-color: #3C3C3E;
      border: none;
      border-bottom: 1px solid #48484A;
      padding: 6px;
      font-size: 11px;
      font-weight: 500;
      color: #ADADAD;
    }
  )");
  listLayout->addWidget(m_tableView, 1);

  // 分页
  auto* pageWidget = new QWidget();
  pageWidget->setFixedHeight(40);
  pageWidget->setStyleSheet("background-color: #3C3C3E; border-top: 1px solid #48484A;");
  auto* pageLayout = new QHBoxLayout(pageWidget);
  pageLayout->setContentsMargins(8, 0, 8, 0);

  m_pageInfoLabel = new QLabel(tr("0 条"));
  m_pageInfoLabel->setStyleSheet("color: #ADADAD; font-size: 11px;");
  pageLayout->addWidget(m_pageInfoLabel);

  pageLayout->addStretch();

  m_pageSizeBox = new QSpinBox();
  m_pageSizeBox->setRange(20, 200);
  m_pageSizeBox->setValue(50);
  m_pageSizeBox->setFixedWidth(60);
  m_pageSizeBox->setFixedHeight(24);
  m_pageSizeBox->setStyleSheet("QSpinBox { border: 1px solid #555; border-radius: 3px; font-size: 11px; background: #2C2C2E; color: #E0E0E0; }");
  pageLayout->addWidget(m_pageSizeBox);

  m_prevPageBtn = new QPushButton("<");
  m_prevPageBtn->setFixedSize(24, 24);
  m_prevPageBtn->setEnabled(false);
  m_prevPageBtn->setStyleSheet("QPushButton { border: 1px solid #555; border-radius: 3px; background: #2C2C2E; color: #E0E0E0; } QPushButton:hover:!disabled { background-color: #48484A; }");
  pageLayout->addWidget(m_prevPageBtn);

  m_nextPageBtn = new QPushButton(">");
  m_nextPageBtn->setFixedSize(24, 24);
  m_nextPageBtn->setEnabled(false);
  m_nextPageBtn->setStyleSheet(m_prevPageBtn->styleSheet());
  pageLayout->addWidget(m_nextPageBtn);

  listLayout->addWidget(pageWidget);

  mainLayout->addWidget(listPanel, 3);  // 30% 宽度

  // 隐藏不需要的成员（保留接口兼容）
  m_detailLabel = new QLabel(this);
  m_detailLabel->hide();
}

void HistoryView::setupConnections() {
  connect(m_searchBtn, &QPushButton::clicked, this, &HistoryView::onSearchClicked);
  connect(m_resetBtn, &QPushButton::clicked, this, &HistoryView::onResetClicked);
  connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &HistoryView::onTableSelectionChanged);
  connect(m_model, &HistoryTableModel::dataLoaded, this, &HistoryView::onDataLoaded);
  connect(m_prevPageBtn, &QPushButton::clicked, this, [this]() {
    if (m_currentPage > 0) onPageChanged(m_currentPage - 1);
  });
  connect(m_nextPageBtn, &QPushButton::clicked, this, [this]() {
    onPageChanged(m_currentPage + 1);
  });
  connect(m_pageSizeBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() {
    m_currentPage = 0;
    onSearchClicked();
  });
  connect(m_deleteBtn, &QPushButton::clicked, this, &HistoryView::onDeleteClicked);

  // 双击预览图放大显示
  m_previewView->viewport()->installEventFilter(this);
}

bool HistoryView::eventFilter(QObject* obj, QEvent* event) {
  if (obj == m_previewView->viewport() && event->type() == QEvent::MouseButtonDblClick) {
    if (!m_currentImagePath.isEmpty() && QFileInfo::exists(m_currentImagePath)) {
      auto* dialog = new ImagePreviewDialog(this);
      dialog->setImage(m_currentImagePath);
      
      // 设置数据库管理器和记录ID，用于保存标注
      dialog->setDatabaseManager(m_dbManager);
      dialog->setInspectionId(m_currentRecordId);
      
      // 传递缺陷标注数据
      if (!m_currentDefects.isEmpty()) {
        std::vector<cv::Rect> boxes;
        QStringList labels;
        std::vector<double> confidences;
        
        for (const auto& defect : m_currentDefects) {
          boxes.push_back(cv::Rect(defect.region.x(), defect.region.y(),
                                   defect.region.width(), defect.region.height()));
          labels.append(defect.defectType);
          confidences.push_back(defect.confidence);
        }
        
        dialog->setDetectionResults(boxes, labels, confidences);
      }
      
      // 连接保存信号，保存后刷新列表
      connect(dialog, &ImagePreviewDialog::annotationsSavedToDatabase, this, [this]() {
        // 重新加载当前记录的缺陷数据
        if (m_dbManager && m_dbManager->isOpen() && m_currentRecordId > 0) {
          m_currentDefects = m_dbManager->defectRepository()->getDefects(m_currentRecordId);
          drawDefectsOnPreview(m_currentDefects);
        }
      });
      
      dialog->exec();
      dialog->deleteLater();
    }
    return true;
  }
  return QWidget::eventFilter(obj, event);
}

void HistoryView::onSearchClicked() {
  if (!m_dbManager || !m_dbManager->isOpen()) return;

  InspectionFilter filter;
  filter.startTime = m_startTimeEdit->dateTime();
  filter.endTime = m_endTimeEdit->dateTime();
  filter.result = m_resultCombo->currentData().toString();
  filter.limit = m_pageSizeBox->value();
  filter.offset = m_currentPage * filter.limit;

  LOG_DEBUG("HistoryView::onSearchClicked - filter: {}-{}, result={}, page={}", 
            filter.startTime.toString("yyyy-MM-dd").toStdString(),
            filter.endTime.toString("yyyy-MM-dd").toStdString(),
            filter.result.isEmpty() ? "all" : filter.result.toStdString(),
            m_currentPage);

  m_model->refresh(filter);
  m_totalCount = m_dbManager->defectRepository()->countInspections(filter);
  updatePagination();
  
  LOG_DEBUG("HistoryView::onSearchClicked - Found {} records", m_totalCount);
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
    m_currentInfoLabel->setText(tr("请选择记录"));
    m_deleteBtn->setEnabled(false);
    return;
  }

  int row = indexes.first().row();
  auto record = m_model->recordAt(row);
  displayRecord(record);
  
  m_currentInfoLabel->setText(tr("第 %1/%2 条").arg(row + 1).arg(m_model->rowCount()));
  m_deleteBtn->setEnabled(UserManager::instance()->hasPermission(Permission::DeleteHistory));
  emit recordSelected(record.id);
}

void HistoryView::onDeleteClicked() {
  if (!UserManager::instance()->hasPermission(Permission::DeleteHistory)) {
    QMessageBox::warning(this, tr("权限不足"), tr("您没有删除历史记录的权限"));
    return;
  }

  auto indexes = m_tableView->selectionModel()->selectedRows();
  if (indexes.isEmpty()) return;

  auto result = QMessageBox::question(this, tr("确认删除"),
                                       tr("确定要删除选中的记录吗？此操作不可恢复。"));
  if (result != QMessageBox::Yes) return;

  int row = indexes.first().row();
  auto record = m_model->recordAt(row);

  if (m_dbManager && m_dbManager->isOpen()) {
    // 删除记录
    if (m_dbManager->defectRepository()->deleteInspection(record.id)) {
      LOG_INFO("HistoryView::onDeleteClicked - Deleted record id={}", record.id);
      QMessageBox::information(this, tr("成功"), tr("记录已删除"));
      onSearchClicked();  // 刷新列表
    } else {
      LOG_ERROR("HistoryView::onDeleteClicked - Failed to delete record id={}", record.id);
      QMessageBox::warning(this, tr("错误"), tr("删除失败"));
    }
  }
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
  int totalPages = qMax(1, (m_totalCount + pageSize - 1) / pageSize);

  m_pageInfoLabel->setText(tr("%1 条 | %2/%3").arg(m_totalCount).arg(m_currentPage + 1).arg(totalPages));
  m_prevPageBtn->setEnabled(m_currentPage > 0);
  m_nextPageBtn->setEnabled((m_currentPage + 1) * pageSize < m_totalCount);
}

void HistoryView::displayRecord(const InspectionRecord& record) {
  // 保存当前记录信息
  m_currentImagePath = record.imagePath;
  m_currentRecordId = record.id;

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
  m_currentDefects.clear();
  if (m_dbManager && m_dbManager->isOpen() && record.id > 0) {
    m_currentDefects = m_dbManager->defectRepository()->getDefects(record.id);
    drawDefectsOnPreview(m_currentDefects);
  }

  // 更新底部信息栏
  auto setInfo = [](QWidget* w, const QString& val, const QString& color = "#e0e0e0") {
    auto* label = w->findChild<QLabel*>("value");
    if (label) {
      label->setText(val);
      label->setStyleSheet(QString("color: %1; font-size: 15px; font-weight: 500;").arg(color));
    }
  };

  setInfo(m_infoTime, record.inspectTime.toString("MM-dd hh:mm:ss"));
  setInfo(m_infoResult, record.result, record.result == "OK" ? "#4caf50" : "#f44336");
  setInfo(m_infoDefects, QString::number(record.defectCount));
  setInfo(m_infoCycleTime, QString("%1 ms").arg(record.cycleTimeMs));

  // 更新严重度条
  m_severityBar->setLevel(record.severityLevel.isEmpty() ? "OK" : record.severityLevel);
}

void HistoryView::drawDefectsOnPreview(const QVector<DefectRecord>& defects) {
  m_previewView->clearDetectionBoxes();
  if (defects.isEmpty()) return;

  QVector<DetectionBox> boxes;
  for (const auto& defect : defects) {
    DetectionBox box;
    box.rect = cv::Rect(defect.region.x(), defect.region.y(),
                        defect.region.width(), defect.region.height());
    box.label = defect.defectType;
    box.confidence = defect.confidence;
    box.color = (defect.severityLevel == "Minor") ? QColor(255, 193, 7) :
                (defect.severityLevel == "Major") ? QColor(255, 152, 0) : QColor(244, 67, 54);
    boxes.append(box);
  }
  m_previewView->drawDetectionBoxes(boxes);
}
