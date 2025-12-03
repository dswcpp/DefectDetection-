#include "CSVExporter.h"
#include "common/Logger.h"

bool CSVExporter::exportTo(const QString &path) {
  LOG_INFO("CSVExporter::exportTo - Exporting to: {}", path.toStdString());
  // TODO: 遍历数据并写入 CSV
  LOG_WARN("CSVExporter::exportTo - Not implemented yet");
  return true;
}
