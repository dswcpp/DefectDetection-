#ifndef FILECAMERA_H
#define FILECAMERA_H

#include "ICamera.h"
#include "hal_global.h"
#include <QStringList>
#include <atomic>

class HAL_EXPORT FileCamera : public ICamera {
public:
  FileCamera() = default;

  bool open(const CameraConfig &cfg) override;
  bool grab(cv::Mat &frame) override;
  void close() override;
  QString currentImagePath() const override { return m_lastImagePath; }

  void setImageDir(const QString &dir);
  void setLoop(bool loop);
  int imageCount() const;

private:
  bool scanImages(const QString &dir);

  QStringList m_imagePaths;
  std::atomic<int> m_currentIndex{0};
  QString m_lastImagePath;
  bool m_loop = true;
  bool m_opened = false;
};

#endif // FILECAMERA_H
