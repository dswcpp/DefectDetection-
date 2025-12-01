#include "mainwindow.h"
#include "widgets/ImageView.h"
#include "widgets/ImageViewControls.h"
#include "widgets/ParamPanel.h"
#include "widgets/ResultCard.h"
#include "widgets/AnnotationPanel.h"
#include "dialogs/SettingsDialog.h"
#include "dialogs/StatisticsDialog.h"
#include "dialogs/HistoryDialog.h"
#include "dialogs/AboutDialog.h"
#include "DetectPipeline.h"
#include "Types.h"
#include "config/ConfigManager.h"
#include "data/DatabaseManager.h"
#include "data/repositories/DefectRepository.h"
#include "Logger.h"
#include <spdlog/spdlog.h>
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
#include <QFileDialog>
#include <QTimer>
#include <QDateTime>
#include <QFileInfo>
#include <QImage>
#include <QDebug>
#include <QTabWidget>
#include <algorithm>
#include "common/Logger.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow{parent} {
  setupUI();

  // 初始化数据库
  m_dbManager = new DatabaseManager(this);
  if (m_dbManager->initFromConfig()) {
    // 执行 schema 初始化表结构
    QString schemaPath = QCoreApplication::applicationDirPath() + "/config/schema.sql";
    if (QFile::exists(schemaPath)) {
      m_dbManager->executeSchema(schemaPath);
    }
    LOG_INFO("Database initialized successfully");
  } else {
    LOG_WARN("Failed to initialize database");
  }
}

MainWindow::~MainWindow()
{
}

void MainWindow::setPipeline(DetectPipeline* pipeline)
{
  m_pipeline = pipeline;
  if (!m_pipeline) return;

  // 连接流水线信号
  connect(m_pipeline, &DetectPipeline::frameReady, this, &MainWindow::onFrameReady);
  connect(m_pipeline, &DetectPipeline::resultReady, this, &MainWindow::onResultReady);
  connect(m_pipeline, &DetectPipeline::error, this, &MainWindow::onError);
  connect(m_pipeline, &DetectPipeline::started, this, [this]() {
    m_actionStart->setEnabled(false);
    m_actionStop->setEnabled(true);
    m_actionSingleShot->setEnabled(false);
    statusBar()->showMessage(tr("检测已启动"), 2000);
  });
  connect(m_pipeline, &DetectPipeline::stopped, this, [this]() {
    m_actionStart->setEnabled(true);
    m_actionStop->setEnabled(false);
    m_actionSingleShot->setEnabled(true);
    statusBar()->showMessage(tr("检测已停止"), 2000);
  });
}

void MainWindow::onStartClicked()
{
  LOG_INFO("MainWindow::onStartClicked() - 启动连续检测");

  if (!m_pipeline) {
    LOG_WARN("Pipeline not set");
    return;
  }

  m_pipeline->start();
}

void MainWindow::onStopClicked()
{
  if (m_pipeline) {
    m_pipeline->stop();
  }
}

void MainWindow::onSingleShotClicked()
{
  LOG_INFO("MainWindow::onSingleShotClicked() - 单拍模式");

  if (!m_pipeline) {
    LOG_WARN("Pipeline not set");
    return;
  }

  m_pipeline->singleShot();
}

void MainWindow::onSettingsClicked()
{
    SettingsDialog dialog(this);

    // 监听配置变更信号，动态更新 Pipeline
    connect(&dialog, &SettingsDialog::settingsChanged, this, [this]() {
        if (m_pipeline) {
            auto camCfg = gConfig.cameraConfig();
            m_pipeline->setImageDir(camCfg.imageDir);
            m_pipeline->setCaptureInterval(camCfg.captureIntervalMs);
            LOG_INFO("Pipeline updated: imageDir={}, interval={}ms",
                     camCfg.imageDir, camCfg.captureIntervalMs);
            statusBar()->showMessage(tr("配置已更新"), 2000);
        }
    });

    dialog.exec();
}

void MainWindow::onStatisticsClicked()
{
    StatisticsDialog dialog(m_dbManager, this);
    dialog.exec();
}

void MainWindow::onHistoryClicked()
{
    HistoryDialog dialog(m_dbManager, this);
    dialog.exec();
}

void MainWindow::onResultReady(const DetectResult &result)
{
    ++m_totalCount;
    if (result.isOK) {
        ++m_okCount;
    } else {
        ++m_ngCount;
    }

    // 更新节拍时间
    m_lastCycleTimeMs = result.cycleTimeMs;
    if (m_cycleTimeLabel) {
        m_cycleTimeLabel->setText(tr("节拍: %1 ms").arg(result.cycleTimeMs));
    }

    updateStatistics();

    // 更新结果卡片
    if (m_resultCard) {
      m_resultCard->setResult(result);
    }

    // 绘制检测框
    if (m_imageView) {
      if (!result.isOK && !result.defects.empty()) {
        QVector<DetectionBox> boxes;
        for (const auto& defect : result.defects) {
          DetectionBox box;
          box.rect = defect.bbox;
          box.label = result.defectType;
          box.confidence = defect.confidence;

          if (result.level == SeverityLevel::Minor) {
            box.color = QColor(255, 193, 7);  // 黄色
          } else if (result.level == SeverityLevel::Major) {
            box.color = QColor(255, 152, 0);  // 橙色
          } else {
            box.color = QColor(244, 67, 54);  // 红色
          }
          boxes.append(box);
        }
        m_imageView->drawDetectionBoxes(boxes);
      } else {
        m_imageView->clearDetectionBoxes();
      }
    }

    // 保存检测结果到数据库
    if (m_dbManager && m_dbManager->isOpen()) {
      auto* repo = m_dbManager->defectRepository();
      if (repo) {
        QString imagePath = m_pipeline ? m_pipeline->currentImagePath() : QString();
        qint64 id = repo->insertFromResult(result, imagePath);
        if (id > 0) {
          LOG_DEBUG("Inspection saved to database, id={}", id);
        } else {
          LOG_WARN("Failed to save inspection to database");
        }
      }
    }

    statusBar()->showMessage(result.isOK ? tr("最近一次结果: OK") : tr("最近一次结果: NG"), 2000);
}

void MainWindow::onFrameReady(const cv::Mat &frame)
{
    // 更新帧率统计
    m_fpsCounter.tick();

    if (m_imageView) {
        m_imageView->setImage(frame);
    }

    // 限制状态栏更新频率
    if (m_statusThrottle.check() && m_fpsLabel) {
        m_fpsLabel->setText(tr("FPS: %1").arg(m_fpsCounter.fps(), 0, 'f', 1));
    }
}

void MainWindow::onError(const QString &module, const QString &message)
{
    statusBar()->showMessage(QString("[%1] %2").arg(module, message), 3000);
}

void MainWindow::updateStatistics()
{
    if (m_totalCountLabel) {
        m_totalCountLabel->setText(tr("总数: %1").arg(m_totalCount));
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
        m_yieldLabel->setText(tr("良率: %1%").arg(yield, 0, 'f', 1));
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

  // 创建选项卡控件来容纳参数面板和标注面板
  auto* tabWidget = new QTabWidget(this);
  tabWidget->setObjectName(QStringLiteral("rightTabWidget"));

  // 参数面板
  m_paramPanel = new ParamPanel(this);
  tabWidget->addTab(m_paramPanel, tr("检测参数"));

  // 标注面板
  m_annotationPanel = new AnnotationPanel(this);
  m_annotationPanel->setImageView(m_imageView);
  tabWidget->addTab(m_annotationPanel, tr("缺陷标注"));

  rightLayout->addWidget(tabWidget, 1);

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
  m_actionHistory = new QAction(QIcon(":/icons/history.svg"), tr("历史"), this);
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
  settingsMenu->addAction(m_actionHistory);
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

  toolBar->addAction(m_actionHistory);
  QToolButton* historyBtn = qobject_cast<QToolButton*>(
      toolBar->widgetForAction(m_actionHistory));
  if (historyBtn) {
    historyBtn->setObjectName(QStringLiteral("historyButton"));
  }
}

void MainWindow::setupStatusBar()
{
  auto* bar = statusBar();

  // FPS 显示
  m_fpsLabel = new QLabel(tr("FPS: --"));
  m_fpsLabel->setMinimumWidth(80);
  m_fpsLabel->setStyleSheet(QLatin1String("color: #1976d2; font-weight: bold;"));
  bar->addWidget(m_fpsLabel);
  bar->addWidget(new QLabel(QLatin1String("|")));

  // 节拍时间
  m_cycleTimeLabel = new QLabel(tr("节拍: -- ms"));
  m_cycleTimeLabel->setMinimumWidth(120);
  bar->addWidget(m_cycleTimeLabel);
  bar->addWidget(new QLabel(QLatin1String("|")));

  // 统计信息
  m_totalCountLabel = new QLabel(tr("总数: 0"));
  bar->addWidget(m_totalCountLabel);
  m_okCountLabel = new QLabel(tr("OK: 0"));
  m_okCountLabel->setStyleSheet(QLatin1String("color: #2e7d32;"));
  bar->addWidget(m_okCountLabel);
  m_ngCountLabel = new QLabel(tr("NG: 0"));
  m_ngCountLabel->setStyleSheet(QLatin1String("color: #c62828;"));
  bar->addWidget(m_ngCountLabel);
  bar->addWidget(new QLabel(QLatin1String("|")));

  // 良率
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
    connect(m_actionHistory, &QAction::triggered, this, &MainWindow::onHistoryClicked);
    connect(m_actionExit, &QAction::triggered, this, &QWidget::close);
    connect(m_actionAbout, &QAction::triggered, this, [this]() {
        AboutDialog dialog(this);
        dialog.exec();
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
