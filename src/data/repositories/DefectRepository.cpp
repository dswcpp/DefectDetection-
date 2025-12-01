#include "DefectRepository.h"
#include "common/Types.h"
#include "common/Logger.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

DefectRepository::DefectRepository(const QString& connectionName, QObject *parent)
    : QObject{parent}
    , m_connectionName(connectionName) {}

qint64 DefectRepository::insertInspection(const InspectionRecord& record) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) {
    LOG_ERROR("DefectRepository: Database not open");
    return -1;
  }

  QSqlQuery query(db);
  query.prepare(R"(
    INSERT INTO inspections (
      product_id, station_id, inspect_time, result, defect_count,
      max_severity, severity_level, cycle_time_ms, image_path,
      annotated_path, thumbnail_path, model_version
    ) VALUES (
      :product_id, :station_id, :inspect_time, :result, :defect_count,
      :max_severity, :severity_level, :cycle_time_ms, :image_path,
      :annotated_path, :thumbnail_path, :model_version
    )
  )");

  query.bindValue(":product_id", record.productId > 0 ? record.productId : QVariant());
  query.bindValue(":station_id", record.stationId > 0 ? record.stationId : QVariant());
  query.bindValue(":inspect_time", record.inspectTime.isValid() ? record.inspectTime : QDateTime::currentDateTime());
  query.bindValue(":result", record.result);
  query.bindValue(":defect_count", record.defectCount);
  query.bindValue(":max_severity", record.maxSeverity);
  query.bindValue(":severity_level", record.severityLevel);
  query.bindValue(":cycle_time_ms", record.cycleTimeMs);
  query.bindValue(":image_path", record.imagePath.isEmpty() ? QVariant() : record.imagePath);
  query.bindValue(":annotated_path", record.annotatedPath.isEmpty() ? QVariant() : record.annotatedPath);
  query.bindValue(":thumbnail_path", record.thumbnailPath.isEmpty() ? QVariant() : record.thumbnailPath);
  query.bindValue(":model_version", record.modelVersion.isEmpty() ? QVariant() : record.modelVersion);

  if (!query.exec()) {
    LOG_ERROR("DefectRepository: Failed to insert inspection: {}", query.lastError().text());
    return -1;
  }

  qint64 id = query.lastInsertId().toLongLong();
  LOG_DEBUG("DefectRepository: Inserted inspection id={}", id);
  return id;
}

qint64 DefectRepository::insertFromResult(const DetectResult& result, const QString& imagePath) {
  InspectionRecord record;
  record.inspectTime = QDateTime::fromMSecsSinceEpoch(result.timestamp);
  record.result = result.isOK ? "OK" : "NG";
  record.defectCount = static_cast<int>(result.defects.size());
  record.maxSeverity = result.severity;
  record.cycleTimeMs = result.cycleTimeMs;
  record.imagePath = imagePath;

  // 转换严重度等级
  switch (result.level) {
    case SeverityLevel::OK: record.severityLevel = "OK"; break;
    case SeverityLevel::Minor: record.severityLevel = "Minor"; break;
    case SeverityLevel::Major: record.severityLevel = "Major"; break;
    case SeverityLevel::Critical: record.severityLevel = "Critical"; break;
  }

  qint64 inspectionId = insertInspection(record);
  if (inspectionId < 0) {
    return -1;
  }

  // 插入缺陷详情
  for (const auto& defect : result.defects) {
    DefectRecord defectRecord;
    defectRecord.inspectionId = inspectionId;
    defectRecord.defectType = result.defectType;
    defectRecord.confidence = defect.confidence;
    defectRecord.severityScore = result.severity;
    defectRecord.severityLevel = record.severityLevel;
    defectRecord.region = QRect(defect.bbox.x, defect.bbox.y, defect.bbox.width, defect.bbox.height);

    if (!insertDefect(defectRecord)) {
      LOG_WARN("DefectRepository: Failed to insert defect for inspection {}", inspectionId);
    }
  }

  return inspectionId;
}

bool DefectRepository::insertDefect(const DefectRecord& defect) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) {
    return false;
  }

  QSqlQuery query(db);
  query.prepare(R"(
    INSERT INTO defects (
      inspection_id, defect_type, confidence, severity_score,
      severity_level, region_x, region_y, region_width, region_height, features
    ) VALUES (
      :inspection_id, :defect_type, :confidence, :severity_score,
      :severity_level, :region_x, :region_y, :region_width, :region_height, :features
    )
  )");

  query.bindValue(":inspection_id", defect.inspectionId);
  query.bindValue(":defect_type", defect.defectType);
  query.bindValue(":confidence", defect.confidence);
  query.bindValue(":severity_score", defect.severityScore);
  query.bindValue(":severity_level", defect.severityLevel);
  query.bindValue(":region_x", defect.region.x());
  query.bindValue(":region_y", defect.region.y());
  query.bindValue(":region_width", defect.region.width());
  query.bindValue(":region_height", defect.region.height());
  query.bindValue(":features", defect.features.isEmpty() ? QVariant() : defect.features);

  if (!query.exec()) {
    LOG_ERROR("DefectRepository: Failed to insert defect: {}", query.lastError().text());
    return false;
  }

  return true;
}

bool DefectRepository::insertDefects(const QVector<DefectRecord>& defects) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen() || defects.isEmpty()) {
    return false;
  }

  db.transaction();
  for (const auto& defect : defects) {
    if (!insertDefect(defect)) {
      db.rollback();
      return false;
    }
  }
  return db.commit();
}

QVector<InspectionRecord> DefectRepository::queryInspections(const InspectionFilter& filter) {
  QVector<InspectionRecord> results;

  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) {
    return results;
  }

  QString sql = R"(
    SELECT id, product_id, station_id, inspect_time, result, defect_count,
           max_severity, severity_level, cycle_time_ms, image_path,
           annotated_path, thumbnail_path, model_version
    FROM inspections
    WHERE 1=1
  )";

  if (filter.startTime.isValid()) {
    sql += " AND inspect_time >= :start_time";
  }
  if (filter.endTime.isValid()) {
    sql += " AND inspect_time <= :end_time";
  }
  if (!filter.result.isEmpty()) {
    sql += " AND result = :result";
  }

  sql += " ORDER BY inspect_time DESC";
  sql += QString(" LIMIT %1 OFFSET %2").arg(filter.limit).arg(filter.offset);

  QSqlQuery query(db);
  query.prepare(sql);

  if (filter.startTime.isValid()) {
    query.bindValue(":start_time", filter.startTime);
  }
  if (filter.endTime.isValid()) {
    query.bindValue(":end_time", filter.endTime);
  }
  if (!filter.result.isEmpty()) {
    query.bindValue(":result", filter.result);
  }

  if (!query.exec()) {
    LOG_ERROR("DefectRepository: Query failed: {}", query.lastError().text());
    return results;
  }

  while (query.next()) {
    InspectionRecord record;
    record.id = query.value("id").toLongLong();
    record.productId = query.value("product_id").toLongLong();
    record.stationId = query.value("station_id").toLongLong();
    record.inspectTime = query.value("inspect_time").toDateTime();
    record.result = query.value("result").toString();
    record.defectCount = query.value("defect_count").toInt();
    record.maxSeverity = query.value("max_severity").toDouble();
    record.severityLevel = query.value("severity_level").toString();
    record.cycleTimeMs = query.value("cycle_time_ms").toInt();
    record.imagePath = query.value("image_path").toString();
    record.annotatedPath = query.value("annotated_path").toString();
    record.thumbnailPath = query.value("thumbnail_path").toString();
    record.modelVersion = query.value("model_version").toString();
    results.append(record);
  }

  return results;
}

InspectionRecord DefectRepository::getInspection(qint64 id) {
  InspectionRecord record;

  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) {
    return record;
  }

  QSqlQuery query(db);
  query.prepare(R"(
    SELECT id, product_id, station_id, inspect_time, result, defect_count,
           max_severity, severity_level, cycle_time_ms, image_path,
           annotated_path, thumbnail_path, model_version
    FROM inspections WHERE id = :id
  )");
  query.bindValue(":id", id);

  if (query.exec() && query.next()) {
    record.id = query.value("id").toLongLong();
    record.productId = query.value("product_id").toLongLong();
    record.stationId = query.value("station_id").toLongLong();
    record.inspectTime = query.value("inspect_time").toDateTime();
    record.result = query.value("result").toString();
    record.defectCount = query.value("defect_count").toInt();
    record.maxSeverity = query.value("max_severity").toDouble();
    record.severityLevel = query.value("severity_level").toString();
    record.cycleTimeMs = query.value("cycle_time_ms").toInt();
    record.imagePath = query.value("image_path").toString();
    record.annotatedPath = query.value("annotated_path").toString();
    record.thumbnailPath = query.value("thumbnail_path").toString();
    record.modelVersion = query.value("model_version").toString();
  }

  return record;
}

QVector<DefectRecord> DefectRepository::getDefects(qint64 inspectionId) {
  QVector<DefectRecord> results;

  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) {
    return results;
  }

  QSqlQuery query(db);
  query.prepare(R"(
    SELECT id, inspection_id, defect_type, confidence, severity_score,
           severity_level, region_x, region_y, region_width, region_height,
           features, created_at
    FROM defects WHERE inspection_id = :inspection_id
  )");
  query.bindValue(":inspection_id", inspectionId);

  if (!query.exec()) {
    LOG_ERROR("DefectRepository: getDefects failed: {}", query.lastError().text());
    return results;
  }

  while (query.next()) {
    DefectRecord record;
    record.id = query.value("id").toLongLong();
    record.inspectionId = query.value("inspection_id").toLongLong();
    record.defectType = query.value("defect_type").toString();
    record.confidence = query.value("confidence").toDouble();
    record.severityScore = query.value("severity_score").toDouble();
    record.severityLevel = query.value("severity_level").toString();
    record.region = QRect(
      query.value("region_x").toInt(),
      query.value("region_y").toInt(),
      query.value("region_width").toInt(),
      query.value("region_height").toInt()
    );
    record.features = query.value("features").toString();
    record.createdAt = query.value("created_at").toDateTime();
    results.append(record);
  }

  return results;
}

int DefectRepository::countInspections(const InspectionFilter& filter) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) {
    return 0;
  }

  QString sql = "SELECT COUNT(*) FROM inspections WHERE 1=1";

  if (filter.startTime.isValid()) {
    sql += " AND inspect_time >= :start_time";
  }
  if (filter.endTime.isValid()) {
    sql += " AND inspect_time <= :end_time";
  }
  if (!filter.result.isEmpty()) {
    sql += " AND result = :result";
  }

  QSqlQuery query(db);
  query.prepare(sql);

  if (filter.startTime.isValid()) {
    query.bindValue(":start_time", filter.startTime);
  }
  if (filter.endTime.isValid()) {
    query.bindValue(":end_time", filter.endTime);
  }
  if (!filter.result.isEmpty()) {
    query.bindValue(":result", filter.result);
  }

  if (query.exec() && query.next()) {
    return query.value(0).toInt();
  }

  return 0;
}

int DefectRepository::countByResult(const QString& result) {
  InspectionFilter filter;
  filter.result = result;
  filter.limit = INT_MAX;
  return countInspections(filter);
}

bool DefectRepository::deleteInspection(qint64 id) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) {
    return false;
  }

  db.transaction();

  // 先删除关联的缺陷记录
  QSqlQuery deleteDefects(db);
  deleteDefects.prepare("DELETE FROM defects WHERE inspection_id = :id");
  deleteDefects.bindValue(":id", id);
  if (!deleteDefects.exec()) {
    db.rollback();
    return false;
  }

  // 删除检测记录
  QSqlQuery deleteInspection(db);
  deleteInspection.prepare("DELETE FROM inspections WHERE id = :id");
  deleteInspection.bindValue(":id", id);
  if (!deleteInspection.exec()) {
    db.rollback();
    return false;
  }

  return db.commit();
}
