/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * BackupManager.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：数据备份管理模块接口定义
 * 描述：数据库和配置文件备份管理，支持定时备份、增量备份、
 *       备份恢复等功能
 *
 * 当前版本：1.0
 */

#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H

#include "../data_global.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QTimer>
#include <QMutex>

/**
 * @brief 备份类型
 */
enum class BackupType {
    Full,           // 全量备份
    Incremental     // 增量备份（仅备份变化的文件）
};

/**
 * @brief 备份状态
 */
enum class BackupStatus {
    Idle,           // 空闲
    BackingUp,      // 备份中
    Restoring,      // 恢复中
    Verifying       // 验证中
};

/**
 * @brief 备份配置
 */
struct DATA_EXPORT BackupConfig {
    QString backupDir = "backups";          // 备份根目录
    int retentionDays = 30;                 // 保留天数
    int autoBackupIntervalHours = 24;       // 自动备份间隔（小时），0禁用
    BackupType type = BackupType::Full;     // 备份类型
    bool compressBackup = true;             // 是否压缩备份
    QStringList includePaths;               // 要备份的路径列表
    QStringList excludePatterns;            // 排除的文件模式（通配符）
    
    // 默认包含的路径
    static BackupConfig defaults() {
        BackupConfig cfg;
        cfg.includePaths = {"config", "database"};
        cfg.excludePatterns = {"*.tmp", "*.log", "*.bak"};
        return cfg;
    }
};

/**
 * @brief 备份信息
 */
struct DATA_EXPORT BackupInfo {
    QString path;               // 备份路径
    QString name;               // 备份名称
    QDateTime timestamp;        // 备份时间
    BackupType type;            // 备份类型
    qint64 sizeBytes;           // 备份大小（字节）
    int fileCount;              // 文件数量
    bool isValid;               // 是否有效
    QString checksum;           // 校验和（用于验证完整性）
};

/**
 * @brief 备份进度信息
 */
struct DATA_EXPORT BackupProgress {
    int currentFile;            // 当前文件索引
    int totalFiles;             // 总文件数
    QString currentFileName;    // 当前处理的文件名
    qint64 bytesProcessed;      // 已处理字节数
    qint64 totalBytes;          // 总字节数
    int percent;                // 百分比 (0-100)
};

/**
 * @brief 数据备份管理器
 *
 * 提供数据库、配置文件的备份和恢复功能，支持：
 * - 全量/增量备份
 * - 定时自动备份
 * - 备份恢复
 * - 备份验证
 * - 过期备份清理
 *
 * 用法：
 * @code
 * BackupManager manager;
 * BackupConfig config = BackupConfig::defaults();
 * config.backupDir = "D:/backups";
 * manager.setConfig(config);
 *
 * connect(&manager, &BackupManager::backupProgress, [](const BackupProgress& p) {
 *     qDebug() << "Progress:" << p.percent << "%" << p.currentFileName;
 * });
 *
 * connect(&manager, &BackupManager::backupCompleted, [](bool success, const QString& msg) {
 *     qDebug() << "Backup" << (success ? "succeeded" : "failed") << msg;
 * });
 *
 * manager.backupTo("D:/backups/manual_backup");
 * @endcode
 */
class DATA_EXPORT BackupManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(BackupStatus status READ status NOTIFY statusChanged)

public:
    explicit BackupManager(QObject* parent = nullptr);
    ~BackupManager() override;

    // ======================== 配置 ========================

    /**
     * @brief 设置配置
     */
    void setConfig(const BackupConfig& config);
    
    /**
     * @brief 获取配置
     */
    BackupConfig config() const { return m_config; }

    // ======================== 状态 ========================

    /**
     * @brief 获取当前状态
     */
    BackupStatus status() const { return m_status; }

    /**
     * @brief 是否正忙（备份或恢复中）
     */
    bool isBusy() const;

    // ======================== 备份操作 ========================

    /**
     * @brief 执行备份到指定目录
     * @param targetDir 目标目录，为空则自动生成带时间戳的目录
     * @return 是否成功启动备份
     */
    bool backupTo(const QString& targetDir = QString());

    /**
     * @brief 取消当前备份操作
     */
    void cancelBackup();

    // ======================== 恢复操作 ========================

    /**
     * @brief 从备份恢复
     * @param sourceDir 备份源目录
     * @return 是否成功启动恢复
     */
    bool restoreFrom(const QString& sourceDir);

    /**
     * @brief 取消当前恢复操作
     */
    void cancelRestore();

    // ======================== 备份管理 ========================

    /**
     * @brief 列出所有备份
     */
    QList<BackupInfo> listBackups() const;

    /**
     * @brief 获取最新备份
     */
    BackupInfo latestBackup() const;

    /**
     * @brief 验证备份完整性
     * @param backupPath 备份路径
     * @return 是否有效
     */
    bool verifyBackup(const QString& backupPath);

    /**
     * @brief 删除指定备份
     */
    bool deleteBackup(const QString& backupPath);

    /**
     * @brief 清理过期备份
     * @param keepDays 保留天数，-1使用配置值
     * @return 删除的备份数量
     */
    int cleanupOldBackups(int keepDays = -1);

    // ======================== 定时备份 ========================

    /**
     * @brief 启动自动备份定时器
     */
    void startAutoBackup();

    /**
     * @brief 停止自动备份定时器
     */
    void stopAutoBackup();

    /**
     * @brief 是否启用了自动备份
     */
    bool isAutoBackupEnabled() const;

    /**
     * @brief 获取下次自动备份时间
     */
    QDateTime nextAutoBackupTime() const;

signals:
    /**
     * @brief 状态改变
     */
    void statusChanged(BackupStatus status);

    /**
     * @brief 备份开始
     */
    void backupStarted(const QString& targetPath);

    /**
     * @brief 备份进度更新
     */
    void backupProgress(const BackupProgress& progress);

    /**
     * @brief 备份完成
     */
    void backupCompleted(bool success, const QString& message);

    /**
     * @brief 恢复开始
     */
    void restoreStarted(const QString& sourcePath);

    /**
     * @brief 恢复进度更新
     */
    void restoreProgress(const BackupProgress& progress);

    /**
     * @brief 恢复完成
     */
    void restoreCompleted(bool success, const QString& message);

    /**
     * @brief 错误发生
     */
    void errorOccurred(const QString& error);

private slots:
    void onAutoBackupTimer();

private:
    // 内部实现
    void setStatus(BackupStatus status);
    QString generateBackupPath() const;
    bool copyDirectory(const QString& srcDir, const QString& dstDir, 
                       BackupProgress& progress, bool& cancelled);
    bool copyFile(const QString& srcPath, const QString& dstPath);
    bool shouldExclude(const QString& filePath) const;
    bool writeManifest(const QString& backupPath, const BackupInfo& info);
    BackupInfo readManifest(const QString& backupPath) const;
    QString calculateChecksum(const QString& path) const;
    QStringList collectFiles(const QString& baseDir) const;
    qint64 calculateTotalSize(const QStringList& files) const;

    BackupConfig m_config;
    BackupStatus m_status = BackupStatus::Idle;
    QTimer* m_autoBackupTimer = nullptr;
    QDateTime m_lastBackupTime;
    
    // 取消标志
    mutable QMutex m_mutex;
    bool m_cancelRequested = false;
};

#endif // BACKUPMANAGER_H
