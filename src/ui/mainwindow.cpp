#include "mainwindow.h"
#include "dialogs/SettingsDialog.h"
#include "views/StatisticsView.h"
#include "widgets/ImageView.h"
#include "widgets/ParamPanel.h"
#include "widgets/ResultCard.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QToolButton>
#include <QVariantMap>
#include <QSize>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow{parent}
{
  setupUI();
}

MainWindow::~MainWindow() = default;

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
}

void MainWindow::onStatisticsClicked()
{
  QDialog dialog(this);
  dialog.setWindowTitle(tr("统计看板"));
  dialog.resize(960, 600);
  auto* layout = new QVBoxLayout(&dialog);
  auto* statsView = new StatisticsView(&dialog);
  layout->addWidget(statsView);
  dialog.exec();
}

void MainWindow::onResultReady(const DetectResult& result)
{
  if (m_resultCard) {
    m_resultCard->setResult(result);
  }

  m_totalCount++;
  if (result.isOK) {
    m_okCount++;
  } else {
    m_ngCount++;
  }
  if (result.cycleTimeMs > 0) {
    m_lastCycleTimeMs = result.cycleTimeMs;
    if (m_cycleTimeLabel) {
      m_cycleTimeLabel->setText(tr("节拍: %1 ms").arg(result.cycleTimeMs));
    }
  }
  updateStatistics();

  statusBar()->showMessage(result.isOK ? tr("最新检测结果: OK") : tr("最新检测结果: NG"), 2000);
}

void MainWindow::onFrameReady(const cv::Mat& frame)
{
  if (m_imageView) {
    m_imageView->setImage(frame);
  }
}

void MainWindow::onError(const QString& module, const QString& message)
{
  statusBar()->showMessage(QStringLiteral("[%1] %2").arg(module, message), 3000);
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
    const double yield = m_totalCount > 0
                             ? static_cast<double>(m_okCount) / static_cast<double>(m_totalCount) * 100.0
                             : 0.0;
    m_yieldLabel->setText(tr("良率: %1%").arg(yield, 0, 'f', 1));
  }
}

void MainWindow::setupUI()
{
  setWindowTitle(tr("缺陷检测系统 v1.0"));
  setMinimumSize(1024, 768);
  resize(1360, 900);

  m_centralWidget = new QWidget(this);
  m_centralWidget->setObjectName(QStringLiteral("MainSurface"));
  setCentralWidget(m_centralWidget);

  auto* rootLayout = new QHBoxLayout(m_centralWidget);
  rootLayout->setContentsMargins(8, 8, 8, 8);
  rootLayout->setSpacing(8);

  auto* splitter = new QSplitter(Qt::Horizontal, m_centralWidget);
  splitter->setObjectName(QStringLiteral("MainSplitter"));
  rootLayout->addWidget(splitter);

  auto* imageContainer = new QWidget(splitter);
  imageContainer->setObjectName(QStringLiteral("ImagePreviewContainer"));
  auto* imageLayout = new QVBoxLayout(imageContainer);
  imageLayout->setContentsMargins(0, 0, 0, 0);
  imageLayout->setSpacing(0);

  m_imageView = new ImageView(imageContainer);
  m_imageView->setObjectName(QStringLiteral("ImagePreview"));
  m_imageView->setMinimumSize(640, 480);
  imageLayout->addWidget(m_imageView);

  splitter->addWidget(imageContainer);

  m_rightPanel = new QWidget(splitter);
  m_rightPanel->setObjectName(QStringLiteral("SidebarPanel"));
  auto* rightLayout = new QVBoxLayout(m_rightPanel);
  rightLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setSpacing(12);

  m_resultCard = new ResultCard(m_rightPanel);
  rightLayout->addWidget(m_resultCard);

  auto* paramGroup = new QGroupBox(tr("参数面板"), m_rightPanel);
  paramGroup->setObjectName(QStringLiteral("ParamsPanelContainer"));
  paramGroup->setFlat(true);
  auto* paramLayout = new QVBoxLayout(paramGroup);
  paramLayout->setContentsMargins(8, 8, 8, 8);
  m_paramPanel = new ParamPanel(paramGroup);
  m_paramPanel->setObjectName(QStringLiteral("ParametersPanelWidget"));
  paramLayout->addWidget(m_paramPanel);
  rightLayout->addWidget(paramGroup, 1);

  splitter->addWidget(m_rightPanel);
  splitter->setStretchFactor(0, 3);
  splitter->setStretchFactor(1, 2);

  createActions();
  setupMenuBar();
  setupToolBar();
  setupStatusBar();
  setupConnections();
}

void MainWindow::createActions()
{
  m_actionStart = new QAction(QIcon(QStringLiteral(":/icons/play.svg")), tr("启动"), this);
  m_actionStart->setShortcut(QKeySequence(QStringLiteral("F5")));

  m_actionStop = new QAction(QIcon(QStringLiteral(":/icons/stop.svg")), tr("停止"), this);
  m_actionStop->setShortcut(QKeySequence(QStringLiteral("F6")));
  m_actionStop->setEnabled(false);

  m_actionSingleShot = new QAction(QIcon(QStringLiteral(":/icons/camera.svg")), tr("单拍"), this);
  m_actionSingleShot->setShortcut(QKeySequence(QStringLiteral("F7")));

  m_actionSettings = new QAction(QIcon(QStringLiteral(":/icons/settings.svg")), tr("参数"), this);
  m_actionStatistics = new QAction(QIcon(QStringLiteral(":/icons/chart.svg")), tr("统计"), this);

  m_actionOpenConfig = new QAction(tr("打开配置..."), this);
  m_actionOpenConfig->setShortcut(QKeySequence::Open);
  m_actionSaveConfig = new QAction(tr("保存配置..."), this);
  m_actionSaveConfig->setShortcut(QKeySequence::Save);
  m_actionExit = new QAction(tr("退出"), this);
  m_actionExit->setShortcut(QKeySequence::Quit);
  m_actionAbout = new QAction(tr("关于"), this);
}

void MainWindow::setupMenuBar()
{
  auto* bar = menuBar();
  bar->setObjectName(QStringLiteral("PrimaryMenuBar"));
  bar->clear();

  auto* fileMenu = bar->addMenu(tr("文件"));
  fileMenu->addAction(m_actionOpenConfig);
  fileMenu->addAction(m_actionSaveConfig);
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
  toolBar->setObjectName(QStringLiteral("PrimaryToolbar"));
  toolBar->setIconSize(QSize(32, 32));
  toolBar->setMovable(false);
  toolBar->setFloatable(false);
  toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

  toolBar->addAction(m_actionStart);
  toolBar->addAction(m_actionStop);
  toolBar->addSeparator();
  toolBar->addAction(m_actionSingleShot);
  toolBar->addSeparator();
  toolBar->addAction(m_actionSettings);
  toolBar->addAction(m_actionStatistics);

  auto assignButtonName = [toolBar](QAction* action, const QString& name) {
    if (auto* button = qobject_cast<QToolButton*>(toolBar->widgetForAction(action))) {
      button->setObjectName(name);
      button->setCursor(Qt::PointingHandCursor);
    }
  };
  assignButtonName(m_actionStart, QStringLiteral("ToolbarButtonStart"));
  assignButtonName(m_actionStop, QStringLiteral("ToolbarButtonStop"));
  assignButtonName(m_actionSingleShot, QStringLiteral("ToolbarButtonCapture"));
  assignButtonName(m_actionSettings, QStringLiteral("ToolbarButtonSettings"));
  assignButtonName(m_actionStatistics, QStringLiteral("ToolbarButtonStats"));
}

void MainWindow::setupStatusBar()
{
  auto* bar = statusBar();
  bar->setObjectName(QStringLiteral("PrimaryStatusBar"));

  m_cycleTimeLabel = new QLabel(tr("检测速度: -- ms"), bar);
  m_cycleTimeLabel->setObjectName(QStringLiteral("StatusCycleLabel"));
  m_cycleTimeLabel->setMinimumWidth(140);
  bar->addWidget(m_cycleTimeLabel);

  bar->addWidget(new QLabel(QStringLiteral("|"), bar));

  m_totalCountLabel = new QLabel(tr("总数: 0"), bar);
  m_totalCountLabel->setObjectName(QStringLiteral("StatusTotalLabel"));
  bar->addWidget(m_totalCountLabel);

  m_okCountLabel = new QLabel(tr("OK: 0"), bar);
  m_okCountLabel->setObjectName(QStringLiteral("StatusOkLabel"));
  bar->addWidget(m_okCountLabel);

  m_ngCountLabel = new QLabel(tr("NG: 0"), bar);
  m_ngCountLabel->setObjectName(QStringLiteral("StatusNgLabel"));
  bar->addWidget(m_ngCountLabel);

  bar->addWidget(new QLabel(QStringLiteral("|"), bar));

  m_yieldLabel = new QLabel(tr("良率: 0.0%"), bar);
  m_yieldLabel->setObjectName(QStringLiteral("StatusYieldLabel"));
  m_yieldLabel->setMinimumWidth(140);
  bar->addWidget(m_yieldLabel);
}

void MainWindow::setupConnections()
{
  connect(m_actionStart, &QAction::triggered, this, &MainWindow::onStartClicked);
  connect(m_actionStop, &QAction::triggered, this, &MainWindow::onStopClicked);
  connect(m_actionSingleShot, &QAction::triggered, this, &MainWindow::onSingleShotClicked);
  connect(m_actionSettings, &QAction::triggered, this, &MainWindow::onSettingsClicked);
  connect(m_actionStatistics, &QAction::triggered, this, &MainWindow::onStatisticsClicked);
  connect(m_actionOpenConfig, &QAction::triggered, this, &MainWindow::openConfigFile);
  connect(m_actionSaveConfig, &QAction::triggered, this, &MainWindow::saveConfigFile);
  connect(m_actionExit, &QAction::triggered, this, &QWidget::close);
  connect(m_actionAbout, &QAction::triggered, this, [this]() {
    QMessageBox::information(this, tr("关于"), tr("缺陷检测系统 v1.0"));
  });

  if (m_paramPanel) {
    connect(m_paramPanel, &ParamPanel::paramsChanged, this,
            [this](const QString& detector, const QVariantMap&) {
              statusBar()->showMessage(tr("%1 参数已更新").arg(detector), 1500);
            });
  }
}

void MainWindow::openConfigFile()
{
  const QString fileName = QFileDialog::getOpenFileName(
      this, tr("打开参数配置"), QString(), tr("配置文件 (*.json);;所有文件 (*.*)"));
  if (fileName.isEmpty() || !m_paramPanel) {
    return;
  }
  m_paramPanel->loadParams(fileName);
  statusBar()->showMessage(tr("已加载配置: %1").arg(QFileInfo(fileName).fileName()), 2000);
}

void MainWindow::saveConfigFile()
{
  const QString fileName = QFileDialog::getSaveFileName(
      this, tr("保存参数配置"), QStringLiteral("config/params.json"),
      tr("配置文件 (*.json);;所有文件 (*.*)"));
  if (fileName.isEmpty() || !m_paramPanel) {
    return;
  }
  m_paramPanel->saveParams(fileName);
  statusBar()->showMessage(tr("已保存配置: %1").arg(QFileInfo(fileName).fileName()), 2000);
}
