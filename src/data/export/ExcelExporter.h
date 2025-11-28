#ifndef EXCELEXPORTER_H
#define EXCELEXPORTER_H

#include <QString>

class ExcelExporter {
public:
  ExcelExporter() = default;

  // TODO: 导出数据到 Excel (xlsx)
  bool exportTo(const QString &path);
};

#endif // EXCELEXPORTER_H
