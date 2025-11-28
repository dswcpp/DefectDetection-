#include "DefectRepository.h"

DefectRepository::DefectRepository(QObject *parent) : QObject{parent} {}

bool DefectRepository::insert() {
  // TODO: 插入缺陷记录到数据库
  return true;
}

bool DefectRepository::query() {
  // TODO: 查询缺陷记录
  return true;
}
