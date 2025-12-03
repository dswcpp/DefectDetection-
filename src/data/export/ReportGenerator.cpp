#include "ReportGenerator.h"
#include "common/Logger.h"

bool ReportGenerator::generate(const QString &path) {
  LOG_INFO("ReportGenerator::generate - Generating report to: {}", path.toStdString());
  // TODO: 汇总数据生成报告文件
  LOG_WARN("ReportGenerator::generate - Not implemented yet");
  return true;
}
