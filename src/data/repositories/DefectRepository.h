/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * DefectRepository.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：缺陷数据仓储接口定义
 * 描述：缺陷记录数据访问层，存储缺陷位置、类型、严重度、置信度等
 *
 * 当前版本：1.0
 */

#ifndef DEFECTREPOSITORY_H
#define DEFECTREPOSITORY_H

#include "IRepository.h"
#include "../data_global.h"
#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QRect>
#include <QMap>

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
  QString operatorName;     // 操作员
  QString batchNo;          // 批次号
  QString productCode;      // 产品编码（字符串形式）
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
  QString severityLevel;    // 严重度过滤
  QString batchNo;          // 批次号过滤
  QString operatorName;     // 操作员过滤
  int minDefects = -1;      // 最小缺陷数（-1表示不限）
  int maxDefects = -1;      // 最大缺陷数（-1表示不限）
  int limit = 100;
  int offset = 0;
  QString orderBy;          // 排序字段
  bool ascending = false;   // 升序/降序
};

class DATA_EXPORT DefectRepository : public QObject, public IRepository {
  Q_OBJECT
public:
  explicit DefectRepository(const QString& connectionName, QObject *parent = nullptr);

  QString name() const override { return "DefectRepository"; }
  bool isReady() const override;
  int totalCount() const override;

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
  
  // 缺陷类型统计（返回 <类型名, 数量>）
  QMap<QString, int> getDefectTypeStatistics(const InspectionFilter& filter = InspectionFilter());
  
  // 严重度统计
  QMap<QString, int> getSeverityStatistics(const InspectionFilter& filter = InspectionFilter());

  // 删除记录
  bool deleteInspection(qint64 id);
  
  // 批量删除
  int deleteInspections(const QVector<qint64>& ids);

private:
  QString m_connectionName;
};

#endif // DEFECTREPOSITORY_H
