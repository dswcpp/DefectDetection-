#include "mainwindow.h"
#include "widgets/ImageView.h"
#include "widgets/ImageViewControls.h"
#include "widgets/ParamPanel.h"
#include "widgets/ResultCard.h"
#include "dialogs/SettingsDialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QToolBar>
#include <QToolButton>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QKeySequence>
#include <QLatin1String>
#include <QMessageBox>
#include <QSize>
#include <QFile>

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
    SettingsDialog dialog(this);
    dialog.exec();

    // 如果设置有更改，可以在这里重新加载配置
    // if (dialog.result() == QDialog::Accepted) {
    //     // 重新加载配置
    // }
}

void MainWindow::onStatisticsClicked()
{
    // TODO: Show Statistics View
}

void MainWindow::onResultReady(const DetectResult &result)
{
    ++m_totalCount;
    if (result.isOK) {
        ++m_okCount;
    } else {
        ++m_ngCount;
    }
    updateStatistics();
    statusBar()->showMessage(result.isOK ? tr("���һ�����: OK") : tr("���һ�����: NG"), 2000);
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
    if (m_totalCountLabel) {
        m_totalCountLabel->setText(tr("����: %1").arg(m_totalCount));
    }
    if (m_okCountLabel) {
        m_okCountLabel->setText(tr("OK: %1").arg(m_okCount));
    }
    if (m_ngCountLabel) {
        m_ngCountLabel->setText(tr("NG: %1").arg(m_ngCount));
    }
    if (m_yieldLabel) {
        const double yield =
            m_totalCount > 0 ? static_cast<double>(m_okCount) / m_totalCount * 100.0 : 0.0;
        m_yieldLabel->setText(tr("����: %1%").arg(yield, 0, 'f', 1));
    }
}

void MainWindow::setupUI()
{
  setWindowTitle(tr("缺陷检测系统 v1.0"));
  setMinimumSize(1024, 768);
  resize(1280, 800);

  // 加载深色主题样式表
  QFile styleFile(QStringLiteral(":/styles/dark-theme.qss"));
  if (!styleFile.exists()) {
    // 如果资源文件不存在，尝试从文件系统加载
    styleFile.setFileName(QStringLiteral("resources/styles/dark-theme.qss"));
  }

  if (styleFile.open(QFile::ReadOnly)) {
    QString styleSheet = QLatin1String(styleFile.readAll());
    setStyleSheet(styleSheet);
    styleFile.close();
  }

  m_centralWidget = new QWidget(this);
  m_centralWidget->setObjectName(QStringLiteral("centralWidget"));
  setCentralWidget(m_centralWidget);

  auto* mainLayout = new QHBoxLayout(m_centralWidget);
  mainLayout->setContentsMargins(8, 8, 8, 8);
  mainLayout->setSpacing(8);

  // 左侧：图像显示区域及其控件
  m_imageViewContainer = new QWidget(this);
  auto* imageLayout = new QVBoxLayout(m_imageViewContainer);
  imageLayout->setContentsMargins(0, 0, 0, 0);
  imageLayout->setSpacing(4);

  // 图像显示区域
  m_imageView = new ImageView(this);
  m_imageView->setMinimumSize(800, 600);
  imageLayout->addWidget(m_imageView, 1);

  // 图像控制栏
  m_imageViewControls = new ImageViewControls(this);
  imageLayout->addWidget(m_imageViewControls);

  mainLayout->addWidget(m_imageViewContainer, 70);  // 占70%宽度

  // 右侧面板
  m_rightPanel = new QWidget(this);
  m_rightPanel->setObjectName(QStringLiteral("rightPanel"));
  auto* rightLayout = new QVBoxLayout(m_rightPanel);
  rightLayout->setContentsMargins(8, 8, 8, 8);
  rightLayout->setSpacing(12);

  // 结果卡片
  m_resultCard = new ResultCard(this);
  m_resultCard->setMinimumHeight(220);  // 增加高度以显示更多信息
  m_resultCard->setMaximumHeight(280);
  rightLayout->addWidget(m_resultCard);

  // 参数面板 - 不再使用QGroupBox，直接使用改进后的ParamPanel
  m_paramPanel = new ParamPanel(this);
  rightLayout->addWidget(m_paramPanel, 1);

  m_rightPanel->setMinimumWidth(350);
  m_rightPanel->setMaximumWidth(400);
  mainLayout->addWidget(m_rightPanel, 30);

  createActions();
  setupMenuBar();
  setupToolBar();
  setupStatusBar();
  setupConnections();
}



void MainWindow::createActions()
{
  m_actionStart = new QAction(QIcon(":/icons/play.svg"), tr("启动"), this);
  m_actionStart->setShortcut(QKeySequence(QStringLiteral("F5")));
  m_actionStop = new QAction(QIcon(":/icons/stop.svg"), tr("停止"), this);
  m_actionStop->setShortcut(QKeySequence(QStringLiteral("F6")));
  m_actionStop->setEnabled(false);
  m_actionSingleShot = new QAction(QIcon(":/icons/camera.svg"), tr("单拍"), this);
  m_actionSingleShot->setShortcut(QKeySequence(QStringLiteral("F7")));
  m_actionSettings = new QAction(QIcon(":/icons/settings.svg"), tr("参数"), this);
  m_actionStatistics = new QAction(QIcon(":/icons/chart.svg"), tr("统计"), this);
  m_actionExit = new QAction(tr("退出"), this);
  m_actionExit->setShortcut(QKeySequence::Quit);
  m_actionAbout = new QAction(tr("关于"), this);
}



void MainWindow::setupMenuBar()
{
  auto* bar = this->menuBar();
  bar->clear();
  auto* fileMenu = bar->addMenu(tr("文件"));
  fileMenu->addAction(m_actionStart);
  fileMenu->addAction(m_actionStop);
  fileMenu->addAction(m_actionSingleShot);
  fileMenu->addSeparator();
  fileMenu->addAction(m_actionExit);
  auto* settingsMenu = bar->addMenu(tr("设置"));
  settingsMenu->addAction(m_actionSettings);
  settingsMenu->addAction(m_actionStatistics);
  auto* helpMenu = bar->addMenu(tr("帮助"));
  helpMenu->addAction(m_actionAbout);
}

void MainWindow::setupToolBar()
{
  auto* toolBar = addToolBar(tr("主工具栏"));
  toolBar->setObjectName(QStringLiteral("mainToolBar"));
  toolBar->setIconSize(QSize(24, 24));
  toolBar->setMovable(false);
  toolBar->setFloatable(false);
  toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

  // 添加动作并设置按钮的对象名称
  toolBar->addAction(m_actionStart);
  QToolButton* startBtn = qobject_cast<QToolButton*>(
      toolBar->widgetForAction(m_actionStart));
  if (startBtn) {
    startBtn->setObjectName(QStringLiteral("startButton"));
  }

  toolBar->addAction(m_actionStop);
  QToolButton* stopBtn = qobject_cast<QToolButton*>(
      toolBar->widgetForAction(m_actionStop));
  if (stopBtn) {
    stopBtn->setObjectName(QStringLiteral("stopButton"));
  }

  toolBar->addSeparator();

  toolBar->addAction(m_actionSingleShot);
  QToolButton* captureBtn = qobject_cast<QToolButton*>(
      toolBar->widgetForAction(m_actionSingleShot));
  if (captureBtn) {
    captureBtn->setObjectName(QStringLiteral("captureButton"));
  }

  toolBar->addSeparator();

  toolBar->addAction(m_actionSettings);
  QToolButton* settingsBtn = qobject_cast<QToolButton*>(
      toolBar->widgetForAction(m_actionSettings));
  if (settingsBtn) {
    settingsBtn->setObjectName(QStringLiteral("settingsButton"));
  }

  toolBar->addAction(m_actionStatistics);
  QToolButton* statsBtn = qobject_cast<QToolButton*>(
      toolBar->widgetForAction(m_actionStatistics));
  if (statsBtn) {
    statsBtn->setObjectName(QStringLiteral("statisticsButton"));
  }
}

void MainWindow::setupStatusBar()
{
  auto* bar = statusBar();
  m_cycleTimeLabel = new QLabel(tr("节拍: -- ms"));
  m_cycleTimeLabel->setMinimumWidth(120);
  bar->addWidget(m_cycleTimeLabel);
  bar->addWidget(new QLabel(QLatin1String("|")));
  m_totalCountLabel = new QLabel(tr("总数: 0"));
  bar->addWidget(m_totalCountLabel);
  m_okCountLabel = new QLabel(tr("OK: 0"));
  m_okCountLabel->setStyleSheet(QLatin1String("color: #2e7d32;"));
  bar->addWidget(m_okCountLabel);
  m_ngCountLabel = new QLabel(tr("NG: 0"));
  m_ngCountLabel->setStyleSheet(QLatin1String("color: #c62828;"));
  bar->addWidget(m_ngCountLabel);
  bar->addWidget(new QLabel(QLatin1String("|")));
  m_yieldLabel = new QLabel(tr("良率: 0.0%"));
  m_yieldLabel->setMinimumWidth(120);
  bar->addWidget(m_yieldLabel);
}

void MainWindow::setupConnections()
{
    connect(m_actionStart, &QAction::triggered, this, &MainWindow::onStartClicked);
    connect(m_actionStop, &QAction::triggered, this, &MainWindow::onStopClicked);
    connect(m_actionSingleShot, &QAction::triggered, this, &MainWindow::onSingleShotClicked);
    connect(m_actionSettings, &QAction::triggered, this, &MainWindow::onSettingsClicked);
    connect(m_actionStatistics, &QAction::triggered, this, &MainWindow::onStatisticsClicked);
    connect(m_actionExit, &QAction::triggered, this, &QWidget::close);
    connect(m_actionAbout, &QAction::triggered, this, [this]() {
        QMessageBox::information(this, tr("关于"), tr("缺陷检测系统 v1.0"));
    });

    // 连接ImageViewControls的信号
    if (m_imageViewControls && m_imageView) {
        // 显示模式切换
        connect(m_imageViewControls, &ImageViewControls::displayModeChanged,
                [this](int mode) {
                    m_imageView->setDisplayMode(mode == 0 ?
                        ImageView::DisplayMode::Original :
                        ImageView::DisplayMode::Annotated);
                });

        // ROI显示控制
        connect(m_imageViewControls, &ImageViewControls::showROIChanged,
                [this](bool show) {
                    m_imageView->enableROIEdit(show);
                });

        // 缩放控制
        connect(m_imageViewControls, &ImageViewControls::zoomInRequested,
                m_imageView, &ImageView::zoomIn);
        connect(m_imageViewControls, &ImageViewControls::zoomOutRequested,
                m_imageView, &ImageView::zoomOut);
        connect(m_imageViewControls, &ImageViewControls::zoomFitRequested,
                m_imageView, &ImageView::zoomFit);
        connect(m_imageViewControls, &ImageViewControls::zoomActualRequested,
                m_imageView, &ImageView::zoomActual);

        // 缩放变化反馈
        connect(m_imageView, &ImageView::zoomChanged,
                m_imageViewControls, &ImageViewControls::setZoomLevel);
    }
}
