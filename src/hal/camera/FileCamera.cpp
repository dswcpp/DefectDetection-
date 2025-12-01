#include "FileCamera.h"
#include "common/Logger.h"
#include <QDir>
#include <QFileInfo>

bool FileCamera::open(const CameraConfig &cfg) {
  if (m_opened) {
    LOG_WARN("FileCamera already opened");
    return true;
  }

  QString imageDir = cfg.ip; // 复用 ip 字段作为图片目录路径
  if (imageDir.isEmpty()) {
    imageDir = QDir::currentPath() + "/images";
  }

  if (!scanImages(imageDir)) {
    LOG_ERROR("FileCamera: No images found in {}", imageDir);
    return false;
  }

  m_currentIndex = 0;
  m_opened = true;
  LOG_INFO("FileCamera opened: {} images from {}", m_imagePaths.size(), imageDir);
  return true;
}

bool FileCamera::grab(cv::Mat &frame) {
  if (!m_opened || m_imagePaths.isEmpty()) {
    return false;
  }

  int idx = m_currentIndex.load();
  if (idx >= m_imagePaths.size()) {
    if (m_loop) {
      idx = 0;
      m_currentIndex.store(0);
    } else {
      LOG_DEBUG("FileCamera: End of image sequence");
      return false;
    }
  }

  const QString &path = m_imagePaths.at(idx);
  frame = cv::imread(path.toStdString(), cv::IMREAD_COLOR);
  if (frame.empty()) {
    LOG_WARN("FileCamera: Failed to read image {}", path);
    m_currentIndex.fetch_add(1);
    return false;
  }

  m_currentIndex.fetch_add(1);
  return true;
}

void FileCamera::close() {
  m_imagePaths.clear();
  m_currentIndex = 0;
  m_opened = false;
  LOG_INFO("FileCamera closed");
}

void FileCamera::setImageDir(const QString &dir) {
  if (m_opened) {
    LOG_WARN("FileCamera: Cannot change dir while opened");
    return;
  }
  scanImages(dir);
}

void FileCamera::setLoop(bool loop) {
  m_loop = loop;
}

int FileCamera::imageCount() const {
  return m_imagePaths.size();
}

bool FileCamera::scanImages(const QString &dir) {
  m_imagePaths.clear();

  QDir imageDir(dir);
  if (!imageDir.exists()) {
    LOG_WARN("FileCamera: Directory does not exist: {}", dir);
    return false;
  }

  QStringList filters;
  filters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp" << "*.tif" << "*.tiff";
  imageDir.setNameFilters(filters);
  imageDir.setSorting(QDir::Name);

  const auto files = imageDir.entryInfoList(QDir::Files | QDir::Readable);
  for (const QFileInfo &fi : files) {
    m_imagePaths.append(fi.absoluteFilePath());
  }

  LOG_DEBUG("FileCamera: Scanned {} images from {}", m_imagePaths.size(), dir);
  return !m_imagePaths.isEmpty();
}
