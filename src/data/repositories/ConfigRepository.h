#ifndef CONFIGREPOSITORY_H
#define CONFIGREPOSITORY_H

#include <QObject>

class ConfigRepository : public QObject {
  Q_OBJECT
public:
  explicit ConfigRepository(QObject *parent = nullptr);

signals:
};

#endif // CONFIGREPOSITORY_H
