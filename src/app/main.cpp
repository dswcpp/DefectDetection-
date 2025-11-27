#include <QApplication>
#include <QObject>
#include "DetectPipeline.h"
#include "mainwindow.h"

#ifdef connect
#undef connect
#endif

void InitConnectFunc(MainWindow* desktop, DetectPipeline& pipeline) {
  const auto sigStart = &MainWindow::startRequested;
  const auto sigStop = &MainWindow::stopRequested;
  const auto sigSingle = &MainWindow::singleShotRequested;

  QObject::connect(desktop, sigStart, &pipeline, &DetectPipeline::start);
  QObject::connect(desktop, sigStop, &pipeline, &DetectPipeline::stop);
  QObject::connect(desktop, sigSingle, &pipeline, &DetectPipeline::singleShot);

  QObject::connect(&pipeline, &DetectPipeline::resultReady, desktop, &MainWindow::onResultReady);
  QObject::connect(&pipeline, &DetectPipeline::frameReady, desktop, &MainWindow::onFrameReady);
  QObject::connect(&pipeline, &DetectPipeline::error, desktop, &MainWindow::onError);
}

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  MainWindow* desktop = new MainWindow();
  DetectPipeline pipeline;

  InitConnectFunc(desktop, pipeline);

  desktop->show();

  int result = a.exec();

  delete desktop;
  desktop = nullptr;
  return result;
}
