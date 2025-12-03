/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * InspectionRepository.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：检测记录仓储接口定义
 * 描述：检测记录数据访问层，存储每次检测的结果、时间、操作员等信息
 *
 * 当前版本：1.0
 */

#ifndef INSPECTIONREPOSITORY_H
#define INSPECTIONREPOSITORY_H

#include "IRepository.h"
#include <QObject>

class InspectionRepository : public QObject, public IRepository {
  Q_OBJECT
public:
  explicit InspectionRepository(QObject *parent = nullptr);

  QString name() const override { return "InspectionRepository"; }

  // TODO: 插入检测记录
  bool insert();

  // TODO: 查询检测记录
  bool query();
};

#endif // INSPECTIONREPOSITORY_H
