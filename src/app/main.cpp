#include <QApplication>
#include <QObject>
#include <QFile>
#include <QDebug>
#include "config/ConfigManager.h"
#include "Logger.h"
#include "ui/DetectPipeline.h"
#include "ui/mainwindow.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

  #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
  #endif

    // 加载配置
    if (!gConfig.load()) {
      qWarning() << "Failed to load config, using defaults";
    }

    // 初始化日志（使用配置）
    auto logCfg = gConfig.logConfig();
    logging::LoggerConfig loggerConfig;
    loggerConfig.logDir = logCfg.dir.toStdString();
    loggerConfig.level = logCfg.level.toStdString();
    loggerConfig.maxFileSizeMB = logCfg.maxFileSizeMB;
    loggerConfig.maxFileCount = logCfg.maxFileCount;
    loggerConfig.enableConsole = logCfg.enableConsole;
    logging::Logger::init(loggerConfig);

    LOG_INFO("Application started, config loaded from: {}", gConfig.configPath());

    // 加载样式表
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

    // 创建流水线（使用配置）
    auto camCfg = gConfig.cameraConfig();
    DetectPipeline pipeline;
    pipeline.setImageDir(camCfg.imageDir);
    pipeline.setCaptureInterval(camCfg.captureIntervalMs);

    // 创建主窗口
    MainWindow* desktop = new MainWindow();
    desktop->setPipeline(&pipeline);
    desktop->show();

    int result = a.exec();

    LOG_INFO("Application exiting");
    logging::Logger::shutdown();

    delete desktop;
    return result;
}
