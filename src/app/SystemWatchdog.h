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
