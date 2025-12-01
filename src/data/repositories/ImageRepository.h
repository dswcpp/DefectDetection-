#ifndef IMAGEREPOSITORY_H
#define IMAGEREPOSITORY_H

#include "IRepository.h"
#include "data_global.h"
#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QString>

// 图片记录结构（对应 images 表）
struct DATA_EXPORT ImageRecord {
  qint64 id = 0;
  QString filePath;           // 系统保存路径
  QString thumbnailPath;      // 缩略图路径
  QString originalPath;       // 原始来源路径
  QString sourceType;         // file/hik/daheng/usb
  int width = 0;
  int height = 0;
  qint64 fileSize = 0;
  QString hash;               // 文件哈希
  QDateTime captureTime;
  qint64 batchId = 0;
  QDateTime createdAt;
};

// 查询过滤条件
struct DATA_EXPORT ImageFilter {
  QDateTime startTime;
  QDateTime endTime;
  QString sourceType;
  qint64 batchId = 0;
  int limit = 100;
  int offset = 0;
};

class DATA_EXPORT ImageRepository : public QObject, public IRepository {
  Q_OBJECT
public:
  explicit ImageRepository(const QString& connectionName, QObject* parent = nullptr);

  QString name() const override { return "ImageRepository"; }

  // 插入图片记录，返回新记录ID
  qint64 insert(const ImageRecord& record);

  // 根据ID获取
  ImageRecord getById(qint64 id);

  // 根据文件路径获取
  ImageRecord getByPath(const QString& filePath);

  // 根据哈希获取（用于去重）
  ImageRecord getByHash(const QString& hash);

  // 查询图片列表
  QVector<ImageRecord> query(const ImageFilter& filter = ImageFilter());

  // 统计数量
  int count(const ImageFilter& filter = ImageFilter());

  // 更新缩略图路径
  bool updateThumbnail(qint64 id, const QString& thumbnailPath);

  // 删除图片记录
  bool remove(qint64 id);

  // 检查是否存在
  bool exists(const QString& filePath);
  bool existsByHash(const QString& hash);

private:
  QString m_connectionName;
};

#endif // IMAGEREPOSITORY_H
