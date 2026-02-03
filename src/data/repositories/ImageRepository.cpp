#include "ImageRepository.h"
#include "common/Logger.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

ImageRepository::ImageRepository(const QString& connectionName, QObject* parent)
    : QObject(parent)
    , m_connectionName(connectionName) {}

bool ImageRepository::isReady() const {
  return isDatabaseOpen(m_connectionName);
}

int ImageRepository::totalCount() const {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return 0;

  QSqlQuery query(db);
  if (query.exec("SELECT COUNT(*) FROM images") && query.next()) {
    return query.value(0).toInt();
  }
  return 0;
}

qint64 ImageRepository::insert(const ImageRecord& record) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) {
    LOG_ERROR("ImageRepository: Database not open");
    return -1;
  }

  QSqlQuery query(db);
  query.prepare(R"(
    INSERT INTO images (
      file_path, thumbnail_path, original_path, source_type,
      width, height, file_size, hash, capture_time, batch_id
    ) VALUES (
      :file_path, :thumbnail_path, :original_path, :source_type,
      :width, :height, :file_size, :hash, :capture_time, :batch_id
    )
  )");

  query.bindValue(":file_path", record.filePath);
  query.bindValue(":thumbnail_path", record.thumbnailPath.isEmpty() ? QVariant() : record.thumbnailPath);
  query.bindValue(":original_path", record.originalPath.isEmpty() ? QVariant() : record.originalPath);
  query.bindValue(":source_type", record.sourceType);
  query.bindValue(":width", record.width > 0 ? record.width : QVariant());
  query.bindValue(":height", record.height > 0 ? record.height : QVariant());
  query.bindValue(":file_size", record.fileSize > 0 ? record.fileSize : QVariant());
  query.bindValue(":hash", record.hash.isEmpty() ? QVariant() : record.hash);
  query.bindValue(":capture_time", record.captureTime.isValid() ? record.captureTime : QDateTime::currentDateTime());
  query.bindValue(":batch_id", record.batchId > 0 ? record.batchId : QVariant());

  if (!query.exec()) {
    LOG_ERROR("ImageRepository: Failed to insert: {}", query.lastError().text());
    return -1;
  }

  qint64 id = query.lastInsertId().toLongLong();
  LOG_DEBUG("ImageRepository: Inserted image id={}, path={}", id, record.filePath);
  return id;
}

ImageRecord ImageRepository::getById(qint64 id) {
  ImageRecord record;
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return record;

  QSqlQuery query(db);
  query.prepare(R"(
    SELECT id, file_path, thumbnail_path, original_path, source_type,
           width, height, file_size, hash, capture_time, batch_id, created_at
    FROM images WHERE id = :id
  )");
  query.bindValue(":id", id);

  if (query.exec() && query.next()) {
    record.id = query.value("id").toLongLong();
    record.filePath = query.value("file_path").toString();
    record.thumbnailPath = query.value("thumbnail_path").toString();
    record.originalPath = query.value("original_path").toString();
    record.sourceType = query.value("source_type").toString();
    record.width = query.value("width").toInt();
    record.height = query.value("height").toInt();
    record.fileSize = query.value("file_size").toLongLong();
    record.hash = query.value("hash").toString();
    record.captureTime = query.value("capture_time").toDateTime();
    record.batchId = query.value("batch_id").toLongLong();
    record.createdAt = query.value("created_at").toDateTime();
  }
  return record;
}

ImageRecord ImageRepository::getByPath(const QString& filePath) {
  ImageRecord record;
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return record;

  QSqlQuery query(db);
  query.prepare(R"(
    SELECT id, file_path, thumbnail_path, original_path, source_type,
           width, height, file_size, hash, capture_time, batch_id, created_at
    FROM images WHERE file_path = :file_path
  )");
  query.bindValue(":file_path", filePath);

  if (query.exec() && query.next()) {
    record.id = query.value("id").toLongLong();
    record.filePath = query.value("file_path").toString();
    record.thumbnailPath = query.value("thumbnail_path").toString();
    record.originalPath = query.value("original_path").toString();
    record.sourceType = query.value("source_type").toString();
    record.width = query.value("width").toInt();
    record.height = query.value("height").toInt();
    record.fileSize = query.value("file_size").toLongLong();
    record.hash = query.value("hash").toString();
    record.captureTime = query.value("capture_time").toDateTime();
    record.batchId = query.value("batch_id").toLongLong();
    record.createdAt = query.value("created_at").toDateTime();
  }
  return record;
}

ImageRecord ImageRepository::getByHash(const QString& hash) {
  ImageRecord record;
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen() || hash.isEmpty()) return record;

  QSqlQuery query(db);
  query.prepare(R"(
    SELECT id, file_path, thumbnail_path, original_path, source_type,
           width, height, file_size, hash, capture_time, batch_id, created_at
    FROM images WHERE hash = :hash LIMIT 1
  )");
  query.bindValue(":hash", hash);

  if (query.exec() && query.next()) {
    record.id = query.value("id").toLongLong();
    record.filePath = query.value("file_path").toString();
    record.thumbnailPath = query.value("thumbnail_path").toString();
    record.originalPath = query.value("original_path").toString();
    record.sourceType = query.value("source_type").toString();
    record.width = query.value("width").toInt();
    record.height = query.value("height").toInt();
    record.fileSize = query.value("file_size").toLongLong();
    record.hash = query.value("hash").toString();
    record.captureTime = query.value("capture_time").toDateTime();
    record.batchId = query.value("batch_id").toLongLong();
    record.createdAt = query.value("created_at").toDateTime();
  }
  return record;
}

QVector<ImageRecord> ImageRepository::query(const ImageFilter& filter) {
  QVector<ImageRecord> results;
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return results;

  QString sql = R"(
    SELECT id, file_path, thumbnail_path, original_path, source_type,
           width, height, file_size, hash, capture_time, batch_id, created_at
    FROM images WHERE 1=1
  )";

  if (filter.startTime.isValid()) sql += " AND capture_time >= :start_time";
  if (filter.endTime.isValid()) sql += " AND capture_time <= :end_time";
  if (!filter.sourceType.isEmpty()) sql += " AND source_type = :source_type";
  if (filter.batchId > 0) sql += " AND batch_id = :batch_id";

  sql += " ORDER BY capture_time DESC";
  sql += " LIMIT :limit OFFSET :offset";

  QSqlQuery query(db);
  query.prepare(sql);

  if (filter.startTime.isValid()) query.bindValue(":start_time", filter.startTime);
  if (filter.endTime.isValid()) query.bindValue(":end_time", filter.endTime);
  if (!filter.sourceType.isEmpty()) query.bindValue(":source_type", filter.sourceType);
  if (filter.batchId > 0) query.bindValue(":batch_id", filter.batchId);
  query.bindValue(":limit", filter.limit);
  query.bindValue(":offset", filter.offset);

  if (!query.exec()) {
    LOG_ERROR("ImageRepository: Query failed: {}", query.lastError().text());
    return results;
  }

  while (query.next()) {
    ImageRecord record;
    record.id = query.value("id").toLongLong();
    record.filePath = query.value("file_path").toString();
    record.thumbnailPath = query.value("thumbnail_path").toString();
    record.originalPath = query.value("original_path").toString();
    record.sourceType = query.value("source_type").toString();
    record.width = query.value("width").toInt();
    record.height = query.value("height").toInt();
    record.fileSize = query.value("file_size").toLongLong();
    record.hash = query.value("hash").toString();
    record.captureTime = query.value("capture_time").toDateTime();
    record.batchId = query.value("batch_id").toLongLong();
    record.createdAt = query.value("created_at").toDateTime();
    results.append(record);
  }

  return results;
}

int ImageRepository::count(const ImageFilter& filter) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return 0;

  QString sql = "SELECT COUNT(*) FROM images WHERE 1=1";
  if (filter.startTime.isValid()) sql += " AND capture_time >= :start_time";
  if (filter.endTime.isValid()) sql += " AND capture_time <= :end_time";
  if (!filter.sourceType.isEmpty()) sql += " AND source_type = :source_type";
  if (filter.batchId > 0) sql += " AND batch_id = :batch_id";

  QSqlQuery query(db);
  query.prepare(sql);

  if (filter.startTime.isValid()) query.bindValue(":start_time", filter.startTime);
  if (filter.endTime.isValid()) query.bindValue(":end_time", filter.endTime);
  if (!filter.sourceType.isEmpty()) query.bindValue(":source_type", filter.sourceType);
  if (filter.batchId > 0) query.bindValue(":batch_id", filter.batchId);

  if (query.exec() && query.next()) {
    return query.value(0).toInt();
  }
  return 0;
}

bool ImageRepository::updateThumbnail(qint64 id, const QString& thumbnailPath) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  query.prepare("UPDATE images SET thumbnail_path = :thumbnail_path WHERE id = :id");
  query.bindValue(":thumbnail_path", thumbnailPath);
  query.bindValue(":id", id);

  return query.exec();
}

bool ImageRepository::remove(qint64 id) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  query.prepare("DELETE FROM images WHERE id = :id");
  query.bindValue(":id", id);

  return query.exec();
}

bool ImageRepository::exists(const QString& filePath) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  query.prepare("SELECT 1 FROM images WHERE file_path = :file_path LIMIT 1");
  query.bindValue(":file_path", filePath);

  return query.exec() && query.next();
}

bool ImageRepository::existsByHash(const QString& hash) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen() || hash.isEmpty()) return false;

  QSqlQuery query(db);
  query.prepare("SELECT 1 FROM images WHERE hash = :hash LIMIT 1");
  query.bindValue(":hash", hash);

  return query.exec() && query.next();
}
