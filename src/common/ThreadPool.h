#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
  explicit ThreadPool(std::size_t threadCount = 0);
  ~ThreadPool();

  // 启动工作线程
  void start();

  // 停止并清理线程，丢弃未执行任务
  void stop();

  // 提交任务（无返回值），后续可扩展为 future 版本
  void enqueue(const std::function<void()> &task);

  bool isRunning() const;
  std::size_t pendingTasks() const;

private:
  void workerLoop();

private:
  std::vector<std::thread> m_workers;
  std::queue<std::function<void()>> m_tasks;
  std::atomic<bool> m_running{false};
  mutable std::mutex m_mutex;
  std::condition_variable m_cv;
};

#endif // THREADPOOL_H
