#include "InspectionRepository.h"

InspectionRepository::InspectionRepository(QObject *parent) : QObject{parent} {}

bool InspectionRepository::insert() {
  // TODO: 插入检测记录到数据库
  return true;
}

bool InspectionRepository::query() {
  // TODO: 查询检测记录
  return true;
}
