/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ImageStorage.cpp
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：图像存储模块实现
 */

#include "ImageStorage.h"
#include "common/Logger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QBuffer>
#include <QImageReader>
#include <QImageWriter>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

ImageStorage::ImageStorage(QObject* parent)
    : QObject(parent)
    , m_autoCleanupTimer(new QTimer(this))
{
    connect(m_autoCleanupTimer, &QTimer::timeout,
            this, &ImageStorage::onAutoCleanupTimer);
    
    LOG_INFO("ImageStorage created");
}

ImageStorage::~ImageStorage()
{
    stopAutoCleanup();
    LOG_INFO("ImageStorage destroyed");
}

void ImageStorage::setConfig(const ImageStorageConfig& config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
    
    // 确保根目录存在
    QDir dir(m_config.rootPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    LOG_INFO("ImageStorage config updated - rootPath: {}, retentionDays: {}, jpegQuality: {}",
             m_config.rootPath, m_config.retentionDays, m_config.jpegQuality);
}

QString ImageStorage::generatePath(const QString& baseName, ImageType type,
                                    const QDateTime& timestamp,
                                    ImageFormat format) const
{
    QMutexLocker locker(&m_mutex);
    
    QString path = m_config.rootPath;
    
    // 按日期组织目录
    if (m_config.organizeByDate) {
        path = QDir(path).filePath(getDateSubdir(timestamp));
    }
    
    // 按类型添加子目录
    path = QDir(path).filePath(getTypeSubdir(type));
    
    // 添加文件名
    QString fileName = baseName.isEmpty() ? generateUniqueName("img", timestamp) : baseName;
    
    // 添加类型后缀
    switch (type) {
        case ImageType::Annotated:
            fileName += "_annotated";
            break;
        case ImageType::Thumbnail:
            fileName += "_thumb";
            break;
        case ImageType::Crop:
            fileName += "_crop";
            break;
        default:
            break;
    }
    
    // 添加扩展名
    fileName += getFormatExtension(format);
    
    return QDir(path).filePath(fileName);
}

QString ImageStorage::generateUniqueName(const QString& prefix, const QDateTime& timestamp) const
{
    int counter = m_uniqueCounter.fetch_add(1);
    return QString("%1_%2_%3")
        .arg(prefix)
        .arg(timestamp.toString("yyyyMMdd_HHmmss"))
        .arg(counter, 4, 10, QChar('0'));
}

ImageSaveResult ImageStorage::saveImage(const QImage& image, ImageType type,
                                         const QString& baseName, ImageFormat format)
{
    ImageSaveResult result;
    result.success = false;
    result.fileSize = 0;
    
    if (image.isNull()) {
        result.errorMessage = tr("图像为空");
        LOG_ERROR("ImageStorage::saveImage - Image is null");
        return result;
    }
    
    // 生成保存路径
    QDateTime timestamp = QDateTime::currentDateTime();
    QString name = baseName.isEmpty() ? generateUniqueName("img", timestamp) : baseName;
    result.path = generatePath(name, type, timestamp, format);
    
    // 确保目录存在
    if (!ensureDirectoryExists(QFileInfo(result.path).absolutePath())) {
        result.errorMessage = tr("无法创建目录");
        LOG_ERROR("ImageStorage::saveImage - Failed to create directory for: {}", result.path);
        return result;
    }
    
    // 保存图像
    QImageWriter writer(result.path);
    
    if (format == ImageFormat::JPEG || 
        (format == ImageFormat::Auto && result.path.endsWith(".jpg", Qt::CaseInsensitive))) {
        writer.setQuality(m_config.jpegQuality);
    }
    
    if (!writer.write(image)) {
        result.errorMessage = tr("保存图像失败: %1").arg(writer.errorString());
        LOG_ERROR("ImageStorage::saveImage - Failed to write image: {} - {}",
                  result.path, writer.errorString());
        return result;
    }
    
    result.fileSize = QFileInfo(result.path).size();
    result.success = true;
    
    LOG_DEBUG("ImageStorage::saveImage - Saved: {} ({} bytes)", result.path, result.fileSize);
    
    // 自动创建缩略图（仅对原图和标注图）
    if (m_config.createThumbnails && 
        (type == ImageType::Original || type == ImageType::Annotated)) {
        result.thumbnailPath = createThumbnail(image, name, timestamp);
    }
    
    // 检查存储限制
    checkStorageLimit();
    
    return result;
}

ImageSaveResult ImageStorage::saveImage(const QByteArray& data, const QString& path)
{
    ImageSaveResult result;
    result.success = false;
    result.path = path;
    result.fileSize = 0;
    
    if (data.isEmpty()) {
        result.errorMessage = tr("数据为空");
        LOG_ERROR("ImageStorage::saveImage - Data is empty");
        return result;
    }
    
    // 确保目录存在
    if (!ensureDirectoryExists(QFileInfo(path).absolutePath())) {
        result.errorMessage = tr("无法创建目录");
        LOG_ERROR("ImageStorage::saveImage - Failed to create directory for: {}", path);
        return result;
    }
    
    // 写入文件
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        result.errorMessage = tr("无法打开文件: %1").arg(file.errorString());
        LOG_ERROR("ImageStorage::saveImage - Failed to open file: {} - {}",
                  path, file.errorString());
        return result;
    }
    
    qint64 written = file.write(data);
    file.close();
    
    if (written != data.size()) {
        result.errorMessage = tr("写入数据不完整");
        LOG_ERROR("ImageStorage::saveImage - Incomplete write: {} of {} bytes",
                  written, data.size());
        return result;
    }
    
    result.fileSize = written;
    result.success = true;
    
    LOG_DEBUG("ImageStorage::saveImage - Saved: {} ({} bytes)", path, result.fileSize);
    
    // 检查存储限制
    checkStorageLimit();
    
    return result;
}

void ImageStorage::saveImageAsync(const QImage& image, ImageType type,
                                   const QString& baseName, ImageFormat format)
{
    // 使用 QtConcurrent 进行异步保存
    QImage imageCopy = image.copy();  // 复制图像以避免线程问题
    QString name = baseName;
    ImageStorageConfig config = m_config;
    
    QFuture<ImageSaveResult> future = QtConcurrent::run([this, imageCopy, type, name, format]() {
        return saveImage(imageCopy, type, name, format);
    });
    
    // 监视结果
    auto* watcher = new QFutureWatcher<ImageSaveResult>(this);
    connect(watcher, &QFutureWatcher<ImageSaveResult>::finished, this, [this, watcher]() {
        ImageSaveResult result = watcher->result();
        emit imageSaved(result);
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

void ImageStorage::saveImagesAsync(const QList<std::tuple<QImage, ImageType, QString>>& images)
{
    if (images.isEmpty()) {
        emit batchSaveCompleted(0, 0);
        return;
    }
    
    // 创建所有图像的副本
    QList<std::tuple<QImage, ImageType, QString>> imagesCopy;
    for (const auto& item : images) {
        imagesCopy.append(std::make_tuple(std::get<0>(item).copy(), 
                                           std::get<1>(item), 
                                           std::get<2>(item)));
    }
    
    QFuture<QList<ImageSaveResult>> future = QtConcurrent::run([this, imagesCopy]() {
        QList<ImageSaveResult> results;
        for (const auto& item : imagesCopy) {
            results.append(saveImage(std::get<0>(item), std::get<1>(item), std::get<2>(item)));
        }
        return results;
    });
    
    auto* watcher = new QFutureWatcher<QList<ImageSaveResult>>(this);
    connect(watcher, &QFutureWatcher<QList<ImageSaveResult>>::finished, this, [this, watcher]() {
        QList<ImageSaveResult> results = watcher->result();
        int success = 0, fail = 0;
        for (const auto& r : results) {
            if (r.success) {
                success++;
                emit imageSaved(r);
            } else {
                fail++;
            }
        }
        emit batchSaveCompleted(success, fail);
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

QImage ImageStorage::loadImage(const QString& path) const
{
    QImage image;
    
    if (!QFile::exists(path)) {
        LOG_WARN("ImageStorage::loadImage - File not found: {}", path);
        return image;
    }
    
    QImageReader reader(path);
    reader.setAutoDetectImageFormat(true);
    
    if (!reader.read(&image)) {
        LOG_ERROR("ImageStorage::loadImage - Failed to read: {} - {}",
                  path, reader.errorString());
        return QImage();
    }
    
    LOG_DEBUG("ImageStorage::loadImage - Loaded: {} ({}x{})", 
              path, image.width(), image.height());
    
    return image;
}

QByteArray ImageStorage::loadImageData(const QString& path) const
{
    QFile file(path);
    
    if (!file.exists()) {
        LOG_WARN("ImageStorage::loadImageData - File not found: {}", path);
        return QByteArray();
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("ImageStorage::loadImageData - Failed to open: {} - {}",
                  path, file.errorString());
        return QByteArray();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    LOG_DEBUG("ImageStorage::loadImageData - Loaded: {} ({} bytes)", path, data.size());
    
    return data;
}

bool ImageStorage::imageExists(const QString& path) const
{
    return QFile::exists(path);
}

QString ImageStorage::createThumbnail(const QImage& image, const QString& baseName,
                                       const QDateTime& timestamp)
{
    if (image.isNull()) {
        return QString();
    }
    
    QImage thumb = createThumbnailImage(image);
    if (thumb.isNull()) {
        return QString();
    }
    
    QString path = generatePath(baseName, ImageType::Thumbnail, timestamp, ImageFormat::JPEG);
    
    if (!ensureDirectoryExists(QFileInfo(path).absolutePath())) {
        LOG_ERROR("ImageStorage::createThumbnail - Failed to create directory");
        return QString();
    }
    
    QImageWriter writer(path);
    writer.setQuality(m_config.jpegQuality);
    
    if (!writer.write(thumb)) {
        LOG_ERROR("ImageStorage::createThumbnail - Failed to write: {}", path);
        return QString();
    }
    
    LOG_DEBUG("ImageStorage::createThumbnail - Created: {}", path);
    
    return path;
}

QString ImageStorage::getThumbnailPath(const QString& originalPath) const
{
    QFileInfo fi(originalPath);
    QString baseName = fi.completeBaseName();
    QString dir = fi.absolutePath();
    
    // 替换目录中的类型子目录
    QString typeDir = getTypeSubdir(ImageType::Original);
    QString thumbDir = getTypeSubdir(ImageType::Thumbnail);
    
    if (dir.contains(typeDir)) {
        dir.replace(typeDir, thumbDir);
    } else {
        dir = QDir(dir).filePath(thumbDir);
    }
    
    // 添加缩略图后缀
    QString thumbName = baseName;
    if (!thumbName.endsWith("_thumb")) {
        thumbName += "_thumb";
    }
    thumbName += ".jpg";
    
    return QDir(dir).filePath(thumbName);
}

bool ImageStorage::deleteImage(const QString& path)
{
    if (!QFile::exists(path)) {
        return false;
    }
    
    bool success = QFile::remove(path);
    
    if (success) {
        LOG_DEBUG("ImageStorage::deleteImage - Deleted: {}", path);
    } else {
        LOG_ERROR("ImageStorage::deleteImage - Failed to delete: {}", path);
    }
    
    return success;
}

int ImageStorage::deleteImagesByDate(const QDate& date)
{
    QMutexLocker locker(&m_mutex);
    
    QString dateDir = m_config.rootPath;
    if (m_config.organizeByDate) {
        dateDir = QDir(m_config.rootPath).filePath(date.toString(m_config.dateFormat));
    }
    
    QDir dir(dateDir);
    if (!dir.exists()) {
        return 0;
    }
    
    int count = 0;
    
    // 递归删除目录中的所有文件
    QDirIterator it(dateDir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        if (QFile::remove(filePath)) {
            count++;
        }
    }
    
    // 清理空目录
    dir.removeRecursively();
    
    LOG_INFO("ImageStorage::deleteImagesByDate - Deleted {} files for date: {}",
             count, date.toString());
    
    return count;
}

int ImageStorage::cleanupExpiredImages(int keepDays)
{
    int days = (keepDays < 0) ? m_config.retentionDays : keepDays;
    
    LOG_INFO("ImageStorage::cleanupExpiredImages - Cleaning images older than {} days", days);
    
    QDate cutoffDate = QDate::currentDate().addDays(-days);
    
    int deletedCount = 0;
    qint64 freedBytes = 0;
    
    // 遍历日期目录
    QDir rootDir(m_config.rootPath);
    
    if (m_config.organizeByDate) {
        // 按日期组织的目录结构
        QDirIterator it(m_config.rootPath, QDir::Dirs | QDir::NoDotAndDotDot, 
                        QDirIterator::Subdirectories);
        
        while (it.hasNext()) {
            QString dirPath = it.next();
            QDir dir(dirPath);
            
            // 检查是否是日期目录（包含图像文件）
            QStringList images = dir.entryList({"*.jpg", "*.jpeg", "*.png", "*.bmp"}, QDir::Files);
            if (images.isEmpty()) {
                continue;
            }
            
            // 获取第一个图像的修改时间来判断目录日期
            QFileInfo fi(dir.filePath(images.first()));
            QDate fileDate = fi.lastModified().date();
            
            if (fileDate < cutoffDate) {
                for (const QString& image : images) {
                    QString imagePath = dir.filePath(image);
                    qint64 fileSize = QFileInfo(imagePath).size();
                    if (QFile::remove(imagePath)) {
                        deletedCount++;
                        freedBytes += fileSize;
                    }
                }
            }
        }
    } else {
        // 扁平目录结构
        QStringList images = collectImageFiles(m_config.rootPath);
        for (const QString& imagePath : images) {
            QFileInfo fi(imagePath);
            if (fi.lastModified().date() < cutoffDate) {
                qint64 fileSize = fi.size();
                if (QFile::remove(imagePath)) {
                    deletedCount++;
                    freedBytes += fileSize;
                }
            }
        }
    }
    
    LOG_INFO("ImageStorage::cleanupExpiredImages - Deleted {} files, freed {} bytes",
             deletedCount, freedBytes);
    
    emit cleanupCompleted(deletedCount, freedBytes);
    
    return deletedCount;
}

int ImageStorage::cleanupExcessStorage()
{
    if (m_config.maxStorageSizeMB <= 0) {
        return 0;  // 无限制
    }
    
    qint64 maxBytes = m_config.maxStorageSizeMB * 1024 * 1024;
    qint64 currentSize = calculateDirectorySize(m_config.rootPath);
    
    if (currentSize <= maxBytes) {
        return 0;
    }
    
    LOG_INFO("ImageStorage::cleanupExcessStorage - Current: {} MB, Limit: {} MB",
             currentSize / (1024 * 1024), m_config.maxStorageSizeMB);
    
    // 收集所有图像文件，按时间排序
    QStringList allImages = collectImageFiles(m_config.rootPath);
    
    // 按修改时间排序（最旧的在前）
    std::sort(allImages.begin(), allImages.end(), [](const QString& a, const QString& b) {
        return QFileInfo(a).lastModified() < QFileInfo(b).lastModified();
    });
    
    int deletedCount = 0;
    qint64 freedBytes = 0;
    
    for (const QString& imagePath : allImages) {
        if (currentSize - freedBytes <= maxBytes * 0.9) {  // 留10%余量
            break;
        }
        
        qint64 fileSize = QFileInfo(imagePath).size();
        if (QFile::remove(imagePath)) {
            deletedCount++;
            freedBytes += fileSize;
        }
    }
    
    LOG_INFO("ImageStorage::cleanupExcessStorage - Deleted {} files, freed {} MB",
             deletedCount, freedBytes / (1024 * 1024));
    
    emit cleanupCompleted(deletedCount, freedBytes);
    
    return deletedCount;
}

void ImageStorage::startAutoCleanup()
{
    if (m_config.autoCleanupIntervalHours <= 0) {
        LOG_WARN("ImageStorage::startAutoCleanup - Auto cleanup interval not configured");
        return;
    }
    
    int intervalMs = m_config.autoCleanupIntervalHours * 60 * 60 * 1000;
    m_autoCleanupTimer->start(intervalMs);
    
    LOG_INFO("ImageStorage::startAutoCleanup - Auto cleanup enabled, interval: {} hours",
             m_config.autoCleanupIntervalHours);
}

void ImageStorage::stopAutoCleanup()
{
    if (m_autoCleanupTimer->isActive()) {
        m_autoCleanupTimer->stop();
        LOG_INFO("ImageStorage::stopAutoCleanup - Auto cleanup disabled");
    }
}

bool ImageStorage::isAutoCleanupEnabled() const
{
    return m_autoCleanupTimer->isActive();
}

StorageStats ImageStorage::getStats() const
{
    StorageStats stats;
    stats.totalSizeBytes = 0;
    stats.totalFileCount = 0;
    stats.originalCount = 0;
    stats.annotatedCount = 0;
    stats.thumbnailCount = 0;
    
    QDateTime oldest = QDateTime::currentDateTime();
    QDateTime newest;
    
    QDirIterator it(m_config.rootPath, {"*.jpg", "*.jpeg", "*.png", "*.bmp"},
                    QDir::Files, QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fi(filePath);
        
        stats.totalSizeBytes += fi.size();
        stats.totalFileCount++;
        
        // 根据路径判断类型
        if (filePath.contains(m_config.thumbnailSubdir)) {
            stats.thumbnailCount++;
        } else if (filePath.contains(m_config.annotatedSubdir)) {
            stats.annotatedCount++;
        } else {
            stats.originalCount++;
        }
        
        // 更新时间范围
        QDateTime fileTime = fi.lastModified();
        if (fileTime < oldest) {
            oldest = fileTime;
        }
        if (!newest.isValid() || fileTime > newest) {
            newest = fileTime;
        }
    }
    
    stats.oldestImageTime = (stats.totalFileCount > 0) ? oldest : QDateTime();
    stats.newestImageTime = newest;
    
    return stats;
}

int ImageStorage::getImageCountByDate(const QDate& date, ImageType type) const
{
    QString dateDir = m_config.rootPath;
    if (m_config.organizeByDate) {
        dateDir = QDir(m_config.rootPath).filePath(date.toString(m_config.dateFormat));
    }
    
    QString typePath = QDir(dateDir).filePath(getTypeSubdir(type));
    
    QDir dir(typePath);
    if (!dir.exists()) {
        return 0;
    }
    
    return dir.entryList({"*.jpg", "*.jpeg", "*.png", "*.bmp"}, QDir::Files).count();
}

QStringList ImageStorage::getImagesByDateRange(const QDate& startDate, const QDate& endDate,
                                                ImageType type) const
{
    QStringList images;
    
    QDate current = startDate;
    while (current <= endDate) {
        QString dateDir = m_config.rootPath;
        if (m_config.organizeByDate) {
            dateDir = QDir(m_config.rootPath).filePath(current.toString(m_config.dateFormat));
        }
        
        QString typePath = QDir(dateDir).filePath(getTypeSubdir(type));
        
        QDir dir(typePath);
        if (dir.exists()) {
            QStringList files = dir.entryList({"*.jpg", "*.jpeg", "*.png", "*.bmp"}, QDir::Files);
            for (const QString& file : files) {
                images.append(dir.filePath(file));
            }
        }
        
        current = current.addDays(1);
    }
    
    return images;
}

void ImageStorage::onAutoCleanupTimer()
{
    LOG_INFO("ImageStorage::onAutoCleanupTimer - Starting scheduled cleanup");
    
    cleanupExpiredImages();
    cleanupExcessStorage();
}

void ImageStorage::onAsyncSaveFinished()
{
    // 由 QFutureWatcher 处理
}

bool ImageStorage::ensureDirectoryExists(const QString& path)
{
    QDir dir(path);
    if (dir.exists()) {
        return true;
    }
    return dir.mkpath(".");
}

QString ImageStorage::getDateSubdir(const QDateTime& dt) const
{
    return dt.toString(m_config.dateFormat);
}

QString ImageStorage::getTypeSubdir(ImageType type) const
{
    switch (type) {
        case ImageType::Original:
            return m_config.originalSubdir;
        case ImageType::Annotated:
            return m_config.annotatedSubdir;
        case ImageType::Thumbnail:
            return m_config.thumbnailSubdir;
        case ImageType::Crop:
            return m_config.cropSubdir;
        default:
            return m_config.originalSubdir;
    }
}

QString ImageStorage::getFormatExtension(ImageFormat format) const
{
    switch (format) {
        case ImageFormat::JPEG:
            return ".jpg";
        case ImageFormat::PNG:
            return ".png";
        case ImageFormat::BMP:
            return ".bmp";
        case ImageFormat::Auto:
        default:
            return ".jpg";
    }
}

ImageFormat ImageStorage::detectFormat(const QString& path) const
{
    QString ext = QFileInfo(path).suffix().toLower();
    if (ext == "jpg" || ext == "jpeg") {
        return ImageFormat::JPEG;
    } else if (ext == "png") {
        return ImageFormat::PNG;
    } else if (ext == "bmp") {
        return ImageFormat::BMP;
    }
    return ImageFormat::Auto;
}

QImage ImageStorage::createThumbnailImage(const QImage& source) const
{
    if (source.isNull()) {
        return QImage();
    }
    
    return source.scaled(m_config.thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void ImageStorage::checkStorageLimit()
{
    if (m_config.maxStorageSizeMB <= 0) {
        return;
    }
    
    qint64 maxBytes = m_config.maxStorageSizeMB * 1024 * 1024;
    qint64 currentSize = calculateDirectorySize(m_config.rootPath);
    
    // 当达到90%容量时发出警告
    if (currentSize > maxBytes * 0.9) {
        emit storageWarning(currentSize, maxBytes);
        LOG_WARN("ImageStorage::checkStorageLimit - Storage at {}% capacity",
                 currentSize * 100 / maxBytes);
    }
}

qint64 ImageStorage::calculateDirectorySize(const QString& dirPath) const
{
    qint64 totalSize = 0;
    
    QDirIterator it(dirPath, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        totalSize += QFileInfo(it.next()).size();
    }
    
    return totalSize;
}

QStringList ImageStorage::collectImageFiles(const QString& dirPath) const
{
    QStringList images;
    
    QDirIterator it(dirPath, {"*.jpg", "*.jpeg", "*.png", "*.bmp"},
                    QDir::Files, QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        images.append(it.next());
    }
    
    return images;
}
