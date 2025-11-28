#ifndef FLOWCONTROLLER_H
#define FLOWCONTROLLER_H

#include <QObject>
#include <QString>

class FlowController : public QObject {
  Q_OBJECT
public:
  explicit FlowController(QObject *parent = nullptr);

  // 状态控制
  void setOnline(bool online);
  bool isOnline() const;

  // 触发开始/停止/单拍
  void start();
  void stop();
  void triggerOnce();

signals:
  void started();
  void stopped();
  void singleShotRequested();
  void statusChanged(bool online);

private:
  bool m_online = false;
};

#endif // FLOWCONTROLLER_H
