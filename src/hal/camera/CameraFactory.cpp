#include "CameraFactory.h"
#include "DahengCamera.h"
#include "FileCamera.h"
#include "GigECamera.h"
#include "HikCamera.h"
#include "USBCamera.h"
#include "common/Logger.h"

std::unique_ptr<ICamera> CameraFactory::create(const QString &type) {
  const auto lower = type.toLower();
  if (lower == "gige") return std::make_unique<GigECamera>();
  if (lower == "usb") return std::make_unique<USBCamera>();
  if (lower == "hik") return std::make_unique<HikCamera>();
  if (lower == "daheng") return std::make_unique<DahengCamera>();
  if (lower == "file" || lower == "mock") return std::make_unique<FileCamera>();
  LOG_WARN("CameraFactory: Unknown camera type: {}", type);
  return nullptr;
}
