/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * StorageService.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：存储服务接口定义
 * 描述：存储空间监控服务，监控程序目录、数据库、图片存储等关键路径的磁盘空间
 *
 * 当前版本：1.0
 */

#ifndef STORAGESERVICE_H
#define STORAGESERVICE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QStorageInfo>
#include <QTimer>
#include "ui_global.h"

// 存储路径类型
enum class StoragePathType {
    Application,    // 程序所在路径
    Database,       // 数据库路径
    ImageSave,      // 图片保存路径
    ImageBackup,    // 图片备份路径
    Log,            // 日志路径
    Custom          // 自定义路径
};

// 存储空间信息结构
struct StorageInfo {
    QString name;               // 路径名称/用途
    QString path;               // 实际路径
    QString rootPath;           // 驱动器根路径
    QString displayName;        // 驱动器显示名称
    qint64 totalBytes = 0;      // 总空间
    qint64 usedBytes = 0;       // 已用空间
    qint64 freeBytes = 0;       // 可用空间
    int usagePercent = 0;       // 使用百分比
    bool isValid = false;       // 是否有效
};

// 存储服务（单例）- 监控多个关键路径的存储空间
class UI_LIBRARY StorageService : public QObject {
    Q_OBJECT
public:
    static StorageService* instance();

    // 初始化（从配置加载路径）
    void initialize();

    // 设置各类路径
    void setPath(StoragePathType type, const QString& path);
    QString getPath(StoragePathType type) const;
    
    // 添加自定义监控路径
    void addCustomPath(const QString& name, const QString& path);
    void removeCustomPath(const QString& name);

    // 获取指定路径的存储信息
    StorageInfo getStorageInfo(const QString& path, const QString& name = QString()) const;
    
    // 获取指定类型路径的存储信息
    StorageInfo getStorageInfo(StoragePathType type) const;
    
    // 获取所有监控路径的存储信息
    QList<StorageInfo> getAllMonitoredStorageInfo() const;
    
    // 获取所有挂载的存储设备信息
    QList<StorageInfo> getAllMountedVolumes() const;
    
    // 格式化存储大小为可读字符串
    static QString formatSize(qint64 bytes);
    
    // 获取格式化的存储信息字符串（单个路径）
    QString getStorageInfoString(const QString& path) const;
    
    // 获取所有监控路径的汇总信息字符串
    QString getSummaryString() const;
    
    // 检查存储空间是否充足
    bool hasEnoughSpace(const QString& path, qint64 requiredBytes) const;
    bool hasEnoughSpace(StoragePathType type, qint64 requiredBytes) const;
    
    // 设置警告阈值（百分比，默认85%）
    void setWarningThreshold(int percent);
    int warningThreshold() const { return m_warningThreshold; }
    
    // 设置危险阈值（百分比，默认95%）
    void setCriticalThreshold(int percent);
    int criticalThreshold() const { return m_criticalThreshold; }
    
    // 启动/停止监控
    void startMonitoring(int intervalMs = 60000);
    void stopMonitoring();
    bool isMonitoring() const { return m_monitoring; }

signals:
    void storageUpdated(const QList<StorageInfo>& infoList);
    void storageWarning(const QString& name, const QString& path, int usagePercent);
    void storageCritical(const QString& name, const QString& path, int usagePercent);
    void storageNormal(const QString& name, const QString& path, int usagePercent);

private slots:
    void onCheckStorage();

private:
    explicit StorageService(QObject* parent = nullptr);
    ~StorageService() = default;
    StorageService(const StorageService&) = delete;
    StorageService& operator=(const StorageService&) = delete;

    QString pathTypeName(StoragePathType type) const;
    StorageInfo queryStorageInfo(const QString& path, const QString& name) const;

    // 监控路径 <类型/名称, 路径>
    QMap<StoragePathType, QString> m_paths;
    QMap<QString, QString> m_customPaths;
    
    // 状态跟踪 <路径, 状态>
    QMap<QString, int> m_pathStatus;  // 0=normal, 1=warning, 2=critical
    
    int m_warningThreshold = 85;
    int m_criticalThreshold = 95;
    bool m_monitoring = false;
    QTimer* m_checkTimer = nullptr;
    
    static StorageService* s_instance;
};

#endif // STORAGESERVICE_H
