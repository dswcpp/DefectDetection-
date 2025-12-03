/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * DefectTableModel.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：缺陷表格模型定义
 * 描述：Qt Model/View架构的缺陷数据模型，供QTableView显示单次检测的缺陷列表
 *
 * 当前版本：1.0
 */

#ifndef DEFECTTABLEMODEL_H
#define DEFECTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QColor>
#include <QRect>
#include <QMap>
#include "ui_global.h"
#include "common/Types.h"

// 缺陷详情结构（扩展 DefectRegion）
struct DefectItem {
  int id = 0;                    // 缺陷ID
  QString className;             // 缺陷类别名称
  int classId = 0;               // 缺陷类别ID
  QRect bbox;                    // 边界框
  double confidence = 0.0;       // 置信度
  double area = 0.0;             // 面积（像素）
  QString severity;              // 严重度等级
  QColor color;                  // 显示颜色
  
  DefectItem() = default;
  DefectItem(const DefectRegion& region, int idx = 0);
};

class UI_LIBRARY DefectTableModel : public QAbstractTableModel {
  Q_OBJECT
public:
  enum Column {
    ColId = 0,          // 序号
    ColClass,           // 缺陷类别
    ColConfidence,      // 置信度
    ColArea,            // 面积
    ColPosition,        // 位置 (x, y)
    ColSize,            // 尺寸 (w x h)
    ColSeverity,        // 严重度
    ColCount
  };

  explicit DefectTableModel(QObject *parent = nullptr);

  // 数据操作
  void setDefects(const std::vector<DefectRegion>& defects);
  void setDefects(const QVector<DefectItem>& defects);
  void addDefect(const DefectItem& defect);
  void removeDefect(int row);
  void updateDefect(int row, const DefectItem& defect);
  void clear();

  // 数据访问
  DefectItem defectAt(int row) const;
  int defectCount() const { return m_defects.size(); }
  QVector<DefectItem> allDefects() const { return m_defects; }

  // 统计信息
  double totalArea() const;
  double maxConfidence() const;
  double minConfidence() const;
  double avgConfidence() const;
  QMap<QString, int> classStatistics() const;

  // QAbstractTableModel 接口
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  // 类别颜色映射
  void setClassColor(int classId, const QColor& color);
  void setClassColor(const QString& className, const QColor& color);
  QColor classColor(int classId) const;
  
  // 类别名称映射
  void setClassName(int classId, const QString& name);
  QString className(int classId) const;

signals:
  void defectsChanged();
  void defectSelected(int row);

private:
  QColor severityColor(const QString& severity) const;
  QString formatPosition(const QRect& bbox) const;
  QString formatSize(const QRect& bbox) const;

  QVector<DefectItem> m_defects;
  QMap<int, QString> m_classNames;
  QMap<int, QColor> m_classColors;
  QMap<QString, QColor> m_classColorsByName;
  
  // 默认类别颜色
  static const QVector<QColor> s_defaultColors;
};

#endif // DEFECTTABLEMODEL_H
