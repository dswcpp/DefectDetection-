/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * mainwindow.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：主窗口接口定义
 * 描述：应用程序主窗口，包含菜单栏、工具栏、状态栏，
 *       管理各功能视图和对话框
 *
 * 当前版本：1.0
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "widgets/FramelessMainWindow.h"
#include "Types.h"
#include "ui_global.h"
#include "Timer.h"
#include <opencv2/core.hpp>  // 只需要 cv::Mat

class QLabel;
class QAction;
class QTableView;
class ImageView;
class ResultCard;
class ParamPanel;
class DetectResult;
class AnnotationPanel;
class DetectPipeline;
class DatabaseManager;
class DefectTableModel;

class UI_LIBRARY MainWindow : public FramelessMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  void setPipeline(DetectPipeline* pipeline);

signals:
  void startRequested();
  void stopRequested();
  void singleShotRequested();

private slots:
  void onStartClicked();
  void onStopClicked();
  void onSingleShotClicked();
  void onSettingsClicked();
  void onStatisticsClicked();
  void onHistoryClicked();
  void onUserManagementClicked();
  void onChangePasswordClicked();
  void onLogoutClicked();
public slots:
  void onResultReady(const DetectResult& result);
  void onFrameReady(const cv::Mat& frame);
  void onError(const QString& module, const QString& message);

  void updateStatistics();

private:
  void setupUI();
  void createActions();
  void setupMenuBar();
  void setupToolBar();
  void setupStatusBar();
  void setupConnections();
  void openConfigFile();
  void saveConfigFile();
  void updateMenuPermissions();
  
  // 统计数据持久化
  void loadStatistics();
  void saveStatistics();
  void resetStatistics();

  // UI 组件
  QWidget* m_centralWidget;
  QWidget* m_imageViewContainer;
  ImageView* m_imageView;
  class ImageViewControls* m_imageViewControls;
  QWidget* m_rightPanel;
  ResultCard* m_resultCard;
  ParamPanel* m_paramPanel;
  AnnotationPanel* m_annotationPanel;

  // 动作
  QAction* m_actionStart;
  QAction* m_actionStop;
  QAction* m_actionSingleShot;
  QAction* m_actionSettings;
  QAction* m_actionStatistics;
  QAction* m_actionHistory;
  QAction* m_actionOpenConfig;
  QAction* m_actionSaveConfig;
  QAction* m_actionExit;
  QAction* m_actionAbout;
  QAction* m_actionUserManagement;
  QAction* m_actionChangePassword;
  QAction* m_actionLogout;
  QAction* m_actionResetStats;

  // 状态栏组件
  QLabel* m_cycleTimeLabel;
  QLabel* m_totalCountLabel;
  QLabel* m_okCountLabel;
  QLabel* m_ngCountLabel;
  QLabel* m_yieldLabel;
  QLabel* m_fpsLabel;

  // 统计
  int m_totalCount = 0;
  int m_okCount = 0;
  int m_ngCount = 0;
  int m_lastCycleTimeMs = 0;

  // 性能监控
  FPSCounter m_fpsCounter;
  PerfStats m_detectStats{"Detection"};
  Throttle m_statusThrottle{100};  // 限制状态栏更新频率 100ms

  // 检测流水线
  DetectPipeline* m_pipeline = nullptr;

  // 数据库管理器
  DatabaseManager* m_dbManager = nullptr;

  // 缺陷列表模型和视图
  DefectTableModel* m_defectModel = nullptr;
  QTableView* m_defectTableView = nullptr;
};
#endif // MAINWINDOW_H
