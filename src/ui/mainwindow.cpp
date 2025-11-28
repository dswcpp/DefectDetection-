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
#include <QIcon>
#include <QKeySequence>
#include <QLatin1String>
#include <QMessageBox>
#include <QSize>

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



  m_centralWidget = new QWidget(this);

  setCentralWidget(m_centralWidget);



  auto* mainLayout = new QHBoxLayout(m_centralWidget);

  mainLayout->setContentsMargins(4, 4, 4, 4);

  mainLayout->setSpacing(8);



  m_imageView = new ImageView(this);

  m_imageView->setMinimumSize(640, 480);

  mainLayout->addWidget(m_imageView, 65);



  m_rightPanel = new QWidget(this);

  auto* rightLayout = new QVBoxLayout(m_rightPanel);

  rightLayout->setContentsMargins(0, 0, 0, 0);

  rightLayout->setSpacing(12);



  m_resultCard = new ResultCard(this);

  m_resultCard->setFixedHeight(200);

  rightLayout->addWidget(m_resultCard);



  auto* paramGroup = new QGroupBox(tr("参数面板"), this);

  auto* paramLayout = new QVBoxLayout(paramGroup);

  m_paramPanel = new ParamPanel(paramGroup);

  paramLayout->addWidget(m_paramPanel);

  rightLayout->addWidget(paramGroup, 1);
  m_rightPanel->setMinimumWidth(320);
  mainLayout->addWidget(m_rightPanel, 35);

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
  m_actionFile = new QAction(tr("退出"), this);
  m_actionFile->setShortcut(QKeySequence::Quit);
  m_actionHelp = new QAction(tr("关于"), this);
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
  fileMenu->addAction(m_actionFile);
  auto* settingsMenu = bar->addMenu(tr("设置"));
  settingsMenu->addAction(m_actionSettings);
  settingsMenu->addAction(m_actionStatistics);
  auto* helpMenu = bar->addMenu(tr("帮助"));
  helpMenu->addAction(m_actionHelp);
}

void MainWindow::setupToolBar()
{
  auto* toolBar = addToolBar(tr("主工具栏"));
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
    connect(m_actionFile, &QAction::triggered, this, &QWidget::close);
    connect(m_actionHelp, &QAction::triggered, this, [this]() {
        QMessageBox::information(this, tr("����"), tr("ȱ�ݼ��ϵͳ v1.0"));
    });
}
