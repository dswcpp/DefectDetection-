#ifndef WSSERVER_H
#define WSSERVER_H

#include <QObject>

class WSServer : public QObject {
  Q_OBJECT
public:
  explicit WSServer(QObject *parent = nullptr);

signals:
};

#endif // WSSERVER_H
