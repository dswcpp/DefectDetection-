/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * main.cpp
 *
 * 优化版本：1.1
 * 作者：Vere
 * 修改日期：2026年02月
 * 摘要：应用程序入口
 * 描述：
 *   - 单实例检测（防止多开）
 *   - 崩溃处理和错误报告
 *   - 系统监控集成
 *   - 优雅退出处理
 */

#include <QApplication>
#include <QObject>
#include <QFile>
#include <QDebug>
#include <QSharedMemory>
#include <QMessageBox>
#include <QDir>
#include <cstdio>
#include <csignal>

#include "config/ConfigManager.h"
#include "Logger.h"
#include "data/DatabaseManager.h"
#include "ui/services/UserManager.h"
#include "ui/dialogs/LoginDialog.h"
#include "ui/DetectPipeline.h"
#include "ui/mainwindow.h"
#include "FlowController.h"
#include "SystemWatchdog.h"
#include "ConfigValidator.h"

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

// ============================================================================
// 全局变量
// ============================================================================
static QSharedMemory* g_sharedMemory = nullptr;
static SystemWatchdog* g_watchdog = nullptr;
static QString g_crashDumpPath;

// ============================================================================
// 单实例检测
// ============================================================================
bool checkSingleInstance(const QString& appKey)
{
    g_sharedMemory = new QSharedMemory(appKey);
    
    // 尝试附加到已存在的共享内存
    if (g_sharedMemory->attach()) {
        // 已有实例运行
        return false;
    }
    
    // 创建新的共享内存
    if (!g_sharedMemory->create(1)) {
        // 可能是上次崩溃后残留，尝试清理
        g_sharedMemory->attach();
        g_sharedMemory->detach();
        
        if (!g_sharedMemory->create(1)) {
            return false;
        }
    }
    
    return true;
}

void releaseSingleInstance()
{
    if (g_sharedMemory) {
        if (g_sharedMemory->isAttached()) {
            g_sharedMemory->detach();
        }
        delete g_sharedMemory;
        g_sharedMemory = nullptr;
    }
}

// ============================================================================
// 崩溃处理
// ============================================================================
#ifdef _WIN32
LONG WINAPI crashHandler(EXCEPTION_POINTERS* exceptionInfo)
{
    // 生成崩溃转储文件
    QString dumpFile = g_crashDumpPath + "/crash_" + 
                       QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".dmp";
    
    HANDLE hFile = CreateFileW(
        reinterpret_cast<LPCWSTR>(dumpFile.utf16()),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = exceptionInfo;
        mdei.ClientPointers = FALSE;
        
        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            MiniDumpWithDataSegs,
            &mdei,
            NULL,
            NULL
        );
        
        CloseHandle(hFile);
        
        LOG_CRIT("Application crashed! Dump saved to: {}", dumpFile.toStdString());
    }
    
    // 记录崩溃日志
    LOG_CRIT("Unhandled exception: code=0x{:08X}, address=0x{:p}",
                 exceptionInfo->ExceptionRecord->ExceptionCode,
                 exceptionInfo->ExceptionRecord->ExceptionAddress);
    
    logging::Logger::shutdown();
    
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

void signalHandler(int signal)
{
    const char* signalName = "UNKNOWN";
    switch (signal) {
    case SIGINT:  signalName = "SIGINT"; break;
    case SIGTERM: signalName = "SIGTERM"; break;
    case SIGSEGV: signalName = "SIGSEGV"; break;
    case SIGABRT: signalName = "SIGABRT"; break;
    }
    
    LOG_CRIT("Received signal: {} ({})", signalName, signal);
    logging::Logger::shutdown();
    
    // 清理
    releaseSingleInstance();
    
    exit(signal);
}

void setupCrashHandler()
{
    // 创建崩溃转储目录
    g_crashDumpPath = QCoreApplication::applicationDirPath() + "/crashes";
    QDir().mkpath(g_crashDumpPath);
    
#ifdef _WIN32
    SetUnhandledExceptionFilter(crashHandler);
#endif
    
    // 设置信号处理
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGABRT, signalHandler);
}

// ============================================================================
// 样式表加载
// ============================================================================
QString loadStyleSheet()
{
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
}

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char* argv[])
{
    int result = -1;
    
    try {
        // 创建应用程序
        qDebug() << "[INIT] Creating QApplication...";
        QApplication app(argc, argv);
        app.setApplicationName("DefectDetection");
        app.setApplicationVersion("1.0.0");
        app.setOrganizationName("DefectDetection");
        qDebug() << "[INIT] QApplication created";

#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
#endif

        // 单实例检测
        const QString appKey = "DefectDetection_SingleInstance_Key";
        if (!checkSingleInstance(appKey)) {
            QMessageBox::warning(nullptr, 
                QObject::tr("警告"), 
                QObject::tr("程序已在运行中，不能重复启动。"));
            return 0;
        }

        // 设置崩溃处理
        setupCrashHandler();

        // 加载配置
        qDebug() << "[INIT] Loading config...";
        if (!gConfig.load()) {
            qWarning() << "Failed to load config, using defaults";
        }
        
        // 验证配置
        ConfigValidator validator;
        auto validationResult = validator.validate(gConfig.config());
        if (!validationResult.valid) {
            qWarning() << "Config validation failed:";
            for (const auto& error : validationResult.errors) {
                qWarning() << "  Error:" << error;
            }
        }
        for (const auto& warning : validationResult.warnings) {
            qWarning() << "  Warning:" << warning;
        }
        qDebug() << "[INIT] Config loaded";

        // 初始化日志
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

        LOG_INFO("=== Application started ===");
        LOG_INFO("Version: {}", app.applicationVersion().toStdString());
        LOG_INFO("Config: {}", gConfig.configPath());

        // 加载样式表
        qDebug() << "[INIT] Loading stylesheet...";
        const QString styleSheet = loadStyleSheet();
        if (!styleSheet.isEmpty()) {
            app.setStyleSheet(styleSheet);
        }
        qDebug() << "[INIT] Stylesheet loaded";

        // 初始化数据库
        qDebug() << "[INIT] Initializing database...";
        DatabaseManager dbManager;
        if (!dbManager.initFromConfig()) {
            LOG_ERROR("Failed to initialize database");
            QMessageBox::critical(nullptr, 
                QObject::tr("错误"), 
                QObject::tr("数据库初始化失败，请检查配置。"));
            releaseSingleInstance();
            return -1;
        }
        qDebug() << "[INIT] Database initialized";

        // 启动系统监控
        qDebug() << "[INIT] Starting system watchdog...";
        g_watchdog = new SystemWatchdog();
        g_watchdog->setCheckInterval(5000);  // 5秒检查一次
        g_watchdog->setMemoryThreshold(90.0);
        g_watchdog->setDiskThreshold(95.0);
        g_watchdog->registerModule("Pipeline", 10000);
        g_watchdog->registerModule("Camera", 5000);
        g_watchdog->start();
        
        // 监控告警
        QObject::connect(g_watchdog, &SystemWatchdog::lowMemoryWarning, [](quint64 availableMB, double usage) {
            LOG_WARN("Low memory warning: {}MB available ({:.1f}% used)", availableMB, usage);
        });
        QObject::connect(g_watchdog, &SystemWatchdog::lowDiskWarning, [](quint64 freeMB, double usage) {
            LOG_WARN("Low disk warning: {}MB free ({:.1f}% used)", freeMB, usage);
        });
        qDebug() << "[INIT] System watchdog started";

        // 显示登录对话框
        qDebug() << "[INIT] Creating login dialog...";
        LoginDialog loginDialog;
        loginDialog.setDatabaseManager(&dbManager);
        qDebug() << "[INIT] Showing login dialog...";
        if (loginDialog.exec() != QDialog::Accepted) {
            LOG_INFO("User cancelled login, exiting");
            g_watchdog->stop();
            delete g_watchdog;
            logging::Logger::shutdown();
            releaseSingleInstance();
            return 0;
        }

        LOG_INFO("User logged in: {}", UserManager::instance()->currentUsername().toStdString());

        // 创建检测流水线
        qDebug() << "[INIT] Creating pipeline...";
        auto camCfg = gConfig.cameraConfig();
        DetectPipeline pipeline;
        pipeline.setImageDir(camCfg.imageDir);
        pipeline.setCaptureInterval(camCfg.captureIntervalMs);
        
        // 流水线心跳
        QObject::connect(&pipeline, &DetectPipeline::resultReady, [](const DetectResult&) {
            if (g_watchdog) {
                g_watchdog->feed("Pipeline");
            }
        });
        qDebug() << "[INIT] Pipeline created";

        // 创建流程控制器（可选使用）
        FlowController flowController;
        flowController.setPipeline(&pipeline);
        flowController.setTriggerMode(FlowController::TriggerMode::Timer);
        flowController.setTriggerInterval(camCfg.captureIntervalMs);

        // 创建主窗口
        qDebug() << "[INIT] Creating main window...";
        MainWindow* mainWindow = new MainWindow();
        mainWindow->setPipeline(&pipeline);
        mainWindow->show();
        qDebug() << "[INIT] Main window shown";

        LOG_INFO("Application ready");

        // 运行事件循环
        result = app.exec();

        // ============ 安全退出 ============
        qDebug() << "[EXIT] Stopping pipeline...";
        pipeline.stop();
        
        qDebug() << "[EXIT] Stopping watchdog...";
        g_watchdog->stop();
        delete g_watchdog;
        g_watchdog = nullptr;
        
        qDebug() << "[EXIT] Deleting main window...";
        delete mainWindow;
        mainWindow = nullptr;

        LOG_INFO("=== Application exiting (code: {}) ===", result);
        
    } catch (const std::exception& e) {
        LOG_CRIT("Unhandled exception: {}", e.what());
        qCritical() << "[CRASH] Exception:" << e.what();
        result = -1;
    } catch (...) {
        LOG_CRIT("Unknown exception");
        qCritical() << "[CRASH] Unknown exception";
        result = -1;
    }

    // 清理
    qDebug() << "[EXIT] Shutting down logger...";
    logging::Logger::shutdown();
    
    qDebug() << "[EXIT] Releasing single instance lock...";
    releaseSingleInstance();
    
    qDebug() << "[EXIT] Done";
    return result;
}
