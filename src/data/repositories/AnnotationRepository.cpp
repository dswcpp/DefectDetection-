#include "AnnotationRepository.h"
#include "common/Logger.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

AnnotationRepository::AnnotationRepository(const QString& connectionName, QObject* parent)
    : QObject(parent)
    , m_connectionName(connectionName) {}

QString AnnotationRepository::statusToString(AnnotationStatus status) const {
  switch (status) {
    case AnnotationStatus::Pending: return "pending";
    case AnnotationStatus::Approved: return "approved";
    case AnnotationStatus::Rejected: return "rejected";
  }
  return "pending";
}

AnnotationStatus AnnotationRepository::stringToStatus(const QString& str) const {
  if (str == "approved") return AnnotationStatus::Approved;
  if (str == "rejected") return AnnotationStatus::Rejected;
  return AnnotationStatus::Pending;
}

qint64 AnnotationRepository::insert(const AnnotationRecord& record) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) {
    LOG_ERROR("AnnotationRepository: Database not open");
    return -1;
  }

  QSqlQuery query(db);
  query.prepare(R"(
    INSERT INTO annotations (
      image_id, class_name, class_id, shape,
      bbox_x, bbox_y, bbox_width, bbox_height, polygon_points,
      confidence, severity, description, annotator_id,
      status, is_manual
    ) VALUES (
      :image_id, :class_name, :class_id, :shape,
      :bbox_x, :bbox_y, :bbox_width, :bbox_height, :polygon_points,
      :confidence, :severity, :description, :annotator_id,
      :status, :is_manual
    )
  )");

  query.bindValue(":image_id", record.imageId);
  query.bindValue(":class_name", record.className);
  query.bindValue(":class_id", record.classId >= 0 ? record.classId : QVariant());
  query.bindValue(":shape", record.shape.isEmpty() ? "rectangle" : record.shape);
  query.bindValue(":bbox_x", record.bbox.x());
  query.bindValue(":bbox_y", record.bbox.y());
  query.bindValue(":bbox_width", record.bbox.width());
  query.bindValue(":bbox_height", record.bbox.height());
  query.bindValue(":polygon_points", record.polygonPoints.isEmpty() ? QVariant() : record.polygonPoints);
  query.bindValue(":confidence", record.confidence);
  query.bindValue(":severity", record.severity.isEmpty() ? QVariant() : record.severity);
  query.bindValue(":description", record.description.isEmpty() ? QVariant() : record.description);
  query.bindValue(":annotator_id", record.annotatorId > 0 ? record.annotatorId : QVariant());
  query.bindValue(":status", statusToString(record.status));
  query.bindValue(":is_manual", record.isManual ? 1 : 0);

  if (!query.exec()) {
    LOG_ERROR("AnnotationRepository: Failed to insert: {}", query.lastError().text());
    return -1;
  }

  return query.lastInsertId().toLongLong();
}

AnnotationRecord AnnotationRepository::getById(qint64 id) {
  AnnotationRecord record;
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return record;

  QSqlQuery query(db);
  query.prepare(R"(
    SELECT id, image_id, class_name, class_id, shape,
           bbox_x, bbox_y, bbox_width, bbox_height, polygon_points,
           confidence, severity, description, annotator_id,
           status, reviewer_id, reviewed_at, review_comment,
           is_manual, created_at, updated_at
    FROM annotations WHERE id = :id
  )");
  query.bindValue(":id", id);

  if (query.exec() && query.next()) {
    record.id = query.value("id").toLongLong();
    record.imageId = query.value("image_id").toLongLong();
    record.className = query.value("class_name").toString();
    record.classId = query.value("class_id").toInt();
    record.shape = query.value("shape").toString();
    record.bbox = QRect(
      query.value("bbox_x").toInt(),
      query.value("bbox_y").toInt(),
      query.value("bbox_width").toInt(),
      query.value("bbox_height").toInt()
    );
    record.polygonPoints = query.value("polygon_points").toString();
    record.confidence = query.value("confidence").toDouble();
    record.severity = query.value("severity").toString();
    record.description = query.value("description").toString();
    record.annotatorId = query.value("annotator_id").toLongLong();
    record.status = stringToStatus(query.value("status").toString());
    record.reviewerId = query.value("reviewer_id").toLongLong();
    record.reviewedAt = query.value("reviewed_at").toDateTime();
    record.reviewComment = query.value("review_comment").toString();
    record.isManual = query.value("is_manual").toInt() == 1;
    record.createdAt = query.value("created_at").toDateTime();
    record.updatedAt = query.value("updated_at").toDateTime();
  }
  return record;
}

QVector<AnnotationRecord> AnnotationRepository::getByImageId(qint64 imageId) {
  AnnotationFilter filter;
  filter.imageId = imageId;
  filter.limit = 1000;
  return query(filter);
}

QVector<AnnotationRecord> AnnotationRepository::query(const AnnotationFilter& filter) {
  QVector<AnnotationRecord> results;
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return results;

  QString sql = R"(
    SELECT id, image_id, class_name, class_id, shape,
           bbox_x, bbox_y, bbox_width, bbox_height, polygon_points,
           confidence, severity, description, annotator_id,
           status, reviewer_id, reviewed_at, review_comment,
           is_manual, created_at, updated_at
    FROM annotations WHERE 1=1
  )";

  if (filter.imageId > 0) sql += " AND image_id = :image_id";
  if (!filter.className.isEmpty()) sql += " AND class_name = :class_name";
  if (filter.filterByStatus) sql += " AND status = :status";
  if (filter.annotatorId > 0) sql += " AND annotator_id = :annotator_id";

  sql += " ORDER BY created_at DESC";
  sql += QString(" LIMIT %1 OFFSET %2").arg(filter.limit).arg(filter.offset);

  QSqlQuery query(db);
  query.prepare(sql);

  if (filter.imageId > 0) query.bindValue(":image_id", filter.imageId);
  if (!filter.className.isEmpty()) query.bindValue(":class_name", filter.className);
  if (filter.filterByStatus) query.bindValue(":status", statusToString(filter.status));
  if (filter.annotatorId > 0) query.bindValue(":annotator_id", filter.annotatorId);

  if (!query.exec()) {
    LOG_ERROR("AnnotationRepository: Query failed: {}", query.lastError().text());
    return results;
  }

  while (query.next()) {
    AnnotationRecord record;
    record.id = query.value("id").toLongLong();
    record.imageId = query.value("image_id").toLongLong();
    record.className = query.value("class_name").toString();
    record.classId = query.value("class_id").toInt();
    record.shape = query.value("shape").toString();
    record.bbox = QRect(
      query.value("bbox_x").toInt(),
      query.value("bbox_y").toInt(),
      query.value("bbox_width").toInt(),
      query.value("bbox_height").toInt()
    );
    record.polygonPoints = query.value("polygon_points").toString();
    record.confidence = query.value("confidence").toDouble();
    record.severity = query.value("severity").toString();
    record.description = query.value("description").toString();
    record.annotatorId = query.value("annotator_id").toLongLong();
    record.status = stringToStatus(query.value("status").toString());
    record.reviewerId = query.value("reviewer_id").toLongLong();
    record.reviewedAt = query.value("reviewed_at").toDateTime();
    record.reviewComment = query.value("review_comment").toString();
    record.isManual = query.value("is_manual").toInt() == 1;
    record.createdAt = query.value("created_at").toDateTime();
    record.updatedAt = query.value("updated_at").toDateTime();
    results.append(record);
  }

  return results;
}

bool AnnotationRepository::update(const AnnotationRecord& record) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  query.prepare(R"(
    UPDATE annotations SET
      class_name = :class_name, class_id = :class_id, shape = :shape,
      bbox_x = :bbox_x, bbox_y = :bbox_y,
      bbox_width = :bbox_width, bbox_height = :bbox_height,
      polygon_points = :polygon_points, confidence = :confidence,
      severity = :severity, description = :description,
      updated_at = CURRENT_TIMESTAMP
    WHERE id = :id
  )");

  query.bindValue(":id", record.id);
  query.bindValue(":class_name", record.className);
  query.bindValue(":class_id", record.classId >= 0 ? record.classId : QVariant());
  query.bindValue(":shape", record.shape);
  query.bindValue(":bbox_x", record.bbox.x());
  query.bindValue(":bbox_y", record.bbox.y());
  query.bindValue(":bbox_width", record.bbox.width());
  query.bindValue(":bbox_height", record.bbox.height());
  query.bindValue(":polygon_points", record.polygonPoints.isEmpty() ? QVariant() : record.polygonPoints);
  query.bindValue(":confidence", record.confidence);
  query.bindValue(":severity", record.severity.isEmpty() ? QVariant() : record.severity);
  query.bindValue(":description", record.description.isEmpty() ? QVariant() : record.description);

  return query.exec();
}

bool AnnotationRepository::remove(qint64 id) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  query.prepare("DELETE FROM annotations WHERE id = :id");
  query.bindValue(":id", id);
  return query.exec();
}

bool AnnotationRepository::removeByImageId(qint64 imageId) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  query.prepare("DELETE FROM annotations WHERE image_id = :image_id");
  query.bindValue(":image_id", imageId);
  return query.exec();
}

bool AnnotationRepository::approve(qint64 id, qint64 reviewerId, const QString& comment) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  query.prepare(R"(
    UPDATE annotations SET
      status = 'approved',
      reviewer_id = :reviewer_id,
      reviewed_at = CURRENT_TIMESTAMP,
      review_comment = :review_comment,
      updated_at = CURRENT_TIMESTAMP
    WHERE id = :id
  )");

  query.bindValue(":id", id);
  query.bindValue(":reviewer_id", reviewerId);
  query.bindValue(":review_comment", comment.isEmpty() ? QVariant() : comment);

  return query.exec();
}

bool AnnotationRepository::reject(qint64 id, qint64 reviewerId, const QString& comment) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return false;

  QSqlQuery query(db);
  query.prepare(R"(
    UPDATE annotations SET
      status = 'rejected',
      reviewer_id = :reviewer_id,
      reviewed_at = CURRENT_TIMESTAMP,
      review_comment = :review_comment,
      updated_at = CURRENT_TIMESTAMP
    WHERE id = :id
  )");

  query.bindValue(":id", id);
  query.bindValue(":reviewer_id", reviewerId);
  query.bindValue(":review_comment", comment);

  return query.exec();
}

bool AnnotationRepository::batchApprove(const QVector<qint64>& ids, qint64 reviewerId) {
  if (ids.isEmpty()) return true;

  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return false;

  db.transaction();
  for (qint64 id : ids) {
    if (!approve(id, reviewerId)) {
      db.rollback();
      return false;
    }
  }
  return db.commit();
}

bool AnnotationRepository::batchReject(const QVector<qint64>& ids, qint64 reviewerId, const QString& comment) {
  if (ids.isEmpty()) return true;

  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return false;

  db.transaction();
  for (qint64 id : ids) {
    if (!reject(id, reviewerId, comment)) {
      db.rollback();
      return false;
    }
  }
  return db.commit();
}

int AnnotationRepository::countByStatus(AnnotationStatus status) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return 0;

  QSqlQuery query(db);
  query.prepare("SELECT COUNT(*) FROM annotations WHERE status = :status");
  query.bindValue(":status", statusToString(status));

  if (query.exec() && query.next()) {
    return query.value(0).toInt();
  }
  return 0;
}

int AnnotationRepository::countByImageId(qint64 imageId) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return 0;

  QSqlQuery query(db);
  query.prepare("SELECT COUNT(*) FROM annotations WHERE image_id = :image_id");
  query.bindValue(":image_id", imageId);

  if (query.exec() && query.next()) {
    return query.value(0).toInt();
  }
  return 0;
}

int AnnotationRepository::countPendingReview() {
  return countByStatus(AnnotationStatus::Pending);
}

int AnnotationRepository::countApproved() {
  return countByStatus(AnnotationStatus::Approved);
}

QVector<DefectClass> AnnotationRepository::getAllClasses() {
  QVector<DefectClass> results;
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return results;

  QSqlQuery query(db);
  query.prepare(R"(
    SELECT class_id, class_name, display_name, color, description, is_active
    FROM defect_classes WHERE is_active = 1 ORDER BY class_id
  )");

  if (!query.exec()) return results;

  while (query.next()) {
    DefectClass cls;
    cls.classId = query.value("class_id").toInt();
    cls.className = query.value("class_name").toString();
    cls.displayName = query.value("display_name").toString();
    cls.color = query.value("color").toString();
    cls.description = query.value("description").toString();
    cls.isActive = query.value("is_active").toInt() == 1;
    results.append(cls);
  }

  return results;
}

DefectClass AnnotationRepository::getClassByName(const QString& name) {
  DefectClass cls;
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return cls;

  QSqlQuery query(db);
  query.prepare(R"(
    SELECT class_id, class_name, display_name, color, description, is_active
    FROM defect_classes WHERE class_name = :name
  )");
  query.bindValue(":name", name);

  if (query.exec() && query.next()) {
    cls.classId = query.value("class_id").toInt();
    cls.className = query.value("class_name").toString();
    cls.displayName = query.value("display_name").toString();
    cls.color = query.value("color").toString();
    cls.description = query.value("description").toString();
    cls.isActive = query.value("is_active").toInt() == 1;
  }

  return cls;
}

DefectClass AnnotationRepository::getClassById(int classId) {
  DefectClass cls;
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return cls;

  QSqlQuery query(db);
  query.prepare(R"(
    SELECT class_id, class_name, display_name, color, description, is_active
    FROM defect_classes WHERE class_id = :class_id
  )");
  query.bindValue(":class_id", classId);

  if (query.exec() && query.next()) {
    cls.classId = query.value("class_id").toInt();
    cls.className = query.value("class_name").toString();
    cls.displayName = query.value("display_name").toString();
    cls.color = query.value("color").toString();
    cls.description = query.value("description").toString();
    cls.isActive = query.value("is_active").toInt() == 1;
  }

  return cls;
}

int AnnotationRepository::getClassIdByName(const QString& name) {
  DefectClass cls = getClassByName(name);
  return cls.className.isEmpty() ? -1 : cls.classId;
}

QVector<AnnotationRecord> AnnotationRepository::getApprovedByImageId(qint64 imageId) {
  AnnotationFilter filter;
  filter.imageId = imageId;
  filter.filterByStatus = true;
  filter.status = AnnotationStatus::Approved;
  filter.limit = 1000;
  return query(filter);
}

QVector<qint64> AnnotationRepository::getImageIdsWithApprovedAnnotations() {
  QVector<qint64> ids;
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) return ids;

  QSqlQuery query(db);
  query.prepare("SELECT DISTINCT image_id FROM annotations WHERE status = 'approved'");

  if (!query.exec()) return ids;

  while (query.next()) {
    ids.append(query.value(0).toLongLong());
  }

  return ids;
}
