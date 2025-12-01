#ifndef ANNOTATIONREPOSITORY_H
#define ANNOTATIONREPOSITORY_H

#include "IRepository.h"
#include "data_global.h"
#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QRect>
#include <QString>

// 标注状态
enum class AnnotationStatus {
  Pending,    // 待审核
  Approved,   // 已通过
  Rejected    // 已驳回
};

// 标注记录结构（对应 annotations 表）
struct DATA_EXPORT AnnotationRecord {
  qint64 id = 0;
  qint64 imageId = 0;
  QString className;          // 缺陷类别名称
  int classId = -1;           // 缺陷类别ID
  QString shape;              // rectangle/polygon/circle
  QRect bbox;                 // 边界框
  QString polygonPoints;      // JSON 多边形点
  double confidence = 0.0;
  QString severity;           // critical/major/minor/info
  QString description;
  qint64 annotatorId = 0;
  AnnotationStatus status = AnnotationStatus::Pending;
  qint64 reviewerId = 0;
  QDateTime reviewedAt;
  QString reviewComment;
  bool isManual = true;
  QDateTime createdAt;
  QDateTime updatedAt;
};

// 查询过滤条件
struct DATA_EXPORT AnnotationFilter {
  qint64 imageId = 0;
  QString className;
  AnnotationStatus status = AnnotationStatus::Pending;
  bool filterByStatus = false;
  qint64 annotatorId = 0;
  int limit = 100;
  int offset = 0;
};

// 缺陷类别
struct DATA_EXPORT DefectClass {
  int classId = 0;
  QString className;
  QString displayName;
  QString color;
  QString description;
  bool isActive = true;
};

class DATA_EXPORT AnnotationRepository : public QObject, public IRepository {
  Q_OBJECT
public:
  explicit AnnotationRepository(const QString& connectionName, QObject* parent = nullptr);

  QString name() const override { return "AnnotationRepository"; }

  // === 标注 CRUD ===
  qint64 insert(const AnnotationRecord& record);
  AnnotationRecord getById(qint64 id);
  QVector<AnnotationRecord> getByImageId(qint64 imageId);
  QVector<AnnotationRecord> query(const AnnotationFilter& filter = AnnotationFilter());
  bool update(const AnnotationRecord& record);
  bool remove(qint64 id);
  bool removeByImageId(qint64 imageId);

  // === 审核功能 ===
  bool approve(qint64 id, qint64 reviewerId, const QString& comment = QString());
  bool reject(qint64 id, qint64 reviewerId, const QString& comment);
  bool batchApprove(const QVector<qint64>& ids, qint64 reviewerId);
  bool batchReject(const QVector<qint64>& ids, qint64 reviewerId, const QString& comment);

  // === 统计 ===
  int countByStatus(AnnotationStatus status);
  int countByImageId(qint64 imageId);
  int countPendingReview();
  int countApproved();

  // === 缺陷类别管理 ===
  QVector<DefectClass> getAllClasses();
  DefectClass getClassByName(const QString& name);
  DefectClass getClassById(int classId);
  int getClassIdByName(const QString& name);

  // === 导出辅助 ===
  QVector<AnnotationRecord> getApprovedByImageId(qint64 imageId);
  QVector<qint64> getImageIdsWithApprovedAnnotations();

private:
  QString m_connectionName;
  QString statusToString(AnnotationStatus status) const;
  AnnotationStatus stringToStatus(const QString& str) const;
};

#endif // ANNOTATIONREPOSITORY_H
