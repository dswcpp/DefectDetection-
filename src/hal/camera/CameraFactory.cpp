#include "CameraFactory.h"
#include "DahengCamera.h"
// #include "FileCamera.h"  // 暂时注释，因为文件不存在
#include "GigECamera.h"
#include "HikCamera.h"
#include "USBCamera.h"

std::unique_ptr<ICamera> CameraFactory::create(const QString &type) {
  const auto lower = type.toLower();
  if (lower == "gige") return std::make_unique<GigECamera>();
  if (lower == "usb") return std::make_unique<USBCamera>();
  if (lower == "hik") return std::make_unique<HikCamera>();
  if (lower == "daheng") return std::make_unique<DahengCamera>();
  // if (lower == "file" || lower == "mock") return std::make_unique<FileCamera>();  // 暂时注释
  // TODO: 记录不支持的相机类型
  return nullptr;
}
