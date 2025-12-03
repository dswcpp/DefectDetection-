/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * SPCView.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：SPC统计过程控制视图接口定义
 * 描述：SPC控制图界面，显示X-bar图、R图、Cpk计算等质量控制工具
 *
 * 当前版本：1.0
 */

#ifndef SPCVIEW_H
#define SPCVIEW_H

#include <QWidget>
#include <QDateTime>
#include <vector>

QT_BEGIN_NAMESPACE
class QLabel;
class QListWidget;
class QPushButton;
class QComboBox;
class QDateTimeEdit;
QT_END_NAMESPACE

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QValueAxis>

// SPC数据点
struct SPCDataPoint {
    QDateTime timestamp;
    double value;
    int sampleGroup;  // 样本组号
};

// SPC统计信息
struct SPCStatistics {
    double mean;        // 均值
    double stdDev;      // 标准差
    double ucl;         // 上控制限
    double lcl;         // 下控制限
    double cl;          // 中心线
    double cpk;         // 过程能力指数
    double ppk;         // 性能指数
    double cp;          // 潜在过程能力指数
    double pp;          // 潜在性能指数
};

class SPCView : public QWidget {
    Q_OBJECT

public:
    explicit SPCView(QWidget* parent = nullptr);
    ~SPCView() = default;

    // 设置规格限
    void setSpecificationLimits(double lsl, double usl);

    // 添加数据
    void addDataPoint(const SPCDataPoint& point);
    void setData(const std::vector<SPCDataPoint>& data);

public slots:
    void refresh();
    void setTimeRange(const QDateTime& start, const QDateTime& end);
    void exportReport();
    void clearData();

signals:
    void dataUpdated();
    void alarmTriggered(const QString& message);

private slots:
    void onChartTypeChanged(int index);
    void onRefreshClicked();
    void onExportClicked();

private:
    void setupUI();
    void createCharts();
    void updateXBarChart();
    void updateRChart();
    void updateHistogram();
    void updateCapabilityIndicators();
    void calculateStatistics();
    void checkRules();  // 检查控制图规则
    void addAlarm(const QString& message, const QString& type);

    // Western Electric规则检查
    bool checkRule1();  // 1个点超出3σ限制
    bool checkRule2();  // 连续9个点在中心线同一侧
    bool checkRule3();  // 连续6个点递增或递减
    bool checkRule4();  // 连续14个点交替上下

    // UI组件
    QChartView* m_xBarChart;      // X-Bar 控制图
    QChartView* m_rChart;         // R 控制图
    QChartView* m_histogramChart; // 直方图

    QComboBox* m_chartTypeCombo;
    QDateTimeEdit* m_startDateEdit;
    QDateTimeEdit* m_endDateEdit;
    QPushButton* m_refreshBtn;
    QPushButton* m_exportBtn;

    // 指标标签
    QLabel* m_meanLabel;
    QLabel* m_stdDevLabel;
    QLabel* m_cpLabel;
    QLabel* m_cpkLabel;
    QLabel* m_ppLabel;
    QLabel* m_ppkLabel;
    QLabel* m_uclLabel;
    QLabel* m_clLabel;
    QLabel* m_lclLabel;

    // 告警列表
    QListWidget* m_alarmList;

    // 数据
    std::vector<SPCDataPoint> m_data;
    std::vector<SPCDataPoint> m_filteredData;
    SPCStatistics m_statistics;

    // 规格限
    double m_lsl = 0.0;  // 下规格限
    double m_usl = 100.0;  // 上规格限
    bool m_hasSpecLimits = false;

    // 子组大小（用于计算R图）
    int m_subgroupSize = 5;
};

#endif // SPCVIEW_H
