#ifndef SPSCQUEUE_H
#define SPSCQUEUE_H

#include <any>
#include <cstddef>
#include <mutex>
#include <queue>

class SPSCQueue {
public:
  SPSCQueue() = default;

  bool push(const std::any &value);
  bool pop(std::any &out);
  bool tryPop(std::any &out);

  void clear();
  bool empty() const;
  std::size_t size() const;

private:
  mutable std::mutex m_mutex;
  std::queue<std::any> m_queue;
};

#endif // SPSCQUEUE_H
