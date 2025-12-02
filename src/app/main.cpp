#include <QApplication>
#include <QObject>
#include <QFile>
#include <QDebug>
#include "config/ConfigManager.h"
#include "Logger.h"
#include "data/DatabaseManager.h"
#include "ui/services/UserManager.h"
#include "ui/dialogs/LoginDialog.h"
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

    // 初始化数据库
    DatabaseManager dbManager;
    if (!dbManager.initFromConfig()) {
      qWarning() << "Failed to initialize database";
    }

    // 显示登录对话框（内部会初始化 UserManager）
    LoginDialog loginDialog;
    loginDialog.setDatabaseManager(&dbManager);
    if (loginDialog.exec() != QDialog::Accepted) {
      LOG_INFO("User cancelled login, exiting");
      return 0;
    }

    LOG_INFO("User logged in: {}", UserManager::instance()->currentUsername().toStdString());

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

    // 安全退出：先停止流水线，再删除窗口
    pipeline.stop();
    
    delete desktop;
    desktop = nullptr;

    LOG_INFO("Application exiting");
    logging::Logger::shutdown();

    return result;
}
