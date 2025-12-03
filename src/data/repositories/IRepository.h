/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * IRepository.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：数据仓储接口基类定义
 * 描述：Repository模式基类，定义CRUD操作接口，所有具体Repository继承此类
 *
 * 当前版本：1.0
 */

#ifndef IREPOSITORY_H
#define IREPOSITORY_H

#include <QString>

class IRepository {
public:
  virtual ~IRepository() = default;

  // TODO: 定义通用接口，如 migrate/clear/cache 控制
  virtual QString name() const = 0;
};

#endif // IREPOSITORY_H
