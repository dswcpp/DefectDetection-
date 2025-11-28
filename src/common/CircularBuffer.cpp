#include "CircularBuffer.h"

CircularBuffer::CircularBuffer(std::size_t capacity)
    : m_storage(capacity == 0 ? 1 : capacity) {}

bool CircularBuffer::push(const std::any &value) {
  if (full()) {
    return false; // TODO: 支持覆盖模式或统计丢帧
  }
  m_storage[m_head] = value;
  m_head = (m_head + 1) % m_storage.size();
  m_full = m_head == m_tail;
  return true;
}

bool CircularBuffer::pop(std::any &out) {
  if (empty()) {
    return false;
  }
  out = std::move(m_storage[m_tail]);
  m_full = false;
  m_tail = (m_tail + 1) % m_storage.size();
  return true;
}

void CircularBuffer::clear() {
  m_head = m_tail = 0;
  m_full = false;
}

bool CircularBuffer::empty() const { return (!m_full && m_head == m_tail); }

bool CircularBuffer::full() const { return m_full; }

std::size_t CircularBuffer::size() const {
  if (m_full) {
    return m_storage.size();
  }
  if (m_head >= m_tail) {
    return m_head - m_tail;
  }
  return m_storage.size() - (m_tail - m_head);
}

std::size_t CircularBuffer::capacity() const { return m_storage.size(); }
