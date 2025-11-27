#include "mainwindow.h"
#include "widgets/ImageView.h"
#include "widgets/ParamPanel.h"
#include "widgets/ResultCard.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent) : QMainWindow{parent} {
  setupUI();
}

MainWindow::~MainWindow()
{

}

void MainWindow::onStartClicked()
{
  emit startRequested();
  m_actionStart->setEnabled(false);
  m_actionStop->setEnabled(true);
}

void MainWindow::onStopClicked()
{
  emit stopRequested();
  m_actionStart->setEnabled(true);
  m_actionStop->setEnabled(false);
}

void MainWindow::onSingleShotClicked()
{
  emit singleShotRequested();
}

void MainWindow::onSettingsClicked()
{
    // TODO: Show Settings Dialog
}

void MainWindow::onStatisticsClicked()
{
    // TODO: Show Statistics View
}

void MainWindow::onResultReady(const DetectResult &result)
{
    // TODO: Update ResultCard and StatusBar
}

void MainWindow::onFrameReady(const cv::Mat &frame)
{
    if (m_imageView) {
        m_imageView->setImage(frame);
    }
}

void MainWindow::onError(const QString &module, const QString &message)
{
    statusBar()->showMessage(QString("[%1] %2").arg(module, message), 3000);
}

void MainWindow::updateStatistics()
{
    // TODO: Update stats labels
}

void MainWindow::setupUI()
{
  // 窗口属性
  setWindowTitle(tr("缺陷检测系统"));
  resize(1280, 800);
  
  // 设置样式
  setStyleSheet(R"(
    QMainWindow {
        background-color: #f3f4f6; /* bg-gray-100 */
    }
    QMenuBar {
        background-color: #1f2937; /* bg-gray-800 */
        color: white;
        border-bottom: 1px solid #374151; /* border-gray-700 */
    }
    QMenuBar::item {
        background-color: transparent;
        padding: 8px 12px;
        margin: 0px;
        border-radius: 4px;
    }
    QMenuBar::item:selected {
        background-color: #374151; /* hover:bg-gray-700 */
    }
    QToolBar {
        background-color: #f3f4f6; /* bg-gray-100 */
        border-bottom: 1px solid #d1d5db; /* border-gray-300 */
        spacing: 12px;
        padding: 12px;
    }
    QToolButton {
        background-color: transparent;
        border-radius: 4px;
        padding: 8px 16px;
        color: white;
        font-weight: bold;
    }
    QStatusBar {
        background-color: #1f2937; /* bg-gray-800 */
        color: white;
        border-top: 1px solid #374151; /* border-gray-700 */
    }
    QLabel {
        color: #1f2937;
    }
    QStatusBar QLabel {
        color: white;
        padding: 0 10px;
    }
  )");

  // 中心组件
  m_centralWidget = new QWidget(this);
  setCentralWidget(m_centralWidget);

  auto* mainLayout = new QHBoxLayout(m_centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // 左侧：图像显示区 (Flex-1)
  // ImagePreview container to match padding/margins if needed, 
  // but design says "flex-1 min-w-0" inside a "flex-1 flex overflow-hidden"
  m_imageView = new ImageView(this);
  mainLayout->addWidget(m_imageView, 1);

  // 右侧：结果 + 参数面板 (w-96 = 384px)
  m_rightPanel = new QWidget(this);
  m_rightPanel->setObjectName("rightPanel");
  m_rightPanel->setFixedWidth(384);
  m_rightPanel->setStyleSheet("#rightPanel { background-color: white; border-left: 1px solid #d1d5db; }");

  auto* rightLayout = new QVBoxLayout(m_rightPanel);
  rightLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setSpacing(0);

  // 检测结果卡片
  m_resultCard = new ResultCard(this);
  // ResultCard styles should be handled inside ResultCard or here
  rightLayout->addWidget(m_resultCard);

  // 参数面板
  m_paramPanel = new ParamPanel(this);
  rightLayout->addWidget(m_paramPanel, 1);

  mainLayout->addWidget(m_rightPanel);

  setupMenuBar();
  setupToolBar();
  setupStatusBar();
  setupConnections();
}

void MainWindow::setupMenuBar()
{
    // Header: File, Settings, Help
    auto* menuBar = this->menuBar();
    
    // File
    auto* fileMenu = menuBar->addMenu(tr("文件"));
    m_actionFile = fileMenu->menuAction();
    
    // Settings (Header)
    auto* settingsMenu = menuBar->addMenu(tr("设置"));
    // Note: Design has "Settings" button in Header AND "Parameters" button in Toolbar.
    
    // Help
    auto* helpMenu = menuBar->addMenu(tr("帮助"));
    m_actionHelp = helpMenu->menuAction();
}

void MainWindow::setupToolBar()
{
    auto* toolBar = addToolBar(tr("Main Toolbar"));
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // Start
    m_actionStart = new QAction(QIcon(":/icons/play.svg"), tr("启动"), this);
    m_actionStart->setToolTip(tr("开始检测"));
    toolBar->addAction(m_actionStart);
    // Custom style for Start button (Green)
    auto* startBtn = toolBar->widgetForAction(m_actionStart);
    if(startBtn) startBtn->setStyleSheet("background-color: #16a34a; color: white;"); // green-600

    // Stop
    m_actionStop = new QAction(QIcon(":/icons/square.svg"), tr("停止"), this);
    m_actionStop->setToolTip(tr("停止检测"));
    m_actionStop->setEnabled(false);
    toolBar->addAction(m_actionStop);
    auto* stopBtn = toolBar->widgetForAction(m_actionStop);
    if(stopBtn) stopBtn->setStyleSheet("background-color: #dc2626; color: white;"); // red-600

    // Single Shot
    m_actionSingleShot = new QAction(QIcon(":/icons/camera.svg"), tr("单拍"), this);
    toolBar->addAction(m_actionSingleShot);
    auto* shotBtn = toolBar->widgetForAction(m_actionSingleShot);
    if(shotBtn) shotBtn->setStyleSheet("background-color: #2563eb; color: white;"); // blue-600

    // Parameters
    m_actionSettings = new QAction(QIcon(":/icons/settings.svg"), tr("参数"), this);
    toolBar->addAction(m_actionSettings);
    auto* setBtn = toolBar->widgetForAction(m_actionSettings);
    if(setBtn) setBtn->setStyleSheet("background-color: #4b5563; color: white;"); // gray-600

    // Statistics
    m_actionStatistics = new QAction(QIcon(":/icons/chart.svg"), tr("统计"), this);
    toolBar->addAction(m_actionStatistics);
    auto* statBtn = toolBar->widgetForAction(m_actionStatistics);
    if(statBtn) statBtn->setStyleSheet("background-color: #9333ea; color: white;"); // purple-600
}

void MainWindow::setupStatusBar()
{
    auto* statusBar = this->statusBar();

    // Detection Speed
    m_speedLabel = new QLabel(tr("检测速度: 0ms"));
    m_speedLabel->setStyleSheet("color: white;");
    statusBar->addWidget(m_speedLabel);

    statusBar->addWidget(new QLabel("|"));

    // Total
    m_totalCountLabel = new QLabel(tr("总数: 0"));
    m_totalCountLabel->setStyleSheet("color: #60a5fa;"); // blue-400
    statusBar->addWidget(m_totalCountLabel);

    // OK
    m_okCountLabel = new QLabel(tr("OK: 0"));
    m_okCountLabel->setStyleSheet("color: #4ade80;"); // green-400
    statusBar->addWidget(m_okCountLabel);

    // NG
    m_ngCountLabel = new QLabel(tr("NG: 0"));
    m_ngCountLabel->setStyleSheet("color: #f87171;"); // red-400
    statusBar->addWidget(m_ngCountLabel);

    // Yield
    m_yieldLabel = new QLabel(tr("良率: 0.0%"));
    m_yieldLabel->setStyleSheet("color: #facc15;"); // yellow-400
    statusBar->addWidget(m_yieldLabel);
}

void MainWindow::setupConnections()
{
    connect(m_actionStart, &QAction::triggered, this, &MainWindow::onStartClicked);
    connect(m_actionStop, &QAction::triggered, this, &MainWindow::onStopClicked);
    connect(m_actionSingleShot, &QAction::triggered, this, &MainWindow::onSingleShotClicked);
    connect(m_actionSettings, &QAction::triggered, this, &MainWindow::onSettingsClicked);
    connect(m_actionStatistics, &QAction::triggered, this, &MainWindow::onStatisticsClicked);
}
