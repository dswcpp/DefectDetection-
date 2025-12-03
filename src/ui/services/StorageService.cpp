#include "StorageService.h"
#include "common/Logger.h"
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

StorageService* StorageService::s_instance = nullptr;

StorageService* StorageService::instance() {
    if (!s_instance) {
        s_instance = new StorageService(qApp);
    }
    return s_instance;
}

StorageService::StorageService(QObject* parent)
    : QObject(parent)
    , m_checkTimer(new QTimer(this))
{
    connect(m_checkTimer, &QTimer::timeout, this, &StorageService::onCheckStorage);
}

void StorageService::initialize() {
    // 设置程序所在路径
    setPath(StoragePathType::Application, QCoreApplication::applicationDirPath());
    
    // 日志路径（默认）
    QString logPath = QCoreApplication::applicationDirPath() + "/logs";
    if (QDir(logPath).exists()) {
        setPath(StoragePathType::Log, logPath);
    }
    
    LOG_INFO("StorageService initialized with {} paths", m_paths.size());
}

void StorageService::setPath(StoragePathType type, const QString& path) {
    if (path.isEmpty()) {
        m_paths.remove(type);
        return;
    }
    
    // 确保路径存在
    QDir dir(path);
    if (!dir.exists()) {
        LOG_WARN("StorageService: Path does not exist: {}", path.toStdString());
    }
    
    m_paths[type] = path;
    LOG_DEBUG("StorageService: Set {} path to {}", 
              pathTypeName(type).toStdString(), path.toStdString());
}

QString StorageService::getPath(StoragePathType type) const {
    return m_paths.value(type);
}

void StorageService::addCustomPath(const QString& name, const QString& path) {
    if (name.isEmpty() || path.isEmpty()) return;
    m_customPaths[name] = path;
    LOG_DEBUG("StorageService: Added custom path '{}': {}", 
              name.toStdString(), path.toStdString());
}

void StorageService::removeCustomPath(const QString& name) {
    m_customPaths.remove(name);
    m_pathStatus.remove(name);
}

QString StorageService::pathTypeName(StoragePathType type) const {
    switch (type) {
        case StoragePathType::Application: return tr("程序目录");
        case StoragePathType::Database:    return tr("数据库");
        case StoragePathType::ImageSave:   return tr("图片存储");
        case StoragePathType::ImageBackup: return tr("图片备份");
        case StoragePathType::Log:         return tr("日志目录");
        case StoragePathType::Custom:      return tr("自定义");
    }
    return tr("未知");
}

StorageInfo StorageService::queryStorageInfo(const QString& path, const QString& name) const {
    StorageInfo info;
    info.name = name;
    info.path = path;
    
    QStorageInfo storage(path);
    if (!storage.isValid() || !storage.isReady()) {
        // 尝试使用路径的根目录
        QFileInfo fi(path);
        storage = QStorageInfo(fi.absolutePath());
    }
    
    if (storage.isValid() && storage.isReady()) {
        info.rootPath = storage.rootPath();
        info.displayName = storage.displayName();
        info.totalBytes = storage.bytesTotal();
        info.freeBytes = storage.bytesAvailable();
        info.usedBytes = info.totalBytes - info.freeBytes;
        info.usagePercent = info.totalBytes > 0 
            ? static_cast<int>((info.usedBytes * 100) / info.totalBytes) 
            : 0;
        info.isValid = true;
    }
    
    return info;
}

StorageInfo StorageService::getStorageInfo(const QString& path, const QString& name) const {
    return queryStorageInfo(path, name.isEmpty() ? path : name);
}

StorageInfo StorageService::getStorageInfo(StoragePathType type) const {
    QString path = m_paths.value(type);
    if (path.isEmpty()) {
        StorageInfo info;
        info.name = pathTypeName(type);
        info.isValid = false;
        return info;
    }
    return queryStorageInfo(path, pathTypeName(type));
}

QList<StorageInfo> StorageService::getAllMonitoredStorageInfo() const {
    QList<StorageInfo> result;
    
    // 添加系统路径
    for (auto it = m_paths.begin(); it != m_paths.end(); ++it) {
        result.append(queryStorageInfo(it.value(), pathTypeName(it.key())));
    }
    
    // 添加自定义路径
    for (auto it = m_customPaths.begin(); it != m_customPaths.end(); ++it) {
        result.append(queryStorageInfo(it.value(), it.key()));
    }
    
    return result;
}

QList<StorageInfo> StorageService::getAllMountedVolumes() const {
    QList<StorageInfo> result;
    
    for (const QStorageInfo& storage : QStorageInfo::mountedVolumes()) {
        if (storage.isValid() && storage.isReady()) {
            StorageInfo info;
            info.name = storage.displayName();
            info.path = storage.rootPath();
            info.rootPath = storage.rootPath();
            info.displayName = storage.displayName();
            info.totalBytes = storage.bytesTotal();
            info.freeBytes = storage.bytesAvailable();
            info.usedBytes = info.totalBytes - info.freeBytes;
            info.usagePercent = info.totalBytes > 0 
                ? static_cast<int>((info.usedBytes * 100) / info.totalBytes) 
                : 0;
            info.isValid = true;
            result.append(info);
        }
    }
    
    return result;
}

QString StorageService::formatSize(qint64 bytes) {
    if (bytes < 0) {
        return QString("N/A");
    }
    
    constexpr qint64 KB = 1024;
    constexpr qint64 MB = KB * 1024;
    constexpr qint64 GB = MB * 1024;
    constexpr qint64 TB = GB * 1024;
    
    if (bytes >= TB) {
        return QString("%1 TB").arg(static_cast<double>(bytes) / TB, 0, 'f', 2);
    } else if (bytes >= GB) {
        return QString("%1 GB").arg(static_cast<double>(bytes) / GB, 0, 'f', 2);
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(static_cast<double>(bytes) / MB, 0, 'f', 2);
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(static_cast<double>(bytes) / KB, 0, 'f', 2);
    } else {
        return QString("%1 B").arg(bytes);
    }
}

QString StorageService::getStorageInfoString(const QString& path) const {
    StorageInfo info = getStorageInfo(path);
    
    if (!info.isValid) {
        return tr("无法获取存储信息");
    }
    
    return tr("驱动器: %1\n"
              "总空间: %2\n"
              "已用空间: %3 (%4%)\n"
              "可用空间: %5")
        .arg(info.rootPath)
        .arg(formatSize(info.totalBytes))
        .arg(formatSize(info.usedBytes))
        .arg(info.usagePercent)
        .arg(formatSize(info.freeBytes));
}

QString StorageService::getSummaryString() const {
    QStringList lines;
    lines << tr("=== 存储空间监控 ===");
    
    auto infoList = getAllMonitoredStorageInfo();
    for (const auto& info : infoList) {
        if (info.isValid) {
            QString status;
            if (info.usagePercent >= m_criticalThreshold) {
                status = tr(" [危险]");
            } else if (info.usagePercent >= m_warningThreshold) {
                status = tr(" [警告]");
            }
            
            lines << tr("%1 (%2): %3/%4 (%5%)%6")
                .arg(info.name)
                .arg(info.rootPath)
                .arg(formatSize(info.usedBytes))
                .arg(formatSize(info.totalBytes))
                .arg(info.usagePercent)
                .arg(status);
        } else {
            lines << tr("%1: 不可用").arg(info.name);
        }
    }
    
    return lines.join("\n");
}

bool StorageService::hasEnoughSpace(const QString& path, qint64 requiredBytes) const {
    StorageInfo info = getStorageInfo(path);
    return info.isValid && info.freeBytes >= requiredBytes;
}

bool StorageService::hasEnoughSpace(StoragePathType type, qint64 requiredBytes) const {
    StorageInfo info = getStorageInfo(type);
    return info.isValid && info.freeBytes >= requiredBytes;
}

void StorageService::setWarningThreshold(int percent) {
    m_warningThreshold = qBound(50, percent, 99);
}

void StorageService::setCriticalThreshold(int percent) {
    m_criticalThreshold = qBound(m_warningThreshold + 1, percent, 100);
}

void StorageService::startMonitoring(int intervalMs) {
    if (m_paths.isEmpty() && m_customPaths.isEmpty()) {
        LOG_WARN("StorageService: No paths configured for monitoring");
        return;
    }
    
    m_monitoring = true;
    m_checkTimer->start(intervalMs);
    
    // 立即执行一次检查
    onCheckStorage();
    
    LOG_INFO("StorageService: Started monitoring {} paths (interval: {}ms)", 
             m_paths.size() + m_customPaths.size(), intervalMs);
}

void StorageService::stopMonitoring() {
    m_monitoring = false;
    m_checkTimer->stop();
    m_pathStatus.clear();
    
    LOG_INFO("StorageService: Stopped monitoring");
}

void StorageService::onCheckStorage() {
    QList<StorageInfo> infoList = getAllMonitoredStorageInfo();
    
    emit storageUpdated(infoList);
    
    for (const auto& info : infoList) {
        if (!info.isValid) continue;
        
        QString key = info.name;
        int oldStatus = m_pathStatus.value(key, 0);
        int newStatus = 0;
        
        if (info.usagePercent >= m_criticalThreshold) {
            newStatus = 2;
            if (oldStatus != 2) {
                LOG_ERROR("StorageService: CRITICAL - {} ({}) at {}%", 
                          info.name.toStdString(), info.rootPath.toStdString(), 
                          info.usagePercent);
                emit storageCritical(info.name, info.path, info.usagePercent);
            }
        } else if (info.usagePercent >= m_warningThreshold) {
            newStatus = 1;
            if (oldStatus == 0) {
                LOG_WARN("StorageService: WARNING - {} ({}) at {}%", 
                         info.name.toStdString(), info.rootPath.toStdString(), 
                         info.usagePercent);
                emit storageWarning(info.name, info.path, info.usagePercent);
            }
        } else {
            newStatus = 0;
            if (oldStatus > 0) {
                LOG_INFO("StorageService: NORMAL - {} ({}) at {}%", 
                         info.name.toStdString(), info.rootPath.toStdString(), 
                         info.usagePercent);
                emit storageNormal(info.name, info.path, info.usagePercent);
            }
        }
        
        m_pathStatus[key] = newStatus;
    }
}
