/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * AlarmDialog.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：报警对话框接口定义
 * 描述：报警信息显示和处理对话框，展示报警详情、确认操作等
 *
 * 当前版本：1.0
 */

#ifndef ALARMDIALOG_H
#define ALARMDIALOG_H

#include <QDialog>
#include <QDateTime>
#include "../../common/Types.h"

QT_BEGIN_NAMESPACE
class QTableWidget;
class QComboBox;
class QPushButton;
class QLabel;
QT_END_NAMESPACE

struct AlarmEntry {
    int id;
    QDateTime timestamp;
    QString module;
    QString message;
    AlarmLevel level;
    bool acknowledged;
};

class AlarmDialog : public QDialog {
    Q_OBJECT

public:
    explicit AlarmDialog(QWidget* parent = nullptr);
    ~AlarmDialog() = default;

public slots:
    void addAlarm(const QString& module, const QString& message, AlarmLevel level);
    void clearAlarms();
    void acknowledgeAlarm(int row);
    void acknowledgeAll();

private slots:
    void onFilterChanged();
    void onExport();
    void updateStatistics();

private:
    void setupUI();
    void updateTable();
    QString levelToString(AlarmLevel level) const;
    QColor levelToColor(AlarmLevel level) const;

    // UI元素
    QTableWidget* m_alarmTable;
    QComboBox* m_levelFilter;
    QLabel* m_totalLabel;
    QLabel* m_unackedLabel;
    QPushButton* m_clearBtn;
    QPushButton* m_ackAllBtn;
    QPushButton* m_exportBtn;

    // 数据
    QList<AlarmEntry> m_alarms;
    int m_alarmIdCounter = 0;
    int m_totalAlarms = 0;
    int m_unackedAlarms = 0;
};

#endif // ALARMDIALOG_H