#include "SPSCQueue.h"

bool SPSCQueue::push(const std::any &value) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_queue.push(value);
  return true;
}

bool SPSCQueue::pop(std::any &out) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_queue.empty()) {
    return false;
  }
  out = std::move(m_queue.front());
  m_queue.pop();
  return true;
}

bool SPSCQueue::tryPop(std::any &out) { return pop(out); }

void SPSCQueue::clear() {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::queue<std::any> empty;
  m_queue.swap(empty);
}

bool SPSCQueue::empty() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_queue.empty();
}

std::size_t SPSCQueue::size() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_queue.size();
}
