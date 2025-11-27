#include "mainwindow.h"
#include "widgets/ImageView.h"
#include "widgets/ParamPanel.h"
#include "widgets/ResultCard.h"
#include <QHBoxLayout>
#include <QGroupBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow{parent} {
  setupUI();
}

MainWindow::~MainWindow()
{

}

void MainWindow::onStartClicked()
{
  emit startRequested();
}

void MainWindow::onStopClicked()
{
  emit stopRequested();
}

void MainWindow::onSingleShotClicked()
{
  emit singleShotRequested();
}

void MainWindow::onSettingsClicked()
{

}

void MainWindow::onStatisticsClicked()
{

}

void MainWindow::onResultReady(const DetectResult &result)
{

}

void MainWindow::onFrameReady(const cv::Mat &frame)
{

}

void MainWindow::onError(const QString &module, const QString &message)
{

}

void MainWindow::updateStatistics()
{

}

void MainWindow::setupUI()
{
  // 中心组件
  auto* centralWidget = new QWidget();
  setCentralWidget(centralWidget);

  auto* mainLayout = new QHBoxLayout(centralWidget);
  mainLayout->setContentsMargins(4, 4, 4, 4);
  mainLayout->setSpacing(4);

  // 左侧：图像显示区
  m_imageView = new ImageView();
  m_imageView->setMinimumSize(640, 480);
  mainLayout->addWidget(m_imageView, 65);  // 65% 宽度

  // 右侧：结果 + 参数面板
  auto* rightPanel = new QWidget();
  auto* rightLayout = new QVBoxLayout(rightPanel);
  rightLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setSpacing(8);

  // 检测结果卡片
  m_resultCard = new ResultCard();
  m_resultCard->setFixedHeight(200);
  rightLayout->addWidget(m_resultCard);

  // 参数面板（可折叠）
  auto* paramGroup = new QGroupBox("检测参数");
  auto* paramLayout = new QVBoxLayout(paramGroup);
  m_paramPanel = new ParamPanel();
  paramLayout->addWidget(m_paramPanel);
  rightLayout->addWidget(paramGroup, 1);

  rightPanel->setMinimumWidth(300);
  mainLayout->addWidget(rightPanel, 35);  // 35% 宽度

  // 窗口属性
  setWindowTitle("缺陷检测系统 v1.0");
  setMinimumSize(1024, 768);
  resize(1280, 800);
}

void MainWindow::setupMenuBar()
{

}

void MainWindow::setupToolBar()
{

}

void MainWindow::setupStatusBar()
{

}

void MainWindow::setupConnections()
{

}
