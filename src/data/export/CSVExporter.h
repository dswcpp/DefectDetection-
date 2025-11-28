#ifndef CSVEXPORTER_H
#define CSVEXPORTER_H

#include <QString>

class CSVExporter {
public:
  CSVExporter() = default;

  // TODO: 导出数据到 CSV
  bool exportTo(const QString &path);
};

#endif // CSVEXPORTER_H
