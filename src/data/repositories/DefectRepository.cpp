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

bool DefectRepository::isReady() const {
  return isDatabaseOpen(m_connectionName);
}

int DefectRepository::totalCount() const {
  QSqlDatabase db = getDatabase(m_connectionName);
  if (!db.isOpen()) return 0;

  QSqlQuery query(db);
  if (query.exec("SELECT COUNT(*) FROM inspections") && query.next()) {
    return query.value(0).toInt();
  }
  return 0;
}

qint64 DefectRepository::insertInspection(const InspectionRecord& record) {
  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) {
    LOG_ERROR("DefectRepository: Database not open");
    return -1;
  }

  // 首先检查表是否存在
  QSqlQuery checkTable(db);
  if (!checkTable.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='inspections'")) {
    LOG_ERROR("DefectRepository: Failed to check table existence: {}", checkTable.lastError().text());
    return -1;
  }
  
  if (!checkTable.next()) {
    LOG_ERROR("DefectRepository: Table 'inspections' does not exist! Please run schema.sql first.");
    return -1;
  }
  
  // 检查表结构
  QSqlQuery schemaQuery(db);
  schemaQuery.exec("PRAGMA table_info(inspections)");

  QSqlQuery query(db);
  QString sql = 
    "INSERT INTO inspections ("
    "product_id, station_id, inspect_time, result, defect_count, "
    "max_severity, severity_level, cycle_time_ms, image_path, "
    "annotated_path, thumbnail_path, model_version"
    ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
  
  if (!query.prepare(sql)) {
    LOG_ERROR("DefectRepository: Failed to prepare: {}", query.lastError().text());
    return -1;
  }

  query.addBindValue(record.productId > 0 ? record.productId : QVariant());
  query.addBindValue(record.stationId > 0 ? record.stationId : QVariant());
  query.addBindValue(record.inspectTime.isValid() ? record.inspectTime : QDateTime::currentDateTime());
  query.addBindValue(record.result);
  query.addBindValue(record.defectCount);
  query.addBindValue(record.maxSeverity);
  query.addBindValue(record.severityLevel);
  query.addBindValue(record.cycleTimeMs);
  query.addBindValue(record.imagePath.isEmpty() ? QVariant() : record.imagePath);
  query.addBindValue(record.annotatedPath.isEmpty() ? QVariant() : record.annotatedPath);
  query.addBindValue(record.thumbnailPath.isEmpty() ? QVariant() : record.thumbnailPath);
  query.addBindValue(record.modelVersion.isEmpty() ? QVariant() : record.modelVersion);

  if (!query.exec()) {
    LOG_ERROR("DefectRepository: Failed to insert inspection: {}", query.lastError().text());
    LOG_ERROR("DefectRepository: SQL was: {}", query.lastQuery());
    LOG_ERROR("DefectRepository: Bound values count: {}", query.boundValues().size());
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
  query.prepare(
    "INSERT INTO defects ("
    "inspection_id, defect_type, confidence, severity_score, "
    "severity_level, region_x, region_y, region_width, region_height, features"
    ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
  );

  query.addBindValue(defect.inspectionId);
  query.addBindValue(defect.defectType);
  query.addBindValue(defect.confidence);
  query.addBindValue(defect.severityScore);
  query.addBindValue(defect.severityLevel);
  query.addBindValue(defect.region.x());
  query.addBindValue(defect.region.y());
  query.addBindValue(defect.region.width());
  query.addBindValue(defect.region.height());
  query.addBindValue(defect.features.isEmpty() ? QVariant() : defect.features);

  if (!query.exec()) {
    LOG_ERROR("DefectRepository: Failed to insert defect: {}", query.lastError().text());
    return false;
  }

  return true;
}

bool DefectRepository::insertDefects(const QVector<DefectRecord>& defects) {
  if (!isDatabaseOpen(m_connectionName) || defects.isEmpty()) {
    return false;
  }

  TransactionGuard guard(m_connectionName);
  if (!guard.isActive()) {
    return false;
  }

  for (const auto& defect : defects) {
    if (!insertDefect(defect)) {
      return false; // guard 析构时自动回滚
    }
  }
  return guard.commit();
}

QVector<InspectionRecord> DefectRepository::queryInspections(const InspectionFilter& filter) {
  QVector<InspectionRecord> results;

  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) {
    return results;
  }

  QString sql =
    "SELECT id, product_id, station_id, inspect_time, result, defect_count, "
    "max_severity, severity_level, cycle_time_ms, image_path, "
    "annotated_path, thumbnail_path, model_version "
    "FROM inspections WHERE 1=1";

  if (filter.startTime.isValid()) {
    sql += " AND inspect_time >= ?";
  }
  if (filter.endTime.isValid()) {
    sql += " AND inspect_time <= ?";
  }
  if (!filter.result.isEmpty()) {
    sql += " AND result = ?";
  }

  sql += " ORDER BY inspect_time DESC";
  sql += " LIMIT ? OFFSET ?";

  QSqlQuery query(db);
  if (!query.prepare(sql)) {
    LOG_ERROR("DefectRepository: Query prepare failed: {}", query.lastError().text());
    LOG_ERROR("DefectRepository: SQL: {}", sql);
    return results;
  }

  if (filter.startTime.isValid()) {
    query.addBindValue(filter.startTime);
  }
  if (filter.endTime.isValid()) {
    query.addBindValue(filter.endTime);
  }
  if (!filter.result.isEmpty()) {
    query.addBindValue(filter.result);
  }
  query.addBindValue(filter.limit);
  query.addBindValue(filter.offset);

  if (!query.exec()) {
    LOG_ERROR("DefectRepository: Query exec failed: {}", query.lastError().text());
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
  
  if (!results.isEmpty()) {
    LOG_DEBUG("DefectRepository::queryInspections - Found {} records (filter: {}-{}, result={})",
              results.size(), 
              filter.startTime.isValid() ? filter.startTime.toString("yyyy-MM-dd").toStdString() : "any",
              filter.endTime.isValid() ? filter.endTime.toString("yyyy-MM-dd").toStdString() : "any",
              filter.result.isEmpty() ? "all" : filter.result.toStdString());
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
  query.prepare(
    "SELECT id, product_id, station_id, inspect_time, result, defect_count, "
    "max_severity, severity_level, cycle_time_ms, image_path, "
    "annotated_path, thumbnail_path, model_version "
    "FROM inspections WHERE id = ?"
  );
  query.addBindValue(id);

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
  query.prepare(
    "SELECT id, inspection_id, defect_type, confidence, severity_score, "
    "severity_level, region_x, region_y, region_width, region_height, "
    "features, created_at "
    "FROM defects WHERE inspection_id = ?"
  );
  query.addBindValue(inspectionId);

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
    sql += " AND inspect_time >= ?";
  }
  if (filter.endTime.isValid()) {
    sql += " AND inspect_time <= ?";
  }
  if (!filter.result.isEmpty()) {
    sql += " AND result = ?";
  }

  QSqlQuery query(db);
  query.prepare(sql);

  if (filter.startTime.isValid()) {
    query.addBindValue(filter.startTime);
  }
  if (filter.endTime.isValid()) {
    query.addBindValue(filter.endTime);
  }
  if (!filter.result.isEmpty()) {
    query.addBindValue(filter.result);
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

QMap<QString, int> DefectRepository::getDefectTypeStatistics(const InspectionFilter& filter) {
  QMap<QString, int> stats;

  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) {
    return stats;
  }

  QString sql = 
    "SELECT d.defect_type, COUNT(*) as count "
    "FROM defects d "
    "INNER JOIN inspections i ON d.inspection_id = i.id "
    "WHERE 1=1";

  if (filter.startTime.isValid()) {
    sql += " AND i.inspect_time >= ?";
  }
  if (filter.endTime.isValid()) {
    sql += " AND i.inspect_time <= ?";
  }
  if (!filter.result.isEmpty()) {
    sql += " AND i.result = ?";
  }

  sql += " GROUP BY d.defect_type ORDER BY count DESC";

  QSqlQuery query(db);
  if (!query.prepare(sql)) {
    LOG_ERROR("DefectRepository: getDefectTypeStatistics prepare failed: {}", query.lastError().text());
    return stats;
  }

  if (filter.startTime.isValid()) {
    query.addBindValue(filter.startTime);
  }
  if (filter.endTime.isValid()) {
    query.addBindValue(filter.endTime);
  }
  if (!filter.result.isEmpty()) {
    query.addBindValue(filter.result);
  }

  if (!query.exec()) {
    LOG_ERROR("DefectRepository: getDefectTypeStatistics exec failed: {}", query.lastError().text());
    return stats;
  }

  while (query.next()) {
    QString type = query.value("defect_type").toString();
    int count = query.value("count").toInt();
    if (!type.isEmpty()) {
      stats[type] = count;
    }
  }

  return stats;
}

QMap<QString, int> DefectRepository::getSeverityStatistics(const InspectionFilter& filter) {
  QMap<QString, int> stats;

  QSqlDatabase db = QSqlDatabase::database(m_connectionName);
  if (!db.isOpen()) {
    return stats;
  }

  QString sql = 
    "SELECT severity_level, COUNT(*) as count "
    "FROM inspections "
    "WHERE result = 'NG'";

  if (filter.startTime.isValid()) {
    sql += " AND inspect_time >= ?";
  }
  if (filter.endTime.isValid()) {
    sql += " AND inspect_time <= ?";
  }

  sql += " GROUP BY severity_level";

  QSqlQuery query(db);
  if (!query.prepare(sql)) {
    return stats;
  }

  if (filter.startTime.isValid()) {
    query.addBindValue(filter.startTime);
  }
  if (filter.endTime.isValid()) {
    query.addBindValue(filter.endTime);
  }

  if (query.exec()) {
    while (query.next()) {
      QString level = query.value("severity_level").toString();
      int count = query.value("count").toInt();
      if (!level.isEmpty()) {
        stats[level] = count;
      }
    }
  }

  return stats;
}

bool DefectRepository::deleteInspection(qint64 id) {
  if (!isDatabaseOpen(m_connectionName)) {
    return false;
  }

  TransactionGuard guard(m_connectionName);
  if (!guard.isActive()) {
    return false;
  }

  QSqlDatabase db = getDatabase(m_connectionName);

  // 先删除关联的缺陷记录
  QSqlQuery deleteDefects(db);
  deleteDefects.prepare("DELETE FROM defects WHERE inspection_id = ?");
  deleteDefects.addBindValue(id);
  if (!deleteDefects.exec()) {
    return false; // guard 析构时自动回滚
  }

  // 删除检测记录
  QSqlQuery deleteInsp(db);
  deleteInsp.prepare("DELETE FROM inspections WHERE id = ?");
  deleteInsp.addBindValue(id);
  if (!deleteInsp.exec()) {
    return false;
  }

  return guard.commit();
}

int DefectRepository::deleteInspections(const QVector<qint64>& ids) {
  if (!isDatabaseOpen(m_connectionName) || ids.isEmpty()) {
    return 0;
  }

  int deletedCount = 0;
  for (qint64 id : ids) {
    if (deleteInspection(id)) {
      deletedCount++;
    }
  }
  
  LOG_INFO("DefectRepository::deleteInspections - Deleted {} of {} records", deletedCount, ids.size());
  return deletedCount;
}
