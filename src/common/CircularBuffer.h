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
