#ifndef DEFECTREPOSITORY_H
#define DEFECTREPOSITORY_H

#include <QObject>

class DefectRepository : public QObject {
  Q_OBJECT
public:
  explicit DefectRepository(QObject *parent = nullptr);

signals:
};

#endif // DEFECTREPOSITORY_H
