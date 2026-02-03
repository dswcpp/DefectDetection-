/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * BackupManager.cpp
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：数据备份管理模块实现
 */

#include "BackupManager.h"
#include "common/Logger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QCoreApplication>

BackupManager::BackupManager(QObject* parent)
    : QObject(parent)
    , m_autoBackupTimer(new QTimer(this))
{
    connect(m_autoBackupTimer, &QTimer::timeout, 
            this, &BackupManager::onAutoBackupTimer);
    
    LOG_INFO("BackupManager created");
}

BackupManager::~BackupManager()
{
    stopAutoBackup();
    LOG_INFO("BackupManager destroyed");
}

void BackupManager::setConfig(const BackupConfig& config)
{
    m_config = config;
    
    // 确保备份目录存在
    QDir dir(m_config.backupDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    LOG_INFO("BackupManager config updated - backupDir: {}, retentionDays: {}, autoInterval: {}h",
             m_config.backupDir, m_config.retentionDays, m_config.autoBackupIntervalHours);
}

bool BackupManager::isBusy() const
{
    return m_status == BackupStatus::BackingUp || 
           m_status == BackupStatus::Restoring ||
           m_status == BackupStatus::Verifying;
}

void BackupManager::setStatus(BackupStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

QString BackupManager::generateBackupPath() const
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString typeSuffix = (m_config.type == BackupType::Full) ? "full" : "incr";
    QString backupName = QString("backup_%1_%2").arg(timestamp, typeSuffix);
    return QDir(m_config.backupDir).filePath(backupName);
}

bool BackupManager::backupTo(const QString& targetDir)
{
    if (isBusy()) {
        LOG_WARN("BackupManager::backupTo - Already busy, cannot start new backup");
        emit errorOccurred(tr("备份管理器正忙，无法启动新备份"));
        return false;
    }
    
    QString backupPath = targetDir.isEmpty() ? generateBackupPath() : targetDir;
    
    // 验证配置
    if (m_config.includePaths.isEmpty()) {
        LOG_ERROR("BackupManager::backupTo - No paths configured for backup");
        emit errorOccurred(tr("未配置备份路径"));
        return false;
    }
    
    LOG_INFO("BackupManager::backupTo - Starting backup to: {}", backupPath);
    
    // 重置取消标志
    {
        QMutexLocker locker(&m_mutex);
        m_cancelRequested = false;
    }
    
    setStatus(BackupStatus::BackingUp);
    emit backupStarted(backupPath);
    
    // 创建备份目录
    QDir dir(backupPath);
    if (!dir.exists() && !dir.mkpath(".")) {
        LOG_ERROR("BackupManager::backupTo - Failed to create backup directory: {}", backupPath);
        setStatus(BackupStatus::Idle);
        emit backupCompleted(false, tr("无法创建备份目录: %1").arg(backupPath));
        return false;
    }
    
    // 收集要备份的文件
    QStringList allFiles;
    QString appDir = QCoreApplication::applicationDirPath();
    
    for (const QString& includePath : m_config.includePaths) {
        QString fullPath = QDir(appDir).filePath(includePath);
        QFileInfo fi(fullPath);
        
        if (fi.isDir()) {
            QStringList files = collectFiles(fullPath);
            for (const QString& file : files) {
                // 转换为相对路径
                QString relPath = includePath + "/" + QDir(fullPath).relativeFilePath(file);
                allFiles.append(relPath);
            }
        } else if (fi.exists()) {
            allFiles.append(includePath);
        }
    }
    
    if (allFiles.isEmpty()) {
        LOG_WARN("BackupManager::backupTo - No files to backup");
        setStatus(BackupStatus::Idle);
        emit backupCompleted(false, tr("没有找到需要备份的文件"));
        return false;
    }
    
    // 计算总大小
    qint64 totalSize = 0;
    for (const QString& relPath : allFiles) {
        QString fullPath = QDir(appDir).filePath(relPath);
        QFileInfo fi(fullPath);
        if (fi.exists()) {
            totalSize += fi.size();
        }
    }
    
    // 执行备份
    BackupProgress progress;
    progress.totalFiles = allFiles.size();
    progress.totalBytes = totalSize;
    progress.currentFile = 0;
    progress.bytesProcessed = 0;
    
    bool cancelled = false;
    int successCount = 0;
    
    for (int i = 0; i < allFiles.size(); ++i) {
        // 检查取消标志
        {
            QMutexLocker locker(&m_mutex);
            if (m_cancelRequested) {
                cancelled = true;
                break;
            }
        }
        
        const QString& relPath = allFiles[i];
        QString srcPath = QDir(appDir).filePath(relPath);
        QString dstPath = QDir(backupPath).filePath(relPath);
        
        // 更新进度
        progress.currentFile = i + 1;
        progress.currentFileName = QFileInfo(relPath).fileName();
        progress.percent = (totalSize > 0) 
            ? static_cast<int>(progress.bytesProcessed * 100 / totalSize) 
            : static_cast<int>((i + 1) * 100 / allFiles.size());
        
        emit backupProgress(progress);
        
        // 确保目标目录存在
        QDir dstDir = QFileInfo(dstPath).dir();
        if (!dstDir.exists()) {
            dstDir.mkpath(".");
        }
        
        // 复制文件
        if (copyFile(srcPath, dstPath)) {
            QFileInfo fi(srcPath);
            progress.bytesProcessed += fi.size();
            successCount++;
        } else {
            LOG_WARN("BackupManager::backupTo - Failed to copy: {} -> {}", srcPath, dstPath);
        }
    }
    
    if (cancelled) {
        LOG_INFO("BackupManager::backupTo - Backup cancelled by user");
        // 清理不完整的备份
        QDir(backupPath).removeRecursively();
        setStatus(BackupStatus::Idle);
        emit backupCompleted(false, tr("备份已被用户取消"));
        return false;
    }
    
    // 写入清单文件
    BackupInfo info;
    info.path = backupPath;
    info.name = QDir(backupPath).dirName();
    info.timestamp = QDateTime::currentDateTime();
    info.type = m_config.type;
    info.sizeBytes = progress.bytesProcessed;
    info.fileCount = successCount;
    info.isValid = true;
    info.checksum = calculateChecksum(backupPath);
    
    writeManifest(backupPath, info);
    
    m_lastBackupTime = info.timestamp;
    
    // 完成
    progress.percent = 100;
    emit backupProgress(progress);
    
    LOG_INFO("BackupManager::backupTo - Backup completed: {} files, {} bytes", 
             successCount, progress.bytesProcessed);
    
    setStatus(BackupStatus::Idle);
    emit backupCompleted(true, tr("备份完成: %1 个文件, %2 字节")
                                   .arg(successCount).arg(progress.bytesProcessed));
    
    return true;
}

void BackupManager::cancelBackup()
{
    QMutexLocker locker(&m_mutex);
    if (m_status == BackupStatus::BackingUp) {
        m_cancelRequested = true;
        LOG_INFO("BackupManager::cancelBackup - Cancel requested");
    }
}

bool BackupManager::restoreFrom(const QString& sourceDir)
{
    if (isBusy()) {
        LOG_WARN("BackupManager::restoreFrom - Already busy, cannot start restore");
        emit errorOccurred(tr("备份管理器正忙，无法启动恢复"));
        return false;
    }
    
    // 验证备份目录
    QDir srcDir(sourceDir);
    if (!srcDir.exists()) {
        LOG_ERROR("BackupManager::restoreFrom - Source directory does not exist: {}", sourceDir);
        emit errorOccurred(tr("备份目录不存在: %1").arg(sourceDir));
        return false;
    }
    
    // 读取清单
    BackupInfo info = readManifest(sourceDir);
    if (!info.isValid) {
        LOG_WARN("BackupManager::restoreFrom - No valid manifest found, proceeding anyway");
    }
    
    LOG_INFO("BackupManager::restoreFrom - Starting restore from: {}", sourceDir);
    
    // 重置取消标志
    {
        QMutexLocker locker(&m_mutex);
        m_cancelRequested = false;
    }
    
    setStatus(BackupStatus::Restoring);
    emit restoreStarted(sourceDir);
    
    // 收集备份中的文件
    QStringList files = collectFiles(sourceDir);
    
    // 过滤掉清单文件
    files.removeAll(QDir(sourceDir).filePath("manifest.json"));
    
    if (files.isEmpty()) {
        LOG_WARN("BackupManager::restoreFrom - No files to restore");
        setStatus(BackupStatus::Idle);
        emit restoreCompleted(false, tr("备份中没有找到文件"));
        return false;
    }
    
    // 计算总大小
    qint64 totalSize = 0;
    for (const QString& file : files) {
        totalSize += QFileInfo(file).size();
    }
    
    // 执行恢复
    BackupProgress progress;
    progress.totalFiles = files.size();
    progress.totalBytes = totalSize;
    progress.currentFile = 0;
    progress.bytesProcessed = 0;
    
    QString appDir = QCoreApplication::applicationDirPath();
    bool cancelled = false;
    int successCount = 0;
    
    for (int i = 0; i < files.size(); ++i) {
        // 检查取消标志
        {
            QMutexLocker locker(&m_mutex);
            if (m_cancelRequested) {
                cancelled = true;
                break;
            }
        }
        
        const QString& srcPath = files[i];
        QString relPath = srcDir.relativeFilePath(srcPath);
        QString dstPath = QDir(appDir).filePath(relPath);
        
        // 更新进度
        progress.currentFile = i + 1;
        progress.currentFileName = QFileInfo(srcPath).fileName();
        progress.percent = (totalSize > 0) 
            ? static_cast<int>(progress.bytesProcessed * 100 / totalSize) 
            : static_cast<int>((i + 1) * 100 / files.size());
        
        emit restoreProgress(progress);
        
        // 确保目标目录存在
        QDir dstDir = QFileInfo(dstPath).dir();
        if (!dstDir.exists()) {
            dstDir.mkpath(".");
        }
        
        // 复制文件
        if (copyFile(srcPath, dstPath)) {
            progress.bytesProcessed += QFileInfo(srcPath).size();
            successCount++;
        } else {
            LOG_WARN("BackupManager::restoreFrom - Failed to copy: {} -> {}", srcPath, dstPath);
        }
    }
    
    if (cancelled) {
        LOG_INFO("BackupManager::restoreFrom - Restore cancelled by user");
        setStatus(BackupStatus::Idle);
        emit restoreCompleted(false, tr("恢复已被用户取消"));
        return false;
    }
    
    // 完成
    progress.percent = 100;
    emit restoreProgress(progress);
    
    LOG_INFO("BackupManager::restoreFrom - Restore completed: {} files", successCount);
    
    setStatus(BackupStatus::Idle);
    emit restoreCompleted(true, tr("恢复完成: %1 个文件").arg(successCount));
    
    return true;
}

void BackupManager::cancelRestore()
{
    QMutexLocker locker(&m_mutex);
    if (m_status == BackupStatus::Restoring) {
        m_cancelRequested = true;
        LOG_INFO("BackupManager::cancelRestore - Cancel requested");
    }
}

QList<BackupInfo> BackupManager::listBackups() const
{
    QList<BackupInfo> backups;
    
    QDir dir(m_config.backupDir);
    if (!dir.exists()) {
        return backups;
    }
    
    QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    
    for (const QString& entry : entries) {
        if (entry.startsWith("backup_")) {
            QString backupPath = dir.filePath(entry);
            BackupInfo info = readManifest(backupPath);
            if (info.isValid || QDir(backupPath).exists()) {
                // 如果没有清单，创建基本信息
                if (!info.isValid) {
                    info.path = backupPath;
                    info.name = entry;
                    info.isValid = false;
                    
                    // 尝试从目录名解析时间
                    QStringList parts = entry.split('_');
                    if (parts.size() >= 3) {
                        QString dateStr = parts[1] + parts[2].left(6);
                        info.timestamp = QDateTime::fromString(dateStr, "yyyyMMddHHmmss");
                    }
                }
                backups.append(info);
            }
        }
    }
    
    return backups;
}

BackupInfo BackupManager::latestBackup() const
{
    QList<BackupInfo> backups = listBackups();
    
    BackupInfo latest;
    latest.isValid = false;
    
    for (const BackupInfo& info : backups) {
        if (!latest.isValid || info.timestamp > latest.timestamp) {
            latest = info;
        }
    }
    
    return latest;
}

bool BackupManager::verifyBackup(const QString& backupPath)
{
    LOG_INFO("BackupManager::verifyBackup - Verifying: {}", backupPath);
    
    setStatus(BackupStatus::Verifying);
    
    BackupInfo info = readManifest(backupPath);
    if (!info.isValid) {
        LOG_WARN("BackupManager::verifyBackup - No manifest found");
        setStatus(BackupStatus::Idle);
        return false;
    }
    
    // 计算当前校验和
    QString currentChecksum = calculateChecksum(backupPath);
    
    bool valid = (currentChecksum == info.checksum);
    
    if (valid) {
        LOG_INFO("BackupManager::verifyBackup - Backup is valid");
    } else {
        LOG_WARN("BackupManager::verifyBackup - Checksum mismatch: expected {}, got {}",
                 info.checksum, currentChecksum);
    }
    
    setStatus(BackupStatus::Idle);
    return valid;
}

bool BackupManager::deleteBackup(const QString& backupPath)
{
    LOG_INFO("BackupManager::deleteBackup - Deleting: {}", backupPath);
    
    QDir dir(backupPath);
    if (!dir.exists()) {
        LOG_WARN("BackupManager::deleteBackup - Directory does not exist");
        return false;
    }
    
    bool success = dir.removeRecursively();
    
    if (success) {
        LOG_INFO("BackupManager::deleteBackup - Successfully deleted");
    } else {
        LOG_ERROR("BackupManager::deleteBackup - Failed to delete");
    }
    
    return success;
}

int BackupManager::cleanupOldBackups(int keepDays)
{
    int days = (keepDays < 0) ? m_config.retentionDays : keepDays;
    
    LOG_INFO("BackupManager::cleanupOldBackups - Cleaning backups older than {} days", days);
    
    QDateTime cutoffTime = QDateTime::currentDateTime().addDays(-days);
    QList<BackupInfo> backups = listBackups();
    
    int deletedCount = 0;
    
    for (const BackupInfo& info : backups) {
        if (info.timestamp.isValid() && info.timestamp < cutoffTime) {
            if (deleteBackup(info.path)) {
                deletedCount++;
            }
        }
    }
    
    LOG_INFO("BackupManager::cleanupOldBackups - Deleted {} old backups", deletedCount);
    
    return deletedCount;
}

void BackupManager::startAutoBackup()
{
    if (m_config.autoBackupIntervalHours <= 0) {
        LOG_WARN("BackupManager::startAutoBackup - Auto backup interval not configured");
        return;
    }
    
    int intervalMs = m_config.autoBackupIntervalHours * 60 * 60 * 1000;
    m_autoBackupTimer->start(intervalMs);
    
    LOG_INFO("BackupManager::startAutoBackup - Auto backup enabled, interval: {} hours",
             m_config.autoBackupIntervalHours);
}

void BackupManager::stopAutoBackup()
{
    if (m_autoBackupTimer->isActive()) {
        m_autoBackupTimer->stop();
        LOG_INFO("BackupManager::stopAutoBackup - Auto backup disabled");
    }
}

bool BackupManager::isAutoBackupEnabled() const
{
    return m_autoBackupTimer->isActive();
}

QDateTime BackupManager::nextAutoBackupTime() const
{
    if (!m_autoBackupTimer->isActive()) {
        return QDateTime();
    }
    
    int remainingMs = m_autoBackupTimer->remainingTime();
    return QDateTime::currentDateTime().addMSecs(remainingMs);
}

void BackupManager::onAutoBackupTimer()
{
    LOG_INFO("BackupManager::onAutoBackupTimer - Starting scheduled backup");
    
    if (!isBusy()) {
        backupTo();
        
        // 自动清理旧备份
        cleanupOldBackups();
    } else {
        LOG_WARN("BackupManager::onAutoBackupTimer - Skipped: manager is busy");
    }
}

bool BackupManager::copyDirectory(const QString& srcDir, const QString& dstDir,
                                   BackupProgress& progress, bool& cancelled)
{
    QDir src(srcDir);
    QDir dst(dstDir);
    
    if (!dst.exists() && !dst.mkpath(".")) {
        return false;
    }
    
    QStringList entries = src.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    
    for (const QString& entry : entries) {
        // 检查取消标志
        {
            QMutexLocker locker(&m_mutex);
            if (m_cancelRequested) {
                cancelled = true;
                return false;
            }
        }
        
        QString srcPath = src.filePath(entry);
        QString dstPath = dst.filePath(entry);
        
        QFileInfo fi(srcPath);
        
        if (fi.isDir()) {
            if (!copyDirectory(srcPath, dstPath, progress, cancelled)) {
                if (cancelled) return false;
            }
        } else {
            if (shouldExclude(srcPath)) {
                continue;
            }
            
            progress.currentFileName = entry;
            emit backupProgress(progress);
            
            if (!copyFile(srcPath, dstPath)) {
                LOG_WARN("BackupManager::copyDirectory - Failed to copy: {}", srcPath);
            } else {
                progress.currentFile++;
                progress.bytesProcessed += fi.size();
            }
        }
    }
    
    return true;
}

bool BackupManager::copyFile(const QString& srcPath, const QString& dstPath)
{
    // 如果目标文件已存在，先删除
    if (QFile::exists(dstPath)) {
        QFile::remove(dstPath);
    }
    
    return QFile::copy(srcPath, dstPath);
}

bool BackupManager::shouldExclude(const QString& filePath) const
{
    QString fileName = QFileInfo(filePath).fileName();
    
    for (const QString& pattern : m_config.excludePatterns) {
        QRegularExpression regex(QRegularExpression::wildcardToRegularExpression(pattern));
        if (regex.match(fileName).hasMatch()) {
            return true;
        }
    }
    
    return false;
}

bool BackupManager::writeManifest(const QString& backupPath, const BackupInfo& info)
{
    QString manifestPath = QDir(backupPath).filePath("manifest.json");
    
    QJsonObject obj;
    obj["name"] = info.name;
    obj["timestamp"] = info.timestamp.toString(Qt::ISODate);
    obj["type"] = (info.type == BackupType::Full) ? "full" : "incremental";
    obj["sizeBytes"] = info.sizeBytes;
    obj["fileCount"] = info.fileCount;
    obj["checksum"] = info.checksum;
    obj["version"] = "1.0";
    
    QJsonDocument doc(obj);
    
    QFile file(manifestPath);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR("BackupManager::writeManifest - Failed to open: {}", manifestPath);
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}

BackupInfo BackupManager::readManifest(const QString& backupPath) const
{
    BackupInfo info;
    info.path = backupPath;
    info.isValid = false;
    
    QString manifestPath = QDir(backupPath).filePath("manifest.json");
    
    QFile file(manifestPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return info;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();
    
    if (error.error != QJsonParseError::NoError) {
        LOG_WARN("BackupManager::readManifest - JSON parse error: {}", error.errorString());
        return info;
    }
    
    QJsonObject obj = doc.object();
    
    info.name = obj["name"].toString();
    info.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
    info.type = (obj["type"].toString() == "full") ? BackupType::Full : BackupType::Incremental;
    info.sizeBytes = obj["sizeBytes"].toVariant().toLongLong();
    info.fileCount = obj["fileCount"].toInt();
    info.checksum = obj["checksum"].toString();
    info.isValid = true;
    
    return info;
}

QString BackupManager::calculateChecksum(const QString& path) const
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    
    QStringList files = collectFiles(path);
    std::sort(files.begin(), files.end());  // 确保顺序一致
    
    for (const QString& filePath : files) {
        // 跳过清单文件本身
        if (filePath.endsWith("manifest.json")) {
            continue;
        }
        
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            hash.addData(file.readAll());
            file.close();
        }
    }
    
    return hash.result().toHex();
}

QStringList BackupManager::collectFiles(const QString& baseDir) const
{
    QStringList files;
    
    QDirIterator it(baseDir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        if (!shouldExclude(filePath)) {
            files.append(filePath);
        }
    }
    
    return files;
}

qint64 BackupManager::calculateTotalSize(const QStringList& files) const
{
    qint64 total = 0;
    for (const QString& file : files) {
        total += QFileInfo(file).size();
    }
    return total;
}
