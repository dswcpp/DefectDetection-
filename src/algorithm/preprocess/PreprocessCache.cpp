#include "PreprocessCache.h"
#include <QDateTime>
#include <QCryptographicHash>

PreprocessCache::PreprocessCache(size_t maxSize) : m_maxSize(maxSize) {
}

cv::Mat PreprocessCache::getOrCompute(const cv::Mat& input, 
                                      const QString& key,
                                      std::function<cv::Mat(const cv::Mat&)> computeFunc) {
  QMutexLocker locker(&m_mutex);
  
  QByteArray inputHash = computeHash(input);
  
  // 检查缓存
  if (m_cache.contains(key)) {
    auto& entry = m_cache[key];
    if (entry.inputHash == inputHash) {
      entry.lastAccess = QDateTime::currentMSecsSinceEpoch();
      ++m_hits;
      return entry.result.clone();
    }
  }
  
  // 缓存未命中，计算结果
  ++m_misses;
  locker.unlock();  // 计算时不持有锁
  
  cv::Mat result = computeFunc(input);
  
  locker.relock();
  
  // 如果缓存已满，移除最旧的
  if (static_cast<size_t>(m_cache.size()) >= m_maxSize) {
    evictOldest();
  }
  
  // 存入缓存
  CacheEntry entry;
  entry.result = result.clone();
  entry.inputHash = inputHash;
  entry.lastAccess = QDateTime::currentMSecsSinceEpoch();
  m_cache[key] = entry;
  
  return result;
}

void PreprocessCache::clear() {
  QMutexLocker locker(&m_mutex);
  m_cache.clear();
  m_hits = 0;
  m_misses = 0;
}

void PreprocessCache::setMaxSize(size_t size) {
  QMutexLocker locker(&m_mutex);
  m_maxSize = size;
  while (static_cast<size_t>(m_cache.size()) > m_maxSize) {
    evictOldest();
  }
}

double PreprocessCache::hitRate() const {
  size_t total = m_hits + m_misses;
  return total > 0 ? static_cast<double>(m_hits) / total : 0.0;
}

QByteArray PreprocessCache::computeHash(const cv::Mat& mat) const {
  if (mat.empty()) {
    return QByteArray();
  }
  
  // 使用图片的部分数据计算哈希（避免大图耗时）
  QCryptographicHash hash(QCryptographicHash::Md5);
  
  // 添加尺寸信息
  int type = mat.type();
  hash.addData(QByteArrayView(reinterpret_cast<const char*>(&mat.cols), sizeof(mat.cols)));
  hash.addData(QByteArrayView(reinterpret_cast<const char*>(&mat.rows), sizeof(mat.rows)));
  hash.addData(QByteArrayView(reinterpret_cast<const char*>(&type), sizeof(type)));
  
  // 采样部分像素
  int step = std::max(1, mat.rows / 10);
  for (int i = 0; i < mat.rows; i += step) {
    const uchar* row = mat.ptr<uchar>(i);
    int rowBytes = static_cast<int>(mat.cols * mat.elemSize());
    int bytesToHash = std::min(rowBytes, 1000);
    hash.addData(QByteArrayView(reinterpret_cast<const char*>(row), bytesToHash));
  }
  
  return hash.result();
}

void PreprocessCache::evictOldest() {
  if (m_cache.isEmpty()) return;
  
  QString oldestKey;
  qint64 oldestTime = LLONG_MAX;
  
  for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
    if (it.value().lastAccess < oldestTime) {
      oldestTime = it.value().lastAccess;
      oldestKey = it.key();
    }
  }
  
  if (!oldestKey.isEmpty()) {
    m_cache.remove(oldestKey);
  }
}
