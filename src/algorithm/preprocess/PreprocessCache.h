/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * PreprocessCache.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：预处理缓存模块接口定义
 * 描述：预处理结果缓存，避免重复计算，支持LRU淘汰策略、
 *       命中率统计
 *
 * 当前版本：1.0
 */

#ifndef PREPROCESSCACHE_H
#define PREPROCESSCACHE_H

#include "../algorithm_global.h"
#include <opencv2/core.hpp>
#include <QMutex>
#include <QHash>
#include <QByteArray>
#include <functional>

// 预处理结果缓存
class ALGORITHM_LIBRARY PreprocessCache {
public:
  PreprocessCache(size_t maxSize = 10);
  
  // 获取或计算预处理结果
  cv::Mat getOrCompute(const cv::Mat& input, 
                       const QString& key,
                       std::function<cv::Mat(const cv::Mat&)> computeFunc);
  
  // 清空缓存
  void clear();
  
  // 设置最大缓存大小
  void setMaxSize(size_t size);
  size_t maxSize() const { return m_maxSize; }
  
  // 统计信息
  size_t hitCount() const { return m_hits; }
  size_t missCount() const { return m_misses; }
  double hitRate() const;

private:
  struct CacheEntry {
    cv::Mat result;
    QByteArray inputHash;
    qint64 lastAccess;
  };
  
  QByteArray computeHash(const cv::Mat& mat) const;
  void evictOldest();
  
  QHash<QString, CacheEntry> m_cache;
  mutable QMutex m_mutex;
  size_t m_maxSize;
  size_t m_hits = 0;
  size_t m_misses = 0;
};

#endif // PREPROCESSCACHE_H
