/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * CircularBuffer.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：环形缓冲区模板类定义
 * 描述：固定大小的环形缓冲区，支持FIFO操作，用于数据流缓存、
 *       历史记录存储等场景
 *
 * 当前版本：1.0
 */

#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <any>
#include <cstddef>
#include <vector>

class CircularBuffer {
public:
  explicit CircularBuffer(std::size_t capacity = 1024);

  bool push(const std::any &value);
  bool pop(std::any &out);

  void clear();
  bool empty() const;
  bool full() const;
  std::size_t size() const;
  std::size_t capacity() const;

private:
  std::vector<std::any> m_storage;
  std::size_t m_head = 0;
  std::size_t m_tail = 0;
  bool m_full = false;
};

#endif // CIRCULARBUFFER_H
