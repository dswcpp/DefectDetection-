#include "AlarmDialog.h"
#include "common/Logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QGroupBox>

AlarmDialog::AlarmDialog(QWidget* parent) : FramelessDialog(parent) {
    setDialogTitle(tr("报警记录"));
    setShowMaxButton(true);
    setupUI();
    updateStatistics();
}

void AlarmDialog::setupUI() {
    setMinimumSize(900, 650);
    resize(1000, 750);

    // 主布局
    auto* mainLayout = contentLayout();
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 顶部工具栏
    auto* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(10);

    // 级别筛选
    auto* filterLabel = new QLabel(tr("级别筛选:"));
    toolbarLayout->addWidget(filterLabel);

    m_levelFilter = new QComboBox();
    m_levelFilter->addItems({tr("全部"), tr("信息"), tr("警告"), tr("错误"), tr("严重")});
    m_levelFilter->setMinimumWidth(100);
    connect(m_levelFilter, &QComboBox::currentIndexChanged, this, &AlarmDialog::onFilterChanged);
    toolbarLayout->addWidget(m_levelFilter);

    toolbarLayout->addStretch();

    // 统计信息
    auto* statsGroup = new QGroupBox(tr("统计"));
    auto* statsLayout = new QHBoxLayout(statsGroup);

    m_totalLabel = new QLabel(tr("总计: 0"));
    m_totalLabel->setStyleSheet("font-weight: bold; color: #E0E0E0;");
    statsLayout->addWidget(m_totalLabel);

    m_unackedLabel = new QLabel(tr("未确认: 0"));
    m_unackedLabel->setStyleSheet("font-weight: bold; color: #ff9800;");
    statsLayout->addWidget(m_unackedLabel);

    toolbarLayout->addWidget(statsGroup);

    toolbarLayout->addStretch();

    // 操作按钮
    m_ackAllBtn = new QPushButton(tr("确认全部"));
    m_ackAllBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 5px 15px;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
    )");
    connect(m_ackAllBtn, &QPushButton::clicked, this, &AlarmDialog::acknowledgeAll);
    toolbarLayout->addWidget(m_ackAllBtn);

    m_clearBtn = new QPushButton(tr("清除"));
    m_clearBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #f44336;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 5px 15px;
        }
        QPushButton:hover {
            background-color: #da190b;
        }
    )");
    connect(m_clearBtn, &QPushButton::clicked, this, &AlarmDialog::clearAlarms);
    toolbarLayout->addWidget(m_clearBtn);

    m_exportBtn = new QPushButton(tr("导出"));
    m_exportBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #2196F3;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 5px 15px;
        }
        QPushButton:hover {
            background-color: #0b7dda;
        }
    )");
    connect(m_exportBtn, &QPushButton::clicked, this, &AlarmDialog::onExport);
    toolbarLayout->addWidget(m_exportBtn);

    mainLayout->addLayout(toolbarLayout);

    // 报警表格
    m_alarmTable = new QTableWidget();
    m_alarmTable->setColumnCount(6);
    m_alarmTable->setHorizontalHeaderLabels({
        tr("ID"), tr("时间"), tr("级别"), tr("模块"), tr("消息"), tr("操作")
    });

    // 设置表格样式
    m_alarmTable->setAlternatingRowColors(true);
    m_alarmTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_alarmTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_alarmTable->horizontalHeader()->setStretchLastSection(false);
    m_alarmTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_alarmTable->verticalHeader()->setVisible(false);

    // 设置列宽
    m_alarmTable->setColumnWidth(0, 60);   // ID
    m_alarmTable->setColumnWidth(1, 150);  // 时间
    m_alarmTable->setColumnWidth(2, 80);   // 级别
    m_alarmTable->setColumnWidth(3, 120);  // 模块
    // 消息列自动拉伸
    m_alarmTable->setColumnWidth(5, 80);   // 操作

    m_alarmTable->setStyleSheet(R"(
        QTableWidget {
            border: 1px solid #555;
            background-color: #2C2C2E;
            alternate-background-color: #333335;
            gridline-color: #555;
            color: #E0E0E0;
        }
        QTableWidget::item {
            padding: 5px;
        }
        QTableWidget::item:selected {
            background-color: #4CAF50;
            color: white;
        }
        QHeaderView::section {
            background-color: #3C3C3E;
            padding: 5px;
            border: none;
            font-weight: bold;
            color: #E0E0E0;
        }
    )");

    mainLayout->addWidget(m_alarmTable);

    // 底部关闭按钮
    auto* bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();

    auto* closeBtn = new QPushButton(tr("关闭"));
    closeBtn->setFixedWidth(100);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #607d8b;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 16px;
        }
        QPushButton:hover {
            background-color: #455a64;
        }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    bottomLayout->addWidget(closeBtn);

    bottomLayout->addStretch();
    mainLayout->addLayout(bottomLayout);
}

void AlarmDialog::addAlarm(const QString& module, const QString& message, AlarmLevel level) {
    AlarmEntry entry;
    entry.id = ++m_alarmIdCounter;
    entry.timestamp = QDateTime::currentDateTime();
    entry.module = module;
    entry.message = message;
    entry.level = level;
    entry.acknowledged = false;

    m_alarms.prepend(entry);  // 最新的在前面
    updateTable();
    updateStatistics();
    
    LOG_INFO("AlarmDialog::addAlarm - id={}, module={}, level={}, message={}", 
             entry.id, module.toStdString(), static_cast<int>(level), message.toStdString());
}

void AlarmDialog::clearAlarms() {
    if (m_alarms.isEmpty()) return;

    auto ret = QMessageBox::question(this, tr("确认"),
        tr("确定要清除所有报警记录吗？"),
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        int count = m_alarms.size();
        m_alarms.clear();
        updateTable();
        updateStatistics();
        LOG_INFO("AlarmDialog::clearAlarms - Cleared {} alarms", count);
    }
}

void AlarmDialog::acknowledgeAlarm(int row) {
    if (row >= 0 && row < m_alarms.size()) {
        m_alarms[row].acknowledged = true;
        updateTable();
        updateStatistics();
        LOG_DEBUG("AlarmDialog::acknowledgeAlarm - Acknowledged alarm id={}", m_alarms[row].id);
    }
}

void AlarmDialog::acknowledgeAll() {
    int count = 0;
    for (auto& alarm : m_alarms) {
        if (!alarm.acknowledged) {
            alarm.acknowledged = true;
            count++;
        }
    }
    updateTable();
    updateStatistics();
    LOG_INFO("AlarmDialog::acknowledgeAll - Acknowledged {} alarms", count);
}

void AlarmDialog::onFilterChanged() {
    updateTable();
}

void AlarmDialog::onExport() {
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("导出报警记录"),
        QString("alarms_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
        tr("CSV Files (*.csv)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            // Qt6中默认使用UTF-8编码

            // 写入表头
            stream << "ID,时间,级别,模块,消息,状态\n";

            // 写入数据
            for (const auto& alarm : m_alarms) {
                stream << alarm.id << ","
                       << alarm.timestamp.toString("yyyy-MM-dd HH:mm:ss") << ","
                       << levelToString(alarm.level) << ","
                       << alarm.module << ","
                       << alarm.message << ","
                       << (alarm.acknowledged ? "已确认" : "未确认") << "\n";
            }

            file.close();
            QMessageBox::information(this, tr("导出成功"),
                tr("报警记录已导出到: %1").arg(fileName));
        }
    }
}

void AlarmDialog::updateStatistics() {
    m_totalAlarms = m_alarms.size();
    m_unackedAlarms = 0;

    for (const auto& alarm : m_alarms) {
        if (!alarm.acknowledged) {
            m_unackedAlarms++;
        }
    }

    m_totalLabel->setText(tr("总计: %1").arg(m_totalAlarms));
    m_unackedLabel->setText(tr("未确认: %1").arg(m_unackedAlarms));
}

void AlarmDialog::updateTable() {
    m_alarmTable->setRowCount(0);

    QString filterText = m_levelFilter->currentText();

    for (int i = 0; i < m_alarms.size(); ++i) {
        const auto& alarm = m_alarms[i];

        // 应用筛选
        if (filterText != tr("全部")) {
            if (levelToString(alarm.level) != filterText) {
                continue;
            }
        }

        int row = m_alarmTable->rowCount();
        m_alarmTable->insertRow(row);

        // ID
        auto* idItem = new QTableWidgetItem(QString::number(alarm.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        m_alarmTable->setItem(row, 0, idItem);

        // 时间
        auto* timeItem = new QTableWidgetItem(alarm.timestamp.toString("yyyy-MM-dd HH:mm:ss"));
        m_alarmTable->setItem(row, 1, timeItem);

        // 级别
        auto* levelItem = new QTableWidgetItem(levelToString(alarm.level));
        levelItem->setTextAlignment(Qt::AlignCenter);
        levelItem->setBackground(QBrush(levelToColor(alarm.level).lighter(180)));
        levelItem->setForeground(QBrush(levelToColor(alarm.level).darker(150)));
        m_alarmTable->setItem(row, 2, levelItem);

        // 模块
        auto* moduleItem = new QTableWidgetItem(alarm.module);
        m_alarmTable->setItem(row, 3, moduleItem);

        // 消息
        auto* msgItem = new QTableWidgetItem(alarm.message);
        m_alarmTable->setItem(row, 4, msgItem);

        // 操作按钮
        if (!alarm.acknowledged) {
            auto* ackBtn = new QPushButton(tr("确认"));
            ackBtn->setStyleSheet(R"(
                QPushButton {
                    background-color: #4CAF50;
                    color: white;
                    border: none;
                    border-radius: 3px;
                    padding: 2px 8px;
                    font-size: 12px;
                }
                QPushButton:hover {
                    background-color: #45a049;
                }
            )");
            connect(ackBtn, &QPushButton::clicked, [this, i]() {
                acknowledgeAlarm(i);
            });
            m_alarmTable->setCellWidget(row, 5, ackBtn);
        } else {
            auto* label = new QLabel(tr("已确认"));
            label->setAlignment(Qt::AlignCenter);
            label->setStyleSheet("color: #4CAF50;");
            m_alarmTable->setCellWidget(row, 5, label);
        }

        // 未确认的行高亮显示
        if (!alarm.acknowledged) {
            for (int col = 0; col < m_alarmTable->columnCount(); ++col) {
                auto* item = m_alarmTable->item(row, col);
                if (item) {
                    item->setBackground(QBrush(QColor("#fff3e0")));
                }
            }
        }
    }
}

QString AlarmDialog::levelToString(AlarmLevel level) const {
    switch (level) {
        case AlarmLevel::Info:     return tr("信息");
        case AlarmLevel::Warning:  return tr("警告");
        case AlarmLevel::Error:    return tr("错误");
        case AlarmLevel::Critical: return tr("严重");
        default:                   return tr("未知");
    }
}

QColor AlarmDialog::levelToColor(AlarmLevel level) const {
    switch (level) {
        case AlarmLevel::Info:     return QColor("#2196F3");  // 蓝色
        case AlarmLevel::Warning:  return QColor("#ff9800");  // 橙色
        case AlarmLevel::Error:    return QColor("#f44336");  // 红色
        case AlarmLevel::Critical: return QColor("#9c27b0");  // 紫色
        default:                   return QColor("#9e9e9e");  // 灰色
    }
}