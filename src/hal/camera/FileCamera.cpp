#include "FileCamera.h"
#include "common/Logger.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>

bool FileCamera::open(const CameraConfig &cfg) {
  if (m_opened) {
    LOG_WARN("FileCamera already opened");
    return true;
  }

  QString imageDir = cfg.ip; // 复用 ip 字段作为图片目录路径
  if (imageDir.isEmpty()) {
    imageDir = "./images";
  }

  // 将相对路径转换为相对于应用程序目录的绝对路径
  if (QDir::isRelativePath(imageDir)) {
    imageDir = QCoreApplication::applicationDirPath() + "/" + imageDir;
  }
  imageDir = QDir::cleanPath(imageDir);

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
  m_lastImagePath = path;

  // 使用 Qt 读取文件，避免 OpenCV 对特殊字符路径的兼容性问题
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    LOG_WARN("FileCamera: Failed to open image {}", path);
    m_currentIndex.fetch_add(1);
    return false;
  }
  QByteArray data = file.readAll();
  file.close();

  if (data.isEmpty()) {
    LOG_WARN("FileCamera: Empty file {}", path);
    m_currentIndex.fetch_add(1);
    return false;
  }

  std::vector<uchar> buffer(data.begin(), data.end());
  frame = cv::imdecode(buffer, cv::IMREAD_COLOR);
  if (frame.empty()) {
    // 尝试其他解码标志
    frame = cv::imdecode(buffer, cv::IMREAD_UNCHANGED);
    if (!frame.empty()) {
      if (frame.channels() == 4) {
        cv::cvtColor(frame, frame, cv::COLOR_BGRA2BGR);
      } else if (frame.channels() == 1) {
        cv::cvtColor(frame, frame, cv::COLOR_GRAY2BGR);
      }
    }
  }

  if (frame.empty()) {
    // 只在解码失败时打印详细信息
    QString header;
    for (int i = 0; i < qMin(8, data.size()); ++i) {
      header += QString("%1 ").arg(static_cast<uchar>(data[i]), 2, 16, QChar('0')).toUpper();
    }
    LOG_WARN("FileCamera: Failed to decode {} (size={}bytes, header={})", 
             QFileInfo(path).fileName().toStdString(), data.size(), header.toStdString());
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
