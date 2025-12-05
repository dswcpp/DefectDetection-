#include <QApplication>
#include <QObject>
#include <QFile>
#include <QDebug>
#include <cstdio>
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
    // 设置异常处理
    try {
        qDebug() << "[INIT] Creating QApplication...";
        QApplication a(argc, argv);
        qDebug() << "[INIT] QApplication created";

      #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
      #endif

        // 加载配置
        qDebug() << "[INIT] Loading config...";
        if (!gConfig.load()) {
          qWarning() << "Failed to load config, using defaults";
        }
        qDebug() << "[INIT] Config loaded";

        // 初始化日志（使用配置）
        qDebug() << "[INIT] Initializing logger...";
        auto logCfg = gConfig.logConfig();
        logging::LoggerConfig loggerConfig;
        loggerConfig.logDir = logCfg.dir.toStdString();
        loggerConfig.level = logCfg.level.toStdString();
        loggerConfig.maxFileSizeMB = logCfg.maxFileSizeMB;
        loggerConfig.maxFileCount = logCfg.maxFileCount;
        loggerConfig.enableConsole = logCfg.enableConsole;
        
        if (!logging::Logger::init(loggerConfig)) {
            qWarning() << "[INIT] Logger init failed:" << QString::fromStdString(logging::Logger::lastError());
        }
        qDebug() << "[INIT] Logger initialized";

        LOG_INFO("Application started, config loaded from: {}", gConfig.configPath());

        // 加载样式表
        qDebug() << "[INIT] Loading stylesheet...";
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
        qDebug() << "[INIT] Stylesheet loaded";

        // 初始化数据库
        qDebug() << "[INIT] Initializing database...";
        DatabaseManager dbManager;
        if (!dbManager.initFromConfig()) {
          qWarning() << "Failed to initialize database";
        }
        qDebug() << "[INIT] Database initialized";

        // 显示登录对话框（内部会初始化 UserManager）
        qDebug() << "[INIT] Creating login dialog...";
        qDebug().nospace() << "[INIT] About to construct LoginDialog";
        std::fflush(stdout);
        std::fflush(stderr);
        LoginDialog loginDialog;
        qDebug() << "[INIT] LoginDialog constructed";
        loginDialog.setDatabaseManager(&dbManager);
        qDebug() << "[INIT] Showing login dialog...";
        if (loginDialog.exec() != QDialog::Accepted) {
          LOG_INFO("User cancelled login, exiting");
          logging::Logger::shutdown();
          return 0;
        }

        LOG_INFO("User logged in: {}", UserManager::instance()->currentUsername().toStdString());

        // 创建流水线（使用配置）
        qDebug() << "[INIT] Creating pipeline...";
        auto camCfg = gConfig.cameraConfig();
        DetectPipeline pipeline;
        pipeline.setImageDir(camCfg.imageDir);
        pipeline.setCaptureInterval(camCfg.captureIntervalMs);
        qDebug() << "[INIT] Pipeline created";

        // 创建主窗口
        qDebug() << "[INIT] Creating main window...";
        MainWindow* desktop = new MainWindow();
        desktop->setPipeline(&pipeline);
        desktop->show();
        qDebug() << "[INIT] Main window shown";

        int result = a.exec();

        // 安全退出：先停止流水线，再删除窗口
        qDebug() << "[EXIT] Stopping pipeline...";
        pipeline.stop();
        
        qDebug() << "[EXIT] Deleting main window...";
        delete desktop;
        desktop = nullptr;

        LOG_INFO("Application exiting");
        qDebug() << "[EXIT] Shutting down logger...";
        logging::Logger::shutdown();
        qDebug() << "[EXIT] Done";

        return result;
        
    } catch (const std::exception& e) {
        qCritical() << "[CRASH] Exception:" << e.what();
        return -1;
    } catch (...) {
        qCritical() << "[CRASH] Unknown exception";
        return -1;
    }
}
