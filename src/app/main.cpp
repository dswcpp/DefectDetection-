#include <QApplication>
#include <QObject>
#include <QFile>
#include <QDebug>
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

  auto loadStyleSheet = []() -> QString {
    QFile resourceFile(QStringLiteral(":/styles/app.qss"));
    if (resourceFile.open(QIODevice::ReadOnly)) {
      return QString::fromUtf8(resourceFile.readAll());
    }
    qWarning() << "Failed to load :/styles/app.qss:" << resourceFile.errorString();

    QFile localFile(QStringLiteral(":resources/styles/app.qss"));
    if (localFile.open(QIODevice::ReadOnly)) {
      qWarning() << "Loaded fallback stylesheet from resources/styles/app.qss";
      return QString::fromUtf8(localFile.readAll());
    }
    qWarning() << "Fallback stylesheet missing:" << localFile.errorString();
    return {};
  };

  const QString styleSheet = loadStyleSheet();
  if (!styleSheet.isEmpty()) {
    a.setStyleSheet(styleSheet);
  }

  MainWindow* desktop = new MainWindow();
  DetectPipeline pipeline;

  InitConnectFunc(desktop, pipeline);

  desktop->show();

  int result = a.exec();

  delete desktop;
  desktop = nullptr;
  return result;
}
