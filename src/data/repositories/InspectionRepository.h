#ifndef INSPECTIONREPOSITORY_H
#define INSPECTIONREPOSITORY_H

#include <QObject>

class InspectionRepository : public QObject {
  Q_OBJECT
public:
  explicit InspectionRepository(QObject *parent = nullptr);

signals:
};

#endif // INSPECTIONREPOSITORY_H
