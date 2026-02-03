/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * SystemWatchdog.h
 *
 * 优化版本：1.1
 * 作者：Vere
 * 修改日期：2026年02月
 * 摘要：系统看门狗接口定义
 * 描述：系统健康监控，检测主线程阻塞、内存泄漏、磁盘空间不足等异常，
 *       支持多模块心跳监控和自动告警
 */

#ifndef SYSTEMWATCHDOG_H
#define SYSTEMWATCHDOG_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QMutex>
#include <QThread>

class QTimer;

// 系统资源状态
struct SystemStatus {
    // 内存
    quint64 totalMemoryMB = 0;
    quint64 availableMemoryMB = 0;
    double memoryUsagePercent = 0.0;
    
    // 磁盘
    quint64 totalDiskMB = 0;
    quint64 freeDiskMB = 0;
    double diskUsagePercent = 0.0;
    
    // CPU（简化）
    double cpuUsagePercent = 0.0;
    
    // 应用
    quint64 appMemoryMB = 0;
    int threadCount = 0;
    
    // 状态标志
    bool isLowMemory = false;
    bool isLowDisk = false;
    bool isHighCPU = false;
};

class SystemWatchdog : public QObject {
    Q_OBJECT
public:
    // 模块状态
    struct ModuleStatus {
        QString name;
        qint64 lastHeartbeat = 0;
        int timeoutMs = 5000;
        bool isAlive = true;
        int missedCount = 0;
    };

    explicit SystemWatchdog(QObject *parent = nullptr);
    ~SystemWatchdog();

    // 配置
    void setCheckInterval(int ms);
    int checkInterval() const { return m_checkIntervalMs; }
    
    void setMemoryThreshold(double percent) { m_memoryThreshold = percent; }
    double memoryThreshold() const { return m_memoryThreshold; }
    
    void setDiskThreshold(double percent) { m_diskThreshold = percent; }
    double diskThreshold() const { return m_diskThreshold; }
    
    void setDiskPath(const QString& path) { m_diskPath = path; }
    QString diskPath() const { return m_diskPath; }

    // 模块注册
    void registerModule(const QString& name, int timeoutMs = 5000);
    void unregisterModule(const QString& name);
    QStringList registeredModules() const;

    // 启动/停止监控
    void start();
    void stop();
    bool isRunning() const { return m_running; }

    // 心跳喂狗
    void feed(const QString& module);
    
    // 查询状态
    SystemStatus systemStatus() const;
    ModuleStatus moduleStatus(const QString& name) const;
    bool isModuleAlive(const QString& name) const;
    bool isSystemHealthy() const;

signals:
    void heartbeatTimeout(const QString& module, int missedCount);
    void moduleRecovered(const QString& module);
    void lowMemoryWarning(quint64 availableMB, double usagePercent);
    void lowDiskWarning(quint64 freeMB, double usagePercent);
    void highCPUWarning(double usagePercent);
    void statusReport(const SystemStatus& status);
    void systemHealthy();
    void systemUnhealthy(const QString& reason);

private slots:
    void onCheckTimer();

private:
    void checkModuleHeartbeats();
    void checkSystemResources();
    SystemStatus collectSystemStatus();
    
    // 平台相关
    quint64 getAvailableMemory();
    quint64 getTotalMemory();
    quint64 getFreeDiskSpace(const QString& path);
    quint64 getTotalDiskSpace(const QString& path);
    quint64 getAppMemoryUsage();

    bool m_running = false;
    int m_checkIntervalMs = 1000;
    double m_memoryThreshold = 90.0;  // 内存使用率告警阈值
    double m_diskThreshold = 95.0;    // 磁盘使用率告警阈值
    QString m_diskPath;
    
    QTimer* m_checkTimer = nullptr;
    QMap<QString, ModuleStatus> m_modules;
    mutable QMutex m_mutex;
    
    SystemStatus m_lastStatus;
    bool m_wasHealthy = true;
};

#endif // SYSTEMWATCHDOG_H
