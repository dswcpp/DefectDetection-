#include "SPCView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <cmath>
#include <numeric>
#include <algorithm>


SPCView::SPCView(QWidget* parent) : QWidget(parent) {
    setupUI();
    createCharts();
}

void SPCView::setSpecificationLimits(double lsl, double usl) {
    m_lsl = lsl;
    m_usl = usl;
    m_hasSpecLimits = true;
    calculateStatistics();
    updateCapabilityIndicators();
}

void SPCView::addDataPoint(const SPCDataPoint& point) {
    m_data.push_back(point);
    refresh();
}

void SPCView::setData(const std::vector<SPCDataPoint>& data) {
    m_data = data;
    refresh();
}

void SPCView::refresh() {
    calculateStatistics();
    updateXBarChart();
    updateRChart();
    updateHistogram();
    updateCapabilityIndicators();
    checkRules();
    emit dataUpdated();
}

void SPCView::setTimeRange(const QDateTime& start, const QDateTime& end) {
    m_filteredData.clear();

    for (const auto& point : m_data) {
        if (point.timestamp >= start && point.timestamp <= end) {
            m_filteredData.push_back(point);
        }
    }

    refresh();
}

void SPCView::exportReport() {
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("导出SPC报告"),
        QString("SPC_Report_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
        tr("CSV Files (*.csv)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);

            // 写入统计信息
            stream << "SPC Statistical Report\n";
            stream << "Generated: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n\n";

            stream << "Statistical Indicators:\n";
            stream << "Mean," << m_statistics.mean << "\n";
            stream << "Standard Deviation," << m_statistics.stdDev << "\n";
            stream << "UCL," << m_statistics.ucl << "\n";
            stream << "CL," << m_statistics.cl << "\n";
            stream << "LCL," << m_statistics.lcl << "\n";

            if (m_hasSpecLimits) {
                stream << "Cp," << m_statistics.cp << "\n";
                stream << "Cpk," << m_statistics.cpk << "\n";
                stream << "Pp," << m_statistics.pp << "\n";
                stream << "Ppk," << m_statistics.ppk << "\n";
            }

            stream << "\nData Points:\n";
            stream << "Timestamp,Value,Sample Group\n";

            const auto& dataToExport = m_filteredData.empty() ? m_data : m_filteredData;
            for (const auto& point : dataToExport) {
                stream << point.timestamp.toString("yyyy-MM-dd HH:mm:ss") << ","
                       << point.value << ","
                       << point.sampleGroup << "\n";
            }

            file.close();
            QMessageBox::information(this, tr("导出成功"),
                tr("SPC报告已导出到: %1").arg(fileName));
        }
    }
}

void SPCView::clearData() {
    m_data.clear();
    m_filteredData.clear();
    m_alarmList->clear();
    refresh();
}

void SPCView::onChartTypeChanged(int index) {
    // 切换显示不同的图表
    m_xBarChart->setVisible(index == 0);
    m_rChart->setVisible(index == 1);
    m_histogramChart->setVisible(index == 2);
}

void SPCView::onRefreshClicked() {
    QDateTime start = m_startDateEdit->dateTime();
    QDateTime end = m_endDateEdit->dateTime();
    setTimeRange(start, end);
}

void SPCView::onExportClicked() {
    exportReport();
}

void SPCView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 顶部控制栏
    auto* controlBar = new QWidget();
    auto* controlLayout = new QHBoxLayout(controlBar);

    // 图表类型选择
    auto* chartTypeLabel = new QLabel(tr("图表类型:"));
    controlLayout->addWidget(chartTypeLabel);

    m_chartTypeCombo = new QComboBox();
    m_chartTypeCombo->addItems({tr("X-Bar 控制图"), tr("R 控制图"), tr("直方图")});
    connect(m_chartTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SPCView::onChartTypeChanged);
    controlLayout->addWidget(m_chartTypeCombo);

    controlLayout->addSpacing(20);

    // 时间范围选择
    auto* timeLabel = new QLabel(tr("时间范围:"));
    controlLayout->addWidget(timeLabel);

    m_startDateEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-7));
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    controlLayout->addWidget(m_startDateEdit);

    auto* toLabel = new QLabel(tr("至"));
    controlLayout->addWidget(toLabel);

    m_endDateEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    controlLayout->addWidget(m_endDateEdit);

    // 按钮
    m_refreshBtn = new QPushButton(tr("刷新"));
    m_refreshBtn->setIcon(QIcon(":/icons/refresh.svg"));
    connect(m_refreshBtn, &QPushButton::clicked, this, &SPCView::onRefreshClicked);
    controlLayout->addWidget(m_refreshBtn);

    m_exportBtn = new QPushButton(tr("导出"));
    m_exportBtn->setIcon(QIcon(":/icons/export.svg"));
    connect(m_exportBtn, &QPushButton::clicked, this, &SPCView::onExportClicked);
    controlLayout->addWidget(m_exportBtn);

    controlLayout->addStretch();

    mainLayout->addWidget(controlBar);

    // 主体区域（图表和指标）
    auto* contentWidget = new QWidget();
    auto* contentLayout = new QHBoxLayout(contentWidget);

    // 左侧图表区域
    auto* chartWidget = new QWidget();
    auto* chartLayout = new QVBoxLayout(chartWidget);
    chartLayout->setContentsMargins(0, 0, 0, 0);

    m_xBarChart = new QChartView();
    m_xBarChart->setRenderHint(QPainter::Antialiasing);
    chartLayout->addWidget(m_xBarChart);

    m_rChart = new QChartView();
    m_rChart->setRenderHint(QPainter::Antialiasing);
    m_rChart->setVisible(false);
    chartLayout->addWidget(m_rChart);

    m_histogramChart = new QChartView();
    m_histogramChart->setRenderHint(QPainter::Antialiasing);
    m_histogramChart->setVisible(false);
    chartLayout->addWidget(m_histogramChart);

    contentLayout->addWidget(chartWidget, 3);

    // 右侧指标和告警区域
    auto* rightPanel = new QWidget();
    rightPanel->setMaximumWidth(350);
    auto* rightLayout = new QVBoxLayout(rightPanel);

    // 统计指标组
    auto* statsGroup = new QGroupBox(tr("统计指标"));
    auto* statsLayout = new QGridLayout();

    // 基本统计
    statsLayout->addWidget(new QLabel(tr("均值:")), 0, 0);
    m_meanLabel = new QLabel("--");
    m_meanLabel->setStyleSheet("font-weight: bold;");
    statsLayout->addWidget(m_meanLabel, 0, 1);

    statsLayout->addWidget(new QLabel(tr("标准差:")), 1, 0);
    m_stdDevLabel = new QLabel("--");
    m_stdDevLabel->setStyleSheet("font-weight: bold;");
    statsLayout->addWidget(m_stdDevLabel, 1, 1);

    // 控制限
    statsLayout->addWidget(new QLabel(tr("UCL:")), 2, 0);
    m_uclLabel = new QLabel("--");
    m_uclLabel->setStyleSheet("color: #f44336; font-weight: bold;");
    statsLayout->addWidget(m_uclLabel, 2, 1);

    statsLayout->addWidget(new QLabel(tr("CL:")), 3, 0);
    m_clLabel = new QLabel("--");
    m_clLabel->setStyleSheet("color: #4CAF50; font-weight: bold;");
    statsLayout->addWidget(m_clLabel, 3, 1);

    statsLayout->addWidget(new QLabel(tr("LCL:")), 4, 0);
    m_lclLabel = new QLabel("--");
    m_lclLabel->setStyleSheet("color: #f44336; font-weight: bold;");
    statsLayout->addWidget(m_lclLabel, 4, 1);

    statsGroup->setLayout(statsLayout);
    rightLayout->addWidget(statsGroup);

    // 过程能力指标组
    auto* capabilityGroup = new QGroupBox(tr("过程能力指标"));
    auto* capabilityLayout = new QGridLayout();

    capabilityLayout->addWidget(new QLabel(tr("Cp:")), 0, 0);
    m_cpLabel = new QLabel("--");
    m_cpLabel->setStyleSheet("font-weight: bold;");
    capabilityLayout->addWidget(m_cpLabel, 0, 1);

    capabilityLayout->addWidget(new QLabel(tr("Cpk:")), 1, 0);
    m_cpkLabel = new QLabel("--");
    m_cpkLabel->setStyleSheet("font-weight: bold;");
    capabilityLayout->addWidget(m_cpkLabel, 1, 1);

    capabilityLayout->addWidget(new QLabel(tr("Pp:")), 2, 0);
    m_ppLabel = new QLabel("--");
    m_ppLabel->setStyleSheet("font-weight: bold;");
    capabilityLayout->addWidget(m_ppLabel, 2, 1);

    capabilityLayout->addWidget(new QLabel(tr("Ppk:")), 3, 0);
    m_ppkLabel = new QLabel("--");
    m_ppkLabel->setStyleSheet("font-weight: bold;");
    capabilityLayout->addWidget(m_ppkLabel, 3, 1);

    capabilityGroup->setLayout(capabilityLayout);
    rightLayout->addWidget(capabilityGroup);

    // 告警列表
    auto* alarmGroup = new QGroupBox(tr("控制规则告警"));
    auto* alarmLayout = new QVBoxLayout();

    m_alarmList = new QListWidget();
    m_alarmList->setMaximumHeight(200);
    alarmLayout->addWidget(m_alarmList);

    alarmGroup->setLayout(alarmLayout);
    rightLayout->addWidget(alarmGroup);

    rightLayout->addStretch();

    contentLayout->addWidget(rightPanel, 1);

    mainLayout->addWidget(contentWidget);
}

void SPCView::createCharts() {
    // 创建X-Bar控制图
    auto* xBarChart = new QChart();
    xBarChart->setTitle(tr("X-Bar 控制图"));
    xBarChart->setAnimationOptions(QChart::SeriesAnimations);
    m_xBarChart->setChart(xBarChart);

    // 创建R控制图
    auto* rChart = new QChart();
    rChart->setTitle(tr("R 控制图"));
    rChart->setAnimationOptions(QChart::SeriesAnimations);
    m_rChart->setChart(rChart);

    // 创建直方图
    auto* histChart = new QChart();
    histChart->setTitle(tr("数据分布直方图"));
    histChart->setAnimationOptions(QChart::SeriesAnimations);
    m_histogramChart->setChart(histChart);
}

void SPCView::updateXBarChart() {
    auto* chart = m_xBarChart->chart();
    chart->removeAllSeries();

    if (m_data.empty()) return;

    const auto& dataToUse = m_filteredData.empty() ? m_data : m_filteredData;

    // 数据点序列
    auto* dataSeries = new QLineSeries();
    dataSeries->setName(tr("测量值"));

    // 控制限序列
    auto* uclSeries = new QLineSeries();
    uclSeries->setName(tr("UCL"));
    uclSeries->setPen(QPen(Qt::red, 2, Qt::DashLine));

    auto* clSeries = new QLineSeries();
    clSeries->setName(tr("CL"));
    clSeries->setPen(QPen(Qt::green, 2));

    auto* lclSeries = new QLineSeries();
    lclSeries->setName(tr("LCL"));
    lclSeries->setPen(QPen(Qt::red, 2, Qt::DashLine));

    // 添加数据点
    int index = 0;
    for (const auto& point : dataToUse) {
        dataSeries->append(index, point.value);
        uclSeries->append(index, m_statistics.ucl);
        clSeries->append(index, m_statistics.cl);
        lclSeries->append(index, m_statistics.lcl);
        index++;
    }

    chart->addSeries(dataSeries);
    chart->addSeries(uclSeries);
    chart->addSeries(clSeries);
    chart->addSeries(lclSeries);

    // 创建坐标轴
    auto* axisX = new QValueAxis();
    axisX->setTitleText(tr("样本编号"));
    axisX->setLabelFormat("%d");
    chart->addAxis(axisX, Qt::AlignBottom);

    auto* axisY = new QValueAxis();
    axisY->setTitleText(tr("测量值"));
    chart->addAxis(axisY, Qt::AlignLeft);

    dataSeries->attachAxis(axisX);
    dataSeries->attachAxis(axisY);
    uclSeries->attachAxis(axisX);
    uclSeries->attachAxis(axisY);
    clSeries->attachAxis(axisX);
    clSeries->attachAxis(axisY);
    lclSeries->attachAxis(axisX);
    lclSeries->attachAxis(axisY);
}

void SPCView::updateRChart() {
    auto* chart = m_rChart->chart();
    chart->removeAllSeries();

    if (m_data.size() < m_subgroupSize) return;

    const auto& dataToUse = m_filteredData.empty() ? m_data : m_filteredData;

    // 计算极差
    auto* rangeSeries = new QLineSeries();
    rangeSeries->setName(tr("极差"));

    std::vector<double> ranges;
    for (size_t i = 0; i <= dataToUse.size() - m_subgroupSize; i += m_subgroupSize) {
        double minVal = dataToUse[i].value;
        double maxVal = dataToUse[i].value;

        for (int j = 1; j < m_subgroupSize; ++j) {
            minVal = std::min(minVal, dataToUse[i + j].value);
            maxVal = std::max(maxVal, dataToUse[i + j].value);
        }

        double range = maxVal - minVal;
        ranges.push_back(range);
        rangeSeries->append(ranges.size() - 1, range);
    }

    // 计算R图的控制限
    double rBar = std::accumulate(ranges.begin(), ranges.end(), 0.0) / ranges.size();

    // D3和D4常数（子组大小为5时）
    double D3 = 0.0;  // 子组大小5时的D3
    double D4 = 2.114; // 子组大小5时的D4

    double rUCL = D4 * rBar;
    double rLCL = D3 * rBar;

    // 添加控制限线
    auto* uclSeries = new QLineSeries();
    uclSeries->setName(tr("UCL"));
    uclSeries->setPen(QPen(Qt::red, 2, Qt::DashLine));

    auto* clSeries = new QLineSeries();
    clSeries->setName(tr("CL"));
    clSeries->setPen(QPen(Qt::green, 2));

    for (size_t i = 0; i < ranges.size(); ++i) {
        uclSeries->append(i, rUCL);
        clSeries->append(i, rBar);
    }

    chart->addSeries(rangeSeries);
    chart->addSeries(uclSeries);
    chart->addSeries(clSeries);

    // 创建坐标轴
    auto* axisX = new QValueAxis();
    axisX->setTitleText(tr("子组编号"));
    axisX->setLabelFormat("%d");
    chart->addAxis(axisX, Qt::AlignBottom);

    auto* axisY = new QValueAxis();
    axisY->setTitleText(tr("极差"));
    chart->addAxis(axisY, Qt::AlignLeft);

    rangeSeries->attachAxis(axisX);
    rangeSeries->attachAxis(axisY);
    uclSeries->attachAxis(axisX);
    uclSeries->attachAxis(axisY);
    clSeries->attachAxis(axisX);
    clSeries->attachAxis(axisY);
}

void SPCView::updateHistogram() {
    auto* chart = m_histogramChart->chart();
    chart->removeAllSeries();

    if (m_data.empty()) return;

    const auto& dataToUse = m_filteredData.empty() ? m_data : m_filteredData;

    // 计算直方图的组数和组宽
    double minVal = std::min_element(dataToUse.begin(), dataToUse.end(),
        [](const SPCDataPoint& a, const SPCDataPoint& b) { return a.value < b.value; })->value;
    double maxVal = std::max_element(dataToUse.begin(), dataToUse.end(),
        [](const SPCDataPoint& a, const SPCDataPoint& b) { return a.value < b.value; })->value;

    int binCount = std::min(20, std::max(5, (int)std::sqrt(dataToUse.size())));
    double binWidth = (maxVal - minVal) / binCount;

    // 统计每个组的频数
    std::vector<int> frequencies(binCount, 0);
    for (const auto& point : dataToUse) {
        int binIndex = std::min((int)((point.value - minVal) / binWidth), binCount - 1);
        frequencies[binIndex]++;
    }

    // 创建柱状图
    auto* barSet = new QBarSet(tr("频数"));
    QStringList categories;

    for (int i = 0; i < binCount; ++i) {
        *barSet << frequencies[i];
        double binStart = minVal + i * binWidth;
        categories << QString::number(binStart, 'f', 2);
    }

    auto* barSeries = new QBarSeries();
    barSeries->append(barSet);

    chart->addSeries(barSeries);
    chart->setTitle(tr("数据分布直方图"));

    // 创建坐标轴
    auto* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText(tr("测量值"));
    chart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    auto* axisY = new QValueAxis();
    axisY->setTitleText(tr("频数"));
    chart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);
}

void SPCView::updateCapabilityIndicators() {
    if (!m_hasSpecLimits || m_data.empty()) {
        m_cpLabel->setText("--");
        m_cpkLabel->setText("--");
        m_ppLabel->setText("--");
        m_ppkLabel->setText("--");
        return;
    }

    m_cpLabel->setText(QString::number(m_statistics.cp, 'f', 3));
    m_cpkLabel->setText(QString::number(m_statistics.cpk, 'f', 3));
    m_ppLabel->setText(QString::number(m_statistics.pp, 'f', 3));
    m_ppkLabel->setText(QString::number(m_statistics.ppk, 'f', 3));

    // 根据Cpk值设置颜色
    QString cpkStyle;
    if (m_statistics.cpk >= 1.33) {
        cpkStyle = "color: #4CAF50; font-weight: bold;"; // 绿色 - 良好
    } else if (m_statistics.cpk >= 1.0) {
        cpkStyle = "color: #ff9800; font-weight: bold;"; // 橙色 - 可接受
    } else {
        cpkStyle = "color: #f44336; font-weight: bold;"; // 红色 - 需改进
    }
    m_cpkLabel->setStyleSheet(cpkStyle);
}

void SPCView::calculateStatistics() {
    const auto& dataToUse = m_filteredData.empty() ? m_data : m_filteredData;

    if (dataToUse.empty()) {
        m_statistics = SPCStatistics{0, 0, 0, 0, 0, 0, 0, 0, 0};
        m_meanLabel->setText("--");
        m_stdDevLabel->setText("--");
        m_uclLabel->setText("--");
        m_clLabel->setText("--");
        m_lclLabel->setText("--");
        return;
    }

    // 计算均值
    double sum = 0;
    for (const auto& point : dataToUse) {
        sum += point.value;
    }
    m_statistics.mean = sum / dataToUse.size();
    m_statistics.cl = m_statistics.mean;

    // 计算标准差（总体标准差）
    double sumSquares = 0;
    for (const auto& point : dataToUse) {
        double diff = point.value - m_statistics.mean;
        sumSquares += diff * diff;
    }
    m_statistics.stdDev = std::sqrt(sumSquares / dataToUse.size());

    // 计算控制限（3σ）
    m_statistics.ucl = m_statistics.mean + 3 * m_statistics.stdDev;
    m_statistics.lcl = m_statistics.mean - 3 * m_statistics.stdDev;

    // 如果有规格限，计算过程能力指标
    if (m_hasSpecLimits) {
        double tolerance = m_usl - m_lsl;

        // Cp - 潜在过程能力指数
        m_statistics.cp = tolerance / (6 * m_statistics.stdDev);

        // Cpk - 实际过程能力指数
        double cpupper = (m_usl - m_statistics.mean) / (3 * m_statistics.stdDev);
        double cplower = (m_statistics.mean - m_lsl) / (3 * m_statistics.stdDev);
        m_statistics.cpk = std::min(cpupper, cplower);

        // Pp和Ppk（使用总体标准差）
        m_statistics.pp = m_statistics.cp;  // 简化处理
        m_statistics.ppk = m_statistics.cpk; // 简化处理
    }

    // 更新显示
    m_meanLabel->setText(QString::number(m_statistics.mean, 'f', 3));
    m_stdDevLabel->setText(QString::number(m_statistics.stdDev, 'f', 3));
    m_uclLabel->setText(QString::number(m_statistics.ucl, 'f', 3));
    m_clLabel->setText(QString::number(m_statistics.cl, 'f', 3));
    m_lclLabel->setText(QString::number(m_statistics.lcl, 'f', 3));
}

void SPCView::checkRules() {
    m_alarmList->clear();

    if (checkRule1()) {
        addAlarm(tr("规则1违反: 存在超出3σ限制的点"), "critical");
    }

    if (checkRule2()) {
        addAlarm(tr("规则2违反: 连续9个点在中心线同一侧"), "warning");
    }

    if (checkRule3()) {
        addAlarm(tr("规则3违反: 连续6个点递增或递减"), "warning");
    }

    if (checkRule4()) {
        addAlarm(tr("规则4违反: 连续14个点交替上下"), "info");
    }
}

bool SPCView::checkRule1() {
    // 检查是否有点超出3σ限制
    const auto& dataToUse = m_filteredData.empty() ? m_data : m_filteredData;

    for (const auto& point : dataToUse) {
        if (point.value > m_statistics.ucl || point.value < m_statistics.lcl) {
            return true;
        }
    }
    return false;
}

bool SPCView::checkRule2() {
    // 检查连续9个点是否在中心线同一侧
    const auto& dataToUse = m_filteredData.empty() ? m_data : m_filteredData;

    if (dataToUse.size() < 9) return false;

    int consecutiveAbove = 0;
    int consecutiveBelow = 0;

    for (const auto& point : dataToUse) {
        if (point.value > m_statistics.cl) {
            consecutiveAbove++;
            consecutiveBelow = 0;
        } else if (point.value < m_statistics.cl) {
            consecutiveBelow++;
            consecutiveAbove = 0;
        }

        if (consecutiveAbove >= 9 || consecutiveBelow >= 9) {
            return true;
        }
    }
    return false;
}

bool SPCView::checkRule3() {
    // 检查连续6个点是否递增或递减
    const auto& dataToUse = m_filteredData.empty() ? m_data : m_filteredData;

    if (dataToUse.size() < 6) return false;

    int increasing = 1;
    int decreasing = 1;

    for (size_t i = 1; i < dataToUse.size(); ++i) {
        if (dataToUse[i].value > dataToUse[i-1].value) {
            increasing++;
            decreasing = 1;
        } else if (dataToUse[i].value < dataToUse[i-1].value) {
            decreasing++;
            increasing = 1;
        } else {
            increasing = 1;
            decreasing = 1;
        }

        if (increasing >= 6 || decreasing >= 6) {
            return true;
        }
    }
    return false;
}

bool SPCView::checkRule4() {
    // 检查连续14个点是否交替上下
    const auto& dataToUse = m_filteredData.empty() ? m_data : m_filteredData;

    if (dataToUse.size() < 14) return false;

    int alternating = 1;
    bool lastUp = false;

    for (size_t i = 1; i < dataToUse.size(); ++i) {
        bool currentUp = dataToUse[i].value > dataToUse[i-1].value;

        if (i == 1) {
            lastUp = currentUp;
            alternating = 2;
        } else if (currentUp != lastUp) {
            alternating++;
            lastUp = currentUp;
        } else {
            alternating = 1;
        }

        if (alternating >= 14) {
            return true;
        }
    }
    return false;
}

void SPCView::addAlarm(const QString& message, const QString& type) {
    auto* item = new QListWidgetItem(message);

    if (type == "critical") {
        item->setForeground(QBrush(QColor("#f44336")));
        item->setIcon(QIcon(":/icons/error.svg"));
    } else if (type == "warning") {
        item->setForeground(QBrush(QColor("#ff9800")));
        item->setIcon(QIcon(":/icons/warning.svg"));
    } else {
        item->setForeground(QBrush(QColor("#2196F3")));
        item->setIcon(QIcon(":/icons/info.svg"));
    }

    m_alarmList->addItem(item);
    emit alarmTriggered(message);
}
