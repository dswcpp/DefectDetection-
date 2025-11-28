#include "mainwindow.h"
#include "widgets/ImageView.h"
#include "widgets/ImageViewControls.h"
#include "widgets/ParamPanel.h"
#include "widgets/ResultCard.h"
#include "widgets/AnnotationPanel.h"
#include "dialogs/SettingsDialog.h"
#include "dialogs/StatisticsDialog.h"
#include "dialogs/AboutDialog.h"
#include "Types.h"
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
#include "common\Logger.h"
MainWindow::MainWindow(QWidget *parent) : QMainWindow{parent} {
  setupUI();
}

MainWindow::~MainWindow()
{

}

void MainWindow::onStartClicked()
{
  LOG_INFO("MainWindow::onStartClicked() - 开始选择测试图片");

  // 调试阶段：选择图片文件并展示
  QString fileName = QFileDialog::getOpenFileName(this,
    tr("选择测试图片"),
    QString(),
    tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.tiff);;所有文件 (*.*)"));

  if (!fileName.isEmpty()) {
    qDebug() << "选择的文件:" << fileName;

    // 检查文件是否存在
    QFileInfo fileInfo(fileName);
    if (!fileInfo.exists()) {
      qCritical() << "文件不存在:" << fileName;
      QMessageBox::warning(this, tr("错误"), tr("文件不存在"));
      return;
    }

    qDebug() << "文件大小:" << fileInfo.size() << "bytes";
    qDebug() << "文件路径:" << fileInfo.absoluteFilePath();

    // 加载图片到OpenCV Mat
    qDebug() << "开始使用OpenCV加载图片";

    // 尝试不同的路径格式
    std::string stdPath = fileName.toStdString();
    std::string localPath = fileName.toLocal8Bit().constData();

    qDebug() << "std::string path:" << QString::fromStdString(stdPath);
    qDebug() << "local8bit path:" << QString::fromStdString(localPath);

    cv::Mat image = cv::imread(stdPath);

    // 如果失败，尝试用本地编码路径
    if (image.empty()) {
      qDebug() << "尝试使用本地编码路径";
      image = cv::imread(localPath);
    }

    // 如果还失败，尝试替换路径分隔符
    if (image.empty()) {
      std::string winPath = stdPath;
      std::replace(winPath.begin(), winPath.end(), '/', '\\');
      qDebug() << "尝试Windows路径格式:" << QString::fromStdString(winPath);
      image = cv::imread(winPath);
    }

    if (!image.empty()) {
      qDebug() << "图片加载成功 - 宽度:" << image.cols
               << "高度:" << image.rows
               << "通道数:" << image.channels();

      // 显示图片
      if (m_imageView) {
        qDebug() << "设置图片到ImageView";
        m_imageView->setImage(image);
      } else {
        qCritical() << "m_imageView为空指针";
      }

      // 更新状态栏
      statusBar()->showMessage(tr("已加载图片: %1").arg(fileName), 3000);

      // 模拟检测过程（延迟后显示结果）
      qDebug() << "启动500ms延迟定时器模拟检测";
      QTimer::singleShot(500, this, [this, image, fileName]() {
        qDebug() << "定时器触发 - 开始生成模拟检测结果";
        // 生成模拟检测结果
        DetectResult result;
        result.isOK = (rand() % 10) > 3; // 70% OK率
        result.timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

        if (!result.isOK) {
          // 生成随机缺陷类型
          QStringList defectTypes = {tr("划痕"), tr("裂纹"), tr("异物"), tr("尺寸偏差")};
          result.defectType = defectTypes[rand() % defectTypes.size()];

          // 设置严重度
          int severityIdx = rand() % 3;
          if (severityIdx == 0) {
            result.level = SeverityLevel::Minor;
            result.severity = 0.3;
          } else if (severityIdx == 1) {
            result.level = SeverityLevel::Major;
            result.severity = 0.6;
          } else {
            result.level = SeverityLevel::Critical;
            result.severity = 0.9;
          }

          // 生成随机缺陷区域
          int numDefects = 1 + rand() % 3; // 1-3个缺陷
          for (int i = 0; i < numDefects; ++i) {
            DefectRegion defect;
            defect.bbox = cv::Rect(
              rand() % (image.cols - 100),
              rand() % (image.rows - 100),
              50 + rand() % 100,
              50 + rand() % 100
            );
            defect.confidence = 0.75 + (rand() % 25) / 100.0;
            defect.classId = rand() % 4; // 0-3对应不同缺陷类型

            result.defects.push_back(defect);
            result.regions.push_back(defect.bbox);
          }

          result.confidence = 0.75 + (rand() % 25) / 100.0;
        } else {
          result.level = SeverityLevel::OK;
          result.severity = 0.0;
          result.confidence = 1.0;
        }

        // 更新结果显示
        if (m_resultCard) {
          m_resultCard->setResult(result);
        }

        // 如果有缺陷，在图像上绘制标注
        if (!result.isOK && m_imageView) {
          QVector<DetectionBox> boxes;
          for (const auto& defect : result.defects) {
            DetectionBox box;
            box.rect = defect.bbox;
            box.label = result.defectType;
            box.confidence = defect.confidence;

            // 根据严重度设置颜色
            if (result.level == SeverityLevel::Minor) {
              box.color = QColor(255, 193, 7); // 黄色
            } else if (result.level == SeverityLevel::Major) {
              box.color = QColor(255, 152, 0); // 橙色
            } else {
              box.color = QColor(244, 67, 54); // 红色
            }

            boxes.append(box);
          }
          m_imageView->drawDetectionBoxes(boxes);
        } else if (m_imageView) {
          // 清除标注框
          m_imageView->clearDetectionBoxes();
        }

        // 更新统计
        onResultReady(result);
      });

      // 启用停止按钮，禁用启动按钮
      m_actionStart->setEnabled(false);
      m_actionStop->setEnabled(true);

    } else {
      qCritical() << "OpenCV无法加载图片:" << fileName;
      qDebug() << "尝试使用不同的标志位重新加载";

      // 尝试用不同的标志位加载
      image = cv::imread(fileName.toStdString(), cv::IMREAD_UNCHANGED);
      if (!image.empty()) {
        qDebug() << "使用IMREAD_UNCHANGED成功加载";
        qDebug() << "图片尺寸:" << image.cols << "x" << image.rows
                 << "通道数:" << image.channels();

        // 显示图片
        if (m_imageView) {
          m_imageView->setImage(image);
          statusBar()->showMessage(tr("已加载图片: %1").arg(fileName), 3000);
        }
      } else {
        image = cv::imread(fileName.toStdString(), cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);
        if (!image.empty()) {
          qDebug() << "使用IMREAD_ANYDEPTH|IMREAD_ANYCOLOR成功加载";
          qDebug() << "图片尺寸:" << image.cols << "x" << image.rows
                   << "通道数:" << image.channels();

          // 显示图片
          if (m_imageView) {
            m_imageView->setImage(image);
            statusBar()->showMessage(tr("已加载图片: %1").arg(fileName), 3000);
          }
        } else {
          qCritical() << "所有尝试均失败";

          // 尝试使用 QImage 加载
          QImage qimg(fileName);
          if (!qimg.isNull()) {
            qDebug() << "使用QImage成功加载，尺寸:" << qimg.width() << "x" << qimg.height();
            if (m_imageView) {
              m_imageView->setImage(qimg);
              statusBar()->showMessage(tr("已加载图片: %1").arg(fileName), 3000);
            }
          } else {
            qCritical() << "QImage也无法加载文件";
            QMessageBox::warning(this, tr("错误"), tr("无法加载图片文件"));
          }
        }
      }
    }
  } else {
    qDebug() << "用户取消了文件选择";
  }
}

void MainWindow::onStopClicked()
{
  emit stopRequested();
  m_actionStart->setEnabled(true);
  m_actionStop->setEnabled(false);
}

void MainWindow::onSingleShotClicked()
{
  qDebug() << "MainWindow::onSingleShotClicked() - 开始单拍模式";

  // 调试阶段：选择单张图片进行检测
  QString fileName = QFileDialog::getOpenFileName(this,
    tr("选择图片进行单次检测"),
    QString(),
    tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.tiff);;所有文件 (*.*)"));

  if (!fileName.isEmpty()) {
    qDebug() << "单拍模式 - 选择的文件:" << fileName;

    // 加载图片到OpenCV Mat
    cv::Mat image = cv::imread(fileName.toStdString());

    if (!image.empty()) {
      qDebug() << "单拍模式 - 图片加载成功，尺寸:" << image.cols << "x" << image.rows;
      // 显示图片
      if (m_imageView) {
        m_imageView->setImage(image);
      }

      // 更新状态栏
      statusBar()->showMessage(tr("单拍模式 - 已加载: %1").arg(fileName), 3000);

      // 立即显示检测结果（单拍模式）
      DetectResult result;
      result.isOK = (rand() % 10) > 3; // 70% OK率
      result.timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

      if (!result.isOK) {
        // 生成随机缺陷类型
        QStringList defectTypes = {tr("划痕"), tr("裂纹"), tr("异物"), tr("尺寸偏差")};
        result.defectType = defectTypes[rand() % defectTypes.size()];

        // 设置严重度
        int severityIdx = rand() % 3;
        if (severityIdx == 0) {
          result.level = SeverityLevel::Minor;
          result.severity = 0.3;
        } else if (severityIdx == 1) {
          result.level = SeverityLevel::Major;
          result.severity = 0.6;
        } else {
          result.level = SeverityLevel::Critical;
          result.severity = 0.9;
        }

        // 生成随机缺陷区域
        int numDefects = 1 + rand() % 2; // 1-2个缺陷
        for (int i = 0; i < numDefects; ++i) {
          DefectRegion defect;
          defect.bbox = cv::Rect(
            rand() % (image.cols - 100),
            rand() % (image.rows - 100),
            50 + rand() % 100,
            50 + rand() % 100
          );
          defect.confidence = 0.75 + (rand() % 25) / 100.0;
          defect.classId = rand() % 4; // 0-3对应不同缺陷类型

          result.defects.push_back(defect);
          result.regions.push_back(defect.bbox);
        }

        result.confidence = 0.75 + (rand() % 25) / 100.0;
      } else {
        result.level = SeverityLevel::OK;
        result.severity = 0.0;
        result.confidence = 1.0;
      }

      // 更新结果显示
      if (m_resultCard) {
        m_resultCard->setResult(result);
      }

      // 如果有缺陷，在图像上绘制标注
      if (!result.isOK && m_imageView) {
        QVector<DetectionBox> boxes;
        for (const auto& defect : result.defects) {
          DetectionBox box;
          box.rect = defect.bbox;
          box.label = result.defectType;
          box.confidence = defect.confidence;

          // 根据严重度设置颜色
          if (result.level == SeverityLevel::Minor) {
            box.color = QColor(255, 193, 7); // 黄色
          } else if (result.level == SeverityLevel::Major) {
            box.color = QColor(255, 152, 0); // 橙色
          } else {
            box.color = QColor(244, 67, 54); // 红色
          }

          boxes.append(box);
        }
        m_imageView->drawDetectionBoxes(boxes);
      } else if (m_imageView) {
        // 清除标注框
        m_imageView->clearDetectionBoxes();
      }

      // 更新统计
      onResultReady(result);

    } else {
      QMessageBox::warning(this, tr("错误"), tr("无法加载图片文件"));
    }
  }
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
    StatisticsDialog dialog(this);
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
    updateStatistics();
    statusBar()->showMessage(result.isOK ? tr("最近一次结果: OK") : tr("最近一次结果: NG"), 2000);
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
