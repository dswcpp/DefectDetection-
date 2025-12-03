/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * SystemWatchdog.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：系统看门狗接口定义
 * 描述：系统健康监控，检测主线程阻塞、内存泄漏、磁盘空间不足等异常
 *
 * 当前版本：1.0
 */

#ifndef SYSTEMWATCHDOG_H
#define SYSTEMWATCHDOG_H

#include <QObject>
#include <QString>

class SystemWatchdog : public QObject {
  Q_OBJECT
public:
  explicit SystemWatchdog(QObject *parent = nullptr);

  // 启动监控：相机/PLC/线程/队列
  void start();

  // 停止监控
  void stop();

  // 心跳喂狗
  void feed(const QString &module);

signals:
  void heartbeatTimeout(const QString &module);
  void statusReport(const QString &message);
};

#endif // SYSTEMWATCHDOG_H
