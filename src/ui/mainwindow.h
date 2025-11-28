#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Types.h"
#include "ui_global.h"
#include "opencv2/opencv.hpp"
class QLabel;
class QAction;
class ImageView;
class ResultCard;
class ParamPanel;
class DetectResult;
class AnnotationPanel;

class UI_LIBRARY MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

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
  QAction* m_actionOpenConfig;
  QAction* m_actionSaveConfig;
  QAction* m_actionExit;
  QAction* m_actionAbout;

  // 状态栏组件
  QLabel* m_cycleTimeLabel;
  QLabel* m_totalCountLabel;
  QLabel* m_okCountLabel;
  QLabel* m_ngCountLabel;
  QLabel* m_yieldLabel;

  // 统计
  int m_totalCount = 0;
  int m_okCount = 0;
  int m_ngCount = 0;
  int m_lastCycleTimeMs = 0;
};
#endif // MAINWINDOW_H
