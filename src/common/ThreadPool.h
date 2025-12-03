/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * ThreadPool.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：线程池模块接口定义
 * 描述：基于std::thread的线程池实现，支持任务队列、动态线程数量、
 *       异步任务提交和future结果获取
 *
 * 当前版本：1.0
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "common_global.h"
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

// ============================================================================
// 线程池 - 支持 future、优先级、异常处理
// ============================================================================

class COMMON_LIBRARY ThreadPool {
public:
    // 任务优先级
    enum class Priority { Low = 0, Normal = 1, High = 2 };

    // 统计信息
    struct Stats {
        size_t totalTasks = 0;      // 总提交任务数
        size_t completedTasks = 0;  // 已完成任务数
        size_t failedTasks = 0;     // 失败任务数
        size_t pendingTasks = 0;    // 待处理任务数
        size_t activeThreads = 0;   // 活跃线程数
    };

    explicit ThreadPool(size_t threadCount = 0, const std::string& name = "ThreadPool");
    ~ThreadPool();

    // 禁止拷贝
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // 启动/停止
    void start();
    void stop();
    void waitAll();  // 等待所有任务完成

    // 提交任务（带返回值）
    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    // 提交任务（带优先级）
    template <typename F, typename... Args>
    auto submit(Priority priority, F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    // 提交任务（无返回值，简化版）
    void enqueue(std::function<void()> task);
    void enqueue(Priority priority, std::function<void()> task);

    // 状态查询
    bool isRunning() const { return m_running.load(); }
    size_t threadCount() const { return m_workers.size(); }
    size_t pendingTasks() const;
    Stats stats() const;

    // 设置异常处理器
    using ExceptionHandler = std::function<void(const std::exception&, const std::string&)>;
    void setExceptionHandler(ExceptionHandler handler);

private:
    struct Task {
        std::function<void()> func;
        Priority priority = Priority::Normal;

        bool operator<(const Task& other) const {
            return static_cast<int>(priority) < static_cast<int>(other.priority);
        }
    };

    void workerLoop(size_t workerId);

    std::string m_name;
    std::vector<std::thread> m_workers;
    std::priority_queue<Task> m_tasks;

    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::condition_variable m_cvComplete;

    std::atomic<bool> m_running{false};
    std::atomic<size_t> m_activeCount{0};
    std::atomic<size_t> m_totalTasks{0};
    std::atomic<size_t> m_completedTasks{0};
    std::atomic<size_t> m_failedTasks{0};

    ExceptionHandler m_exceptionHandler;
    size_t m_desiredThreadCount;
};

// ============================================================================
// 模板实现
// ============================================================================

template <typename F, typename... Args>
auto ThreadPool::submit(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result<F, Args...>::type> {
    return submit(Priority::Normal, std::forward<F>(f), std::forward<Args>(args)...);
}

template <typename F, typename... Args>
auto ThreadPool::submit(Priority priority, F&& f, Args&&... args)
    -> std::future<typename std::invoke_result<F, Args...>::type> {
    using ReturnType = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<ReturnType> result = task->get_future();

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_running) {
            throw std::runtime_error("ThreadPool is not running");
        }
        m_tasks.push(Task{[task]() { (*task)(); }, priority});
        ++m_totalTasks;
    }

    m_cv.notify_one();
    return result;
}

// ============================================================================
// 全局线程池单例
// ============================================================================

COMMON_LIBRARY ThreadPool& globalThreadPool();

#endif // THREADPOOL_H
