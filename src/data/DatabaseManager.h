#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>

class DatabaseManager : public QObject {
  Q_OBJECT
public:
  explicit DatabaseManager(QObject *parent = nullptr);

signals:
};

#endif // DATABASEMANAGER_H
