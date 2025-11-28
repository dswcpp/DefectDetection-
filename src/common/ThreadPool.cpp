#include "ThreadPool.h"
#include <algorithm>
#include <utility>

ThreadPool::ThreadPool(std::size_t threadCount) {
  if (threadCount == 0) {
    threadCount = std::max<std::size_t>(1, std::thread::hardware_concurrency());
  }
  m_workers.reserve(threadCount);
}

ThreadPool::~ThreadPool() { stop(); }

void ThreadPool::start() {
  if (m_running.exchange(true)) {
    return;
  }
  const auto desired = m_workers.capacity() == 0
                           ? std::max<std::size_t>(1, std::thread::hardware_concurrency())
                           : m_workers.capacity();
  for (std::size_t i = 0; i < desired; ++i) {
    m_workers.emplace_back(&ThreadPool::workerLoop, this);
  }
}

void ThreadPool::stop() {
  if (!m_running.exchange(false)) {
    return;
  }
  m_cv.notify_all();
  for (auto &t : m_workers) {
    if (t.joinable()) {
      t.join();
    }
  }
  m_workers.clear();

  // TODO: 需要时持久化或统计未执行任务
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::queue<std::function<void()>> empty;
    m_tasks.swap(empty);
  }
}

void ThreadPool::enqueue(const std::function<void()> &task) {
  if (!m_running.load()) {
    // TODO: 是否需要缓存待启动任务，当前直接丢弃
    return;
  }
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_tasks.push(task);
  }
  m_cv.notify_one();
}

bool ThreadPool::isRunning() const { return m_running.load(); }

std::size_t ThreadPool::pendingTasks() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_tasks.size();
}

void ThreadPool::workerLoop() {
  while (m_running.load()) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cv.wait(lock, [this] { return !m_running.load() || !m_tasks.empty(); });
      if (!m_running.load() && m_tasks.empty()) {
        return;
      }
      task = std::move(m_tasks.front());
      m_tasks.pop();
    }
    if (task) {
      task(); // TODO: 捕获异常并记录日志
    }
  }
}
