#include "DetectorFactory.h"

// 预定义的检测器类型常量
const QString DetectorFactory::TYPE_SCRATCH = "scratch";
const QString DetectorFactory::TYPE_CRACK = "crack";
const QString DetectorFactory::TYPE_FOREIGN = "foreign";
const QString DetectorFactory::TYPE_DIMENSION = "dimension";
const QString DetectorFactory::TYPE_YOLO = "yolo";

DetectorFactory& DetectorFactory::instance() {
  static DetectorFactory factory;
  return factory;
}

void DetectorFactory::registerDetector(const QString& type, CreatorFunc creator) {
  m_creators[type] = std::move(creator);
}

DetectorPtr DetectorFactory::create(const QString& type) const {
  auto it = m_creators.find(type);
  if (it != m_creators.end()) {
    return it->second();
  }
  return nullptr;
}

QStringList DetectorFactory::registeredTypes() const {
  QStringList types;
  for (const auto& pair : m_creators) {
    types.append(pair.first);
  }
  return types;
}

bool DetectorFactory::isRegistered(const QString& type) const {
  return m_creators.find(type) != m_creators.end();
}
