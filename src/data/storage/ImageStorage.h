/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ImageStorage.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：图像存储模块接口定义
 * 描述：管理检测图像的存储，支持原图/标注图保存、按日期分目录、
 *       自动清理过期文件等功能
 *
 * 当前版本：1.0
 */

#ifndef IMAGESTORAGE_H
#define IMAGESTORAGE_H

#include "../data_global.h"
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QImage>
#include <QDateTime>
#include <QSize>
#include <QTimer>
#include <QMutex>
#include <QThread>

/**
 * @brief 图像类型
 */
enum class ImageType {
    Original,       // 原始图像
    Annotated,      // 标注图像（带检测结果）
    Thumbnail,      // 缩略图
    Crop            // 裁剪图（缺陷区域）
};

/**
 * @brief 图像格式
 */
enum class ImageFormat {
    JPEG,           // JPEG 格式（有损压缩，小文件）
    PNG,            // PNG 格式（无损压缩）
    BMP,            // BMP 格式（无压缩）
    Auto            // 根据扩展名自动选择
};

/**
 * @brief 图像存储配置
 */
struct DATA_EXPORT ImageStorageConfig {
    QString rootPath = "images";            // 图像存储根目录
    int retentionDays = 90;                 // 图像保留天数
    int jpegQuality = 85;                   // JPEG 质量 (0-100)
    bool createThumbnails = true;           // 是否自动创建缩略图
    QSize thumbnailSize{200, 200};          // 缩略图大小
    bool organizeByDate = true;             // 是否按日期组织目录
    QString dateFormat = "yyyy/MM/dd";      // 日期目录格式
    int autoCleanupIntervalHours = 24;      // 自动清理间隔（小时），0禁用
    qint64 maxStorageSizeMB = 0;            // 最大存储空间（MB），0表示无限制
    
    // 子目录名称
    QString originalSubdir = "original";
    QString annotatedSubdir = "annotated";
    QString thumbnailSubdir = "thumbnail";
    QString cropSubdir = "crop";
};

/**
 * @brief 图像保存结果
 */
struct DATA_EXPORT ImageSaveResult {
    bool success;                   // 是否成功
    QString path;                   // 保存路径
    QString thumbnailPath;          // 缩略图路径（如果创建了）
    QString errorMessage;           // 错误信息
    qint64 fileSize;                // 文件大小（字节）
};

/**
 * @brief 存储统计信息
 */
struct DATA_EXPORT StorageStats {
    qint64 totalSizeBytes;          // 总大小（字节）
    int totalFileCount;             // 总文件数
    int originalCount;              // 原图数量
    int annotatedCount;             // 标注图数量
    int thumbnailCount;             // 缩略图数量
    QDateTime oldestImageTime;      // 最早图像时间
    QDateTime newestImageTime;      // 最新图像时间
};

/**
 * @brief 图像存储管理器
 *
 * 提供检测图像的存储、加载、管理功能，支持：
 * - 多种图像类型存储（原图、标注图、缩略图、裁剪图）
 * - 按日期自动组织目录结构
 * - 自动创建缩略图
 * - 过期图像自动清理
 * - 异步保存（不阻塞主线程）
 * - 存储空间限制
 *
 * 目录结构示例：
 * @code
 * images/
 * ├── 2025/
 * │   └── 12/
 * │       └── 03/
 * │           ├── original/
 * │           │   └── img_20251203_143022_001.jpg
 * │           ├── annotated/
 * │           │   └── img_20251203_143022_001_annotated.jpg
 * │           ├── thumbnail/
 * │           │   └── img_20251203_143022_001_thumb.jpg
 * │           └── crop/
 * │               └── img_20251203_143022_001_defect_0.jpg
 * @endcode
 *
 * 用法：
 * @code
 * ImageStorage storage;
 * ImageStorageConfig config;
 * config.rootPath = "D:/DetectionImages";
 * config.jpegQuality = 90;
 * storage.setConfig(config);
 *
 * // 同步保存
 * QImage image = ...;
 * ImageSaveResult result = storage.saveImage(image, ImageType::Original, "product_001");
 *
 * // 异步保存
 * connect(&storage, &ImageStorage::imageSaved, [](const ImageSaveResult& r) {
 *     qDebug() << "Saved:" << r.path << "Size:" << r.fileSize;
 * });
 * storage.saveImageAsync(image, ImageType::Annotated, "product_001");
 *
 * // 加载图像
 * QImage loaded = storage.loadImage(result.path);
 * @endcode
 */
class DATA_EXPORT ImageStorage : public QObject {
    Q_OBJECT

public:
    explicit ImageStorage(QObject* parent = nullptr);
    ~ImageStorage() override;

    // ======================== 配置 ========================

    /**
     * @brief 设置配置
     */
    void setConfig(const ImageStorageConfig& config);
    
    /**
     * @brief 获取配置
     */
    ImageStorageConfig config() const { return m_config; }

    // ======================== 路径生成 ========================

    /**
     * @brief 生成基于日期和类型的存储路径
     * @param baseName 基础文件名（不含扩展名）
     * @param type 图像类型
     * @param timestamp 时间戳，默认当前时间
     * @param format 图像格式
     * @return 完整存储路径
     */
    QString generatePath(const QString& baseName, ImageType type,
                         const QDateTime& timestamp = QDateTime::currentDateTime(),
                         ImageFormat format = ImageFormat::JPEG) const;

    /**
     * @brief 生成唯一文件名
     * @param prefix 前缀
     * @param timestamp 时间戳
     * @return 唯一文件名（不含扩展名和路径）
     */
    QString generateUniqueName(const QString& prefix = "img",
                               const QDateTime& timestamp = QDateTime::currentDateTime()) const;

    // ======================== 保存图像 ========================

    /**
     * @brief 保存 QImage（同步）
     * @param image 要保存的图像
     * @param type 图像类型
     * @param baseName 基础文件名（可选，为空自动生成）
     * @param format 图像格式
     * @return 保存结果
     */
    ImageSaveResult saveImage(const QImage& image, ImageType type,
                              const QString& baseName = QString(),
                              ImageFormat format = ImageFormat::JPEG);

    /**
     * @brief 保存 QByteArray 数据（同步）
     * @param data 图像数据
     * @param path 完整保存路径
     * @return 保存结果
     */
    ImageSaveResult saveImage(const QByteArray& data, const QString& path);

    /**
     * @brief 异步保存 QImage
     * @param image 要保存的图像
     * @param type 图像类型
     * @param baseName 基础文件名（可选）
     * @param format 图像格式
     * @note 完成后发出 imageSaved 信号
     */
    void saveImageAsync(const QImage& image, ImageType type,
                        const QString& baseName = QString(),
                        ImageFormat format = ImageFormat::JPEG);

    /**
     * @brief 批量异步保存
     * @param images 图像列表（图像, 类型, 基础名称）
     */
    void saveImagesAsync(const QList<std::tuple<QImage, ImageType, QString>>& images);

    // ======================== 加载图像 ========================

    /**
     * @brief 加载图像为 QImage
     * @param path 图像路径
     * @return 加载的图像，失败返回空图像
     */
    QImage loadImage(const QString& path) const;

    /**
     * @brief 加载图像为 QByteArray
     * @param path 图像路径
     * @return 图像数据，失败返回空数组
     */
    QByteArray loadImageData(const QString& path) const;

    /**
     * @brief 检查图像是否存在
     */
    bool imageExists(const QString& path) const;

    // ======================== 缩略图 ========================

    /**
     * @brief 创建缩略图
     * @param image 原始图像
     * @param baseName 基础文件名
     * @param timestamp 时间戳
     * @return 缩略图路径，失败返回空
     */
    QString createThumbnail(const QImage& image, const QString& baseName,
                            const QDateTime& timestamp = QDateTime::currentDateTime());

    /**
     * @brief 获取图像的缩略图路径
     * @param originalPath 原图路径
     * @return 对应的缩略图路径
     */
    QString getThumbnailPath(const QString& originalPath) const;

    // ======================== 删除操作 ========================

    /**
     * @brief 删除图像
     * @param path 图像路径
     * @return 是否成功
     */
    bool deleteImage(const QString& path);

    /**
     * @brief 删除指定日期的所有图像
     * @param date 日期
     * @return 删除的文件数
     */
    int deleteImagesByDate(const QDate& date);

    /**
     * @brief 清理过期图像
     * @param keepDays 保留天数，-1使用配置值
     * @return 删除的文件数
     */
    int cleanupExpiredImages(int keepDays = -1);

    /**
     * @brief 清理超出存储限制的图像
     * @return 删除的文件数
     */
    int cleanupExcessStorage();

    // ======================== 自动清理 ========================

    /**
     * @brief 启动自动清理定时器
     */
    void startAutoCleanup();

    /**
     * @brief 停止自动清理定时器
     */
    void stopAutoCleanup();

    /**
     * @brief 是否启用了自动清理
     */
    bool isAutoCleanupEnabled() const;

    // ======================== 统计信息 ========================

    /**
     * @brief 获取存储统计信息
     */
    StorageStats getStats() const;

    /**
     * @brief 获取指定日期的图像数量
     */
    int getImageCountByDate(const QDate& date, ImageType type = ImageType::Original) const;

    /**
     * @brief 获取指定日期范围的图像路径
     */
    QStringList getImagesByDateRange(const QDate& startDate, const QDate& endDate,
                                      ImageType type = ImageType::Original) const;

signals:
    /**
     * @brief 图像保存完成（异步操作）
     */
    void imageSaved(const ImageSaveResult& result);

    /**
     * @brief 批量保存完成
     */
    void batchSaveCompleted(int successCount, int failCount);

    /**
     * @brief 清理完成
     */
    void cleanupCompleted(int filesRemoved, qint64 bytesFreed);

    /**
     * @brief 存储空间警告
     */
    void storageWarning(qint64 usedBytes, qint64 limitBytes);

    /**
     * @brief 错误发生
     */
    void errorOccurred(const QString& error);

private slots:
    void onAutoCleanupTimer();
    void onAsyncSaveFinished();

private:
    // 内部实现
    bool ensureDirectoryExists(const QString& path);
    QString getDateSubdir(const QDateTime& dt) const;
    QString getTypeSubdir(ImageType type) const;
    QString getFormatExtension(ImageFormat format) const;
    ImageFormat detectFormat(const QString& path) const;
    QImage createThumbnailImage(const QImage& source) const;
    void checkStorageLimit();
    qint64 calculateDirectorySize(const QString& dirPath) const;
    QStringList collectImageFiles(const QString& dirPath) const;

    ImageStorageConfig m_config;
    QTimer* m_autoCleanupTimer = nullptr;
    
    // 异步保存
    QThread* m_saveThread = nullptr;
    mutable QMutex m_mutex;
    mutable std::atomic<int> m_uniqueCounter{0};
};

#endif // IMAGESTORAGE_H
