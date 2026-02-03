/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * SystemWatchdog.cpp
 *
 * 优化版本：1.1
 * 作者：Vere
 * 修改日期：2026年02月
 * 摘要：系统看门狗实现
 */

#include "SystemWatchdog.h"
#include "common/Logger.h"
#include <QTimer>
#include <QDateTime>
#include <QStorageInfo>
#include <QThread>
#include <QCoreApplication>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#else
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

SystemWatchdog::SystemWatchdog(QObject *parent)
    : QObject(parent)
    , m_checkTimer(new QTimer(this))
{
    connect(m_checkTimer, &QTimer::timeout, this, &SystemWatchdog::onCheckTimer);
    
    // 默认监控应用程序所在磁盘
    m_diskPath = QCoreApplication::applicationDirPath();
}

SystemWatchdog::~SystemWatchdog()
{
    stop();
}

void SystemWatchdog::setCheckInterval(int ms)
{
    m_checkIntervalMs = qMax(100, ms);
    if (m_running) {
        m_checkTimer->setInterval(m_checkIntervalMs);
    }
}

void SystemWatchdog::registerModule(const QString& name, int timeoutMs)
{
    QMutexLocker locker(&m_mutex);
    
    ModuleStatus status;
    status.name = name;
    status.timeoutMs = timeoutMs;
    status.lastHeartbeat = QDateTime::currentMSecsSinceEpoch();
    status.isAlive = true;
    status.missedCount = 0;
    
    m_modules[name] = status;
    
    LOG_INFO("SystemWatchdog: Registered module '{}' with timeout {}ms", 
             name.toStdString(), timeoutMs);
}

void SystemWatchdog::unregisterModule(const QString& name)
{
    QMutexLocker locker(&m_mutex);
    m_modules.remove(name);
    LOG_INFO("SystemWatchdog: Unregistered module '{}'", name.toStdString());
}

QStringList SystemWatchdog::registeredModules() const
{
    QMutexLocker locker(&m_mutex);
    return m_modules.keys();
}

void SystemWatchdog::start()
{
    if (m_running) {
        return;
    }
    
    LOG_INFO("SystemWatchdog: Starting monitoring (interval={}ms, memThreshold={:.1f}%, diskThreshold={:.1f}%)",
             m_checkIntervalMs, m_memoryThreshold, m_diskThreshold);
    
    m_running = true;
    m_wasHealthy = true;
    m_checkTimer->start(m_checkIntervalMs);
    
    // 立即执行一次检查
    onCheckTimer();
}

void SystemWatchdog::stop()
{
    if (!m_running) {
        return;
    }
    
    LOG_INFO("SystemWatchdog: Stopping monitoring");
    m_running = false;
    m_checkTimer->stop();
}

void SystemWatchdog::feed(const QString& module)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_modules.contains(module)) {
        LOG_WARN("SystemWatchdog: Unknown module '{}' sent heartbeat", module.toStdString());
        return;
    }
    
    ModuleStatus& status = m_modules[module];
    status.lastHeartbeat = QDateTime::currentMSecsSinceEpoch();
    
    // 如果之前是死亡状态，现在恢复
    if (!status.isAlive) {
        status.isAlive = true;
        status.missedCount = 0;
        LOG_INFO("SystemWatchdog: Module '{}' recovered", module.toStdString());
        locker.unlock();
        emit moduleRecovered(module);
    }
}

SystemStatus SystemWatchdog::systemStatus() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastStatus;
}

SystemWatchdog::ModuleStatus SystemWatchdog::moduleStatus(const QString& name) const
{
    QMutexLocker locker(&m_mutex);
    return m_modules.value(name);
}

bool SystemWatchdog::isModuleAlive(const QString& name) const
{
    QMutexLocker locker(&m_mutex);
    if (!m_modules.contains(name)) {
        return false;
    }
    return m_modules[name].isAlive;
}

bool SystemWatchdog::isSystemHealthy() const
{
    QMutexLocker locker(&m_mutex);
    
    // 检查所有模块
    for (const auto& module : m_modules) {
        if (!module.isAlive) {
            return false;
        }
    }
    
    // 检查系统资源
    if (m_lastStatus.isLowMemory || m_lastStatus.isLowDisk) {
        return false;
    }
    
    return true;
}

void SystemWatchdog::onCheckTimer()
{
    checkModuleHeartbeats();
    checkSystemResources();
}

void SystemWatchdog::checkModuleHeartbeats()
{
    QMutexLocker locker(&m_mutex);
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    
    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        ModuleStatus& status = it.value();
        qint64 elapsed = now - status.lastHeartbeat;
        
        if (elapsed > status.timeoutMs) {
            if (status.isAlive) {
                status.isAlive = false;
                status.missedCount++;
                
                LOG_WARN("SystemWatchdog: Module '{}' heartbeat timeout ({}ms > {}ms), missed: {}",
                         status.name.toStdString(), elapsed, status.timeoutMs, status.missedCount);
                
                locker.unlock();
                emit heartbeatTimeout(status.name, status.missedCount);
                locker.relock();
            } else {
                // 持续超时
                status.missedCount++;
            }
        }
    }
}

void SystemWatchdog::checkSystemResources()
{
    SystemStatus status = collectSystemStatus();
    
    QMutexLocker locker(&m_mutex);
    
    bool wasLowMemory = m_lastStatus.isLowMemory;
    bool wasLowDisk = m_lastStatus.isLowDisk;
    
    m_lastStatus = status;
    
    locker.unlock();
    
    // 检查内存
    if (status.isLowMemory && !wasLowMemory) {
        LOG_WARN("SystemWatchdog: Low memory warning - available: {}MB, usage: {:.1f}%",
                 status.availableMemoryMB, status.memoryUsagePercent);
        emit lowMemoryWarning(status.availableMemoryMB, status.memoryUsagePercent);
    }
    
    // 检查磁盘
    if (status.isLowDisk && !wasLowDisk) {
        LOG_WARN("SystemWatchdog: Low disk warning - free: {}MB, usage: {:.1f}%",
                 status.freeDiskMB, status.diskUsagePercent);
        emit lowDiskWarning(status.freeDiskMB, status.diskUsagePercent);
    }
    
    // 检查整体健康状态
    bool isHealthy = isSystemHealthy();
    if (isHealthy && !m_wasHealthy) {
        LOG_INFO("SystemWatchdog: System recovered to healthy state");
        emit systemHealthy();
    } else if (!isHealthy && m_wasHealthy) {
        QString reason;
        if (status.isLowMemory) reason += "Low memory; ";
        if (status.isLowDisk) reason += "Low disk; ";
        
        locker.relock();
        for (const auto& module : m_modules) {
            if (!module.isAlive) {
                reason += QString("Module '%1' timeout; ").arg(module.name);
            }
        }
        locker.unlock();
        
        LOG_WARN("SystemWatchdog: System unhealthy - {}", reason.toStdString());
        emit systemUnhealthy(reason);
    }
    m_wasHealthy = isHealthy;
    
    // 发送状态报告
    emit statusReport(status);
}

SystemStatus SystemWatchdog::collectSystemStatus()
{
    SystemStatus status;
    
    // 内存信息
    status.totalMemoryMB = getTotalMemory();
    status.availableMemoryMB = getAvailableMemory();
    if (status.totalMemoryMB > 0) {
        status.memoryUsagePercent = 100.0 * (status.totalMemoryMB - status.availableMemoryMB) / status.totalMemoryMB;
    }
    status.isLowMemory = (status.memoryUsagePercent >= m_memoryThreshold);
    
    // 磁盘信息
    status.totalDiskMB = getTotalDiskSpace(m_diskPath);
    status.freeDiskMB = getFreeDiskSpace(m_diskPath);
    if (status.totalDiskMB > 0) {
        status.diskUsagePercent = 100.0 * (status.totalDiskMB - status.freeDiskMB) / status.totalDiskMB;
    }
    status.isLowDisk = (status.diskUsagePercent >= m_diskThreshold);
    
    // 应用内存
    status.appMemoryMB = getAppMemoryUsage();
    
    // 线程数
    status.threadCount = QThread::idealThreadCount();
    
    return status;
}

quint64 SystemWatchdog::getAvailableMemory()
{
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return memInfo.ullAvailPhys / (1024 * 1024);
    }
    return 0;
#else
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return (info.freeram * info.mem_unit) / (1024 * 1024);
    }
    return 0;
#endif
}

quint64 SystemWatchdog::getTotalMemory()
{
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return memInfo.ullTotalPhys / (1024 * 1024);
    }
    return 0;
#else
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return (info.totalram * info.mem_unit) / (1024 * 1024);
    }
    return 0;
#endif
}

quint64 SystemWatchdog::getFreeDiskSpace(const QString& path)
{
    QStorageInfo storage(path);
    if (storage.isValid()) {
        return storage.bytesAvailable() / (1024 * 1024);
    }
    return 0;
}

quint64 SystemWatchdog::getTotalDiskSpace(const QString& path)
{
    QStorageInfo storage(path);
    if (storage.isValid()) {
        return storage.bytesTotal() / (1024 * 1024);
    }
    return 0;
}

quint64 SystemWatchdog::getAppMemoryUsage()
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / (1024 * 1024);
    }
    return 0;
#else
    // Linux: 读取 /proc/self/statm
    FILE* file = fopen("/proc/self/statm", "r");
    if (file) {
        long size = 0;
        if (fscanf(file, "%ld", &size) == 1) {
            fclose(file);
            return (size * sysconf(_SC_PAGESIZE)) / (1024 * 1024);
        }
        fclose(file);
    }
    return 0;
#endif
}
