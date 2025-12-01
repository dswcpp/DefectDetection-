#ifndef DEFECTREPOSITORY_H
#define DEFECTREPOSITORY_H

#include "IRepository.h"
#include "../data_global.h"
#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QRect>

struct DetectResult;

// 检测记录结构（对应 inspections 表）
struct DATA_EXPORT InspectionRecord {
  qint64 id = 0;
  qint64 productId = 0;
  qint64 stationId = 0;
  QDateTime inspectTime;
  QString result;           // OK/NG/ERROR
  int defectCount = 0;
  double maxSeverity = 0.0;
  QString severityLevel;
  int cycleTimeMs = 0;
  QString imagePath;
  QString annotatedPath;
  QString thumbnailPath;
  QString modelVersion;
};

// 缺陷详情结构（对应 defects 表）
struct DATA_EXPORT DefectRecord {
  qint64 id = 0;
  qint64 inspectionId = 0;
  QString defectType;
  double confidence = 0.0;
  double severityScore = 0.0;
  QString severityLevel;
  QRect region;
  QString features;         // JSON
  QDateTime createdAt;
};

// 查询过滤条件
struct DATA_EXPORT InspectionFilter {
  QDateTime startTime;
  QDateTime endTime;
  QString result;           // 空表示全部, "OK"/"NG"
  int limit = 100;
  int offset = 0;
};

class DATA_EXPORT DefectRepository : public QObject, public IRepository {
  Q_OBJECT
public:
  explicit DefectRepository(const QString& connectionName, QObject *parent = nullptr);

  QString name() const override { return "DefectRepository"; }

  // 插入检测记录，返回新记录ID，失败返回-1
  qint64 insertInspection(const InspectionRecord& record);

  // 从 DetectResult 插入（便捷方法）
  qint64 insertFromResult(const DetectResult& result, const QString& imagePath = QString());

  // 插入缺陷详情
  bool insertDefect(const DefectRecord& defect);

  // 批量插入缺陷
  bool insertDefects(const QVector<DefectRecord>& defects);

  // 查询检测记录列表
  QVector<InspectionRecord> queryInspections(const InspectionFilter& filter = InspectionFilter());

  // 查询单条记录
  InspectionRecord getInspection(qint64 id);

  // 查询检测记录的缺陷详情
  QVector<DefectRecord> getDefects(qint64 inspectionId);

  // 统计查询
  int countInspections(const InspectionFilter& filter = InspectionFilter());
  int countByResult(const QString& result);  // OK/NG 数量

  // 删除记录
  bool deleteInspection(qint64 id);

private:
  QString m_connectionName;
};

#endif // DEFECTREPOSITORY_H
